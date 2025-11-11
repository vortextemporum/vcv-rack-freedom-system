#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TapeAgeAudioProcessor::TapeAgeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
      parameters (*this, nullptr, juce::Identifier ("TapeAge"), createParameterLayout())
{
}

TapeAgeAudioProcessor::~TapeAgeAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout TapeAgeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // DRIVE parameter (0-100%, default 50%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive",
        "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + "%"; }
    ));

    // AGE parameter (0-100%, default 25%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "age",
        "Age",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.25f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + "%"; }
    ));

    // MIX parameter (0-100%, default 100%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mix",
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value * 100)) + "%"; }
    ));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String TapeAgeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TapeAgeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TapeAgeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TapeAgeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TapeAgeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TapeAgeAudioProcessor::getNumPrograms()
{
    return 1;
}

int TapeAgeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TapeAgeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TapeAgeAudioProcessor::getProgramName (int index)
{
    return {};
}

void TapeAgeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TapeAgeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize oversampler (2x, polyphase IIR for quality)
    const int numChannels = getTotalNumOutputChannels();
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(
        numChannels,
        1,  // 2^1 = 2x oversampling
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true  // normalize gain
    );

    oversampler->initProcessing(static_cast<size_t>(samplesPerBlock));

    // Preallocate dry buffer for mix stage (Phase 4.4)
    dryBuffer.setSize(numChannels, samplesPerBlock);
    dryBuffer.clear();

    // Phase 4.2: Initialize delay buffer for modulation (10ms capacity)
    // 10ms is enough for wow/flutter modulation while keeping latency low
    delayBufferLength = static_cast<int>(std::ceil(sampleRate * 0.01));  // 10ms in samples
    delayBuffer.setSize(numChannels, delayBufferLength);
    delayBuffer.clear();
    delayWritePosition = 0;

    // Reset LFO phases
    wowPhase = 0.0;
    flutterPhase = 0.0;

    // Phase 4.3: Initialize dropout and noise systems
    // Reset dropout state
    dropoutActive = false;
    dropoutSamplesRemaining = 0;
    dropoutGain = 1.0f;
    dropoutTargetGain = 1.0f;
    dropoutEnvelope.reset(sampleRate, 0.01);  // 10ms attack time
    dropoutEnvelope.setCurrentAndTargetValue(1.0f);

    // Reset pink noise generator state
    pinkNoise_b0 = 0.0f;
    pinkNoise_b1 = 0.0f;
    pinkNoise_b2 = 0.0f;
    pinkNoise_b3 = 0.0f;
    pinkNoise_b4 = 0.0f;
    pinkNoise_b5 = 0.0f;
    pinkNoise_b6 = 0.0f;
}

void TapeAgeAudioProcessor::releaseResources()
{
    // Reset oversampler
    if (oversampler)
        oversampler->reset();

    // Release dry buffer
    dryBuffer.setSize(0, 0);

    // Release delay buffer
    delayBuffer.setSize(0, 0);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TapeAgeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This plugin supports stereo in/out
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    #if ! JucePlugin_IsSynth
     if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
         return false;
    #endif

    return true;
  #endif
}
#endif

void TapeAgeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get parameter values (atomic, real-time safe)
    auto drive = parameters.getRawParameterValue("drive")->load();
    auto age = parameters.getRawParameterValue("age")->load();  // Will be used in Phase 4.2, 4.3
    auto mix = parameters.getRawParameterValue("mix")->load();  // Will be used in Phase 4.4
    juce::ignoreUnused(age, mix);

    // PHASE 4.1: CORE SATURATION
    // ========================================

    // Save dry buffer for later mix stage (Phase 4.4)
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    dryBuffer.makeCopyOf(buffer, true);

    // Map drive parameter (0.0-1.0) to saturation gain (1.0-10.0 logarithmic)
    // drive=0.0 → gain=1.0 (minimal saturation)
    // drive=0.5 → gain=3.0 (moderate warmth)
    // drive=1.0 → gain=10.0 (heavy saturation)
    const float saturationGain = std::pow(10.0f, drive * 2.0f);  // 10^(drive*2) = 1.0 to 100, then clamped
    const float driveAmount = juce::jmap(drive, 1.0f, 10.0f);    // Linear mapping 1-10

    // Create DSP block from buffer
    juce::dsp::AudioBlock<float> block(buffer);

    // Upsample audio for nonlinear processing (reduces aliasing)
    juce::dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(block);

    // Apply tape saturation (tanh waveshaper with asymmetry)
    for (size_t channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        auto* channelData = oversampledBlock.getChannelPointer(channel);
        const size_t numOversampledSamples = oversampledBlock.getNumSamples();

        for (size_t sample = 0; sample < numOversampledSamples; ++sample)
        {
            float x = channelData[sample];

            // Add small asymmetry for even harmonics (tape-like character)
            // Asymmetry factor creates 2nd harmonic
            const float asymmetry = 0.1f;  // Small amount for subtle even harmonics
            float asymmetricInput = x + asymmetry * x * x;

            // Apply tanh waveshaping with drive
            // y = tanh(g * x) where g = drive parameter
            float saturated = std::tanh(driveAmount * asymmetricInput);

            channelData[sample] = saturated;
        }
    }

    // Downsample back to original sample rate
    oversampler->processSamplesDown(block);

    // PHASE 4.2: MODULATION SYSTEM (WOW & FLUTTER)
    // ========================================

    // Get sample rate for LFO phase increment calculation
    const double sampleRate = getSampleRate();
    const double phaseIncrement = 1.0 / sampleRate;

    // Modulation depth scaling based on age parameter
    // age=0.0 → no modulation
    // age=0.5 → moderate tape wear
    // age=1.0 → heavily degraded tape
    const float wowDepthMs = 0.5f * age;      // 0 to 0.5ms modulation
    const float flutterDepthMs = 0.3f * age;  // 0 to 0.3ms modulation

    // Convert ms to samples
    const float wowDepthSamples = wowDepthMs * 0.001f * static_cast<float>(sampleRate);
    const float flutterDepthSamples = flutterDepthMs * 0.001f * static_cast<float>(sampleRate);

    // Process each sample for modulated delay
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Generate wow LFO (sine wave, 0.3 Hz)
        const float wowLFO = std::sin(2.0 * juce::MathConstants<double>::pi * wowPhase);

        // Generate flutter LFO (triangle wave, 7 Hz)
        // Triangle wave: 4*|phase - 0.5| - 1
        const float flutterLFO = 4.0f * std::abs(static_cast<float>(flutterPhase) - 0.5f) - 1.0f;

        // Combine modulations (additive)
        const float modulationSamples = wowLFO * wowDepthSamples + flutterLFO * flutterDepthSamples;

        // Process each channel
        for (int channel = 0; channel < numChannels; ++channel)
        {
            // Get pointers
            auto* channelData = buffer.getWritePointer(channel);
            auto* delayData = delayBuffer.getWritePointer(channel);

            // Calculate modulated read position (current write position minus delay)
            // Base delay is 5ms (half the buffer) to allow bidirectional modulation
            const float baseDelaySamples = delayBufferLength * 0.5f;
            float readPosition = delayWritePosition - baseDelaySamples + modulationSamples;

            // Wrap read position (handle negative values)
            while (readPosition < 0.0f)
                readPosition += delayBufferLength;
            while (readPosition >= delayBufferLength)
                readPosition -= delayBufferLength;

            // Linear interpolation for fractional delay
            const int readIndex1 = static_cast<int>(readPosition);
            const int readIndex2 = (readIndex1 + 1) % delayBufferLength;
            const float fraction = readPosition - readIndex1;

            const float sample1 = delayData[readIndex1];
            const float sample2 = delayData[readIndex2];
            const float delayedSample = sample1 + fraction * (sample2 - sample1);

            // Write current sample to delay buffer
            delayData[delayWritePosition] = channelData[sample];

            // Replace buffer sample with modulated delayed sample
            channelData[sample] = delayedSample;
        }

        // Advance write position (circular buffer)
        delayWritePosition = (delayWritePosition + 1) % delayBufferLength;

        // Advance LFO phases
        wowPhase += wowFrequency * phaseIncrement;
        if (wowPhase >= 1.0)
            wowPhase -= 1.0;

        flutterPhase += flutterFrequency * phaseIncrement;
        if (flutterPhase >= 1.0)
            flutterPhase -= 1.0;
    }

    // PHASE 4.3: DEGRADATION EFFECTS (DROPOUT & NOISE)
    // ========================================

    // DROPOUT: Random intermittent gain reduction events
    // Probability check once per buffer (not per sample for efficiency)
    const float dropoutProbabilityPerSecond = age * 0.05f;  // 0 to 5% chance per second at age=1.0
    const float dropoutProbabilityPerBuffer = dropoutProbabilityPerSecond * (numSamples / static_cast<float>(sampleRate));

    // Check if we should start a new dropout event
    if (!dropoutActive && dropoutRandom.nextFloat() < dropoutProbabilityPerBuffer)
    {
        dropoutActive = true;
        // Random duration: 10-50ms
        const float dropoutDurationMs = 10.0f + dropoutRandom.nextFloat() * 40.0f;
        dropoutSamplesRemaining = static_cast<int>(dropoutDurationMs * 0.001f * sampleRate);

        // Random depth: 6-20dB reduction (linear gain 0.5 to 0.1)
        const float dropoutDepthDB = -6.0f - dropoutRandom.nextFloat() * 14.0f;  // -6 to -20 dB
        dropoutTargetGain = juce::Decibels::decibelsToGain(dropoutDepthDB);

        // Set envelope to attack to target gain (10ms attack)
        dropoutEnvelope.reset(sampleRate, 0.01);  // 10ms attack
        dropoutEnvelope.setTargetValue(dropoutTargetGain);
    }

    // Apply dropout gain envelope (smooth to avoid clicks)
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Get current envelope gain
        const float envelopeGain = dropoutEnvelope.getNextValue();

        // Apply gain to all channels
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            channelData[sample] *= envelopeGain;
        }

        // Update dropout state
        if (dropoutActive)
        {
            dropoutSamplesRemaining--;
            if (dropoutSamplesRemaining <= 0)
            {
                // End dropout event - smooth release back to unity gain
                dropoutActive = false;
                dropoutEnvelope.reset(sampleRate, 0.03);  // 30ms release
                dropoutEnvelope.setTargetValue(1.0f);
            }
        }
    }

    // PINK NOISE: Tape hiss character
    // Generate pink noise using Paul Kellet's method and mix with processed signal
    const float noiseLevel = juce::Decibels::decibelsToGain(-50.0f + age * 20.0f);  // -50dB to -30dB range

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Generate pink noise using Paul Kellet's algorithm
        // This approximates 1/f spectrum (pink noise)
        float white = noiseRandom.nextFloat() * 2.0f - 1.0f;  // White noise -1 to +1

        pinkNoise_b0 = 0.99886f * pinkNoise_b0 + white * 0.0555179f;
        pinkNoise_b1 = 0.99332f * pinkNoise_b1 + white * 0.0750759f;
        pinkNoise_b2 = 0.96900f * pinkNoise_b2 + white * 0.1538520f;
        pinkNoise_b3 = 0.86650f * pinkNoise_b3 + white * 0.3104856f;
        pinkNoise_b4 = 0.55000f * pinkNoise_b4 + white * 0.5329522f;
        pinkNoise_b5 = -0.7616f * pinkNoise_b5 - white * 0.0168980f;

        float pink = pinkNoise_b0 + pinkNoise_b1 + pinkNoise_b2 + pinkNoise_b3 + pinkNoise_b4 + pinkNoise_b5 + pinkNoise_b6 + white * 0.5362f;
        pinkNoise_b6 = white * 0.115926f;

        // Simple high-shelf filter approximation for "hiss" character
        // Boost high frequencies by mixing with differentiated signal
        const float highShelfBoost = 0.3f;  // Boost amount
        float tapeHiss = pink * (1.0f + highShelfBoost);  // Simple approximation

        // Mix noise with processed signal (all channels)
        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);
            channelData[sample] += tapeHiss * noiseLevel;
        }
    }

    // Phase 4.4 (dry/wet mix) will be added in next phase
}

//==============================================================================
bool TapeAgeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TapeAgeAudioProcessor::createEditor()
{
    return new TapeAgeAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void TapeAgeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void TapeAgeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapeAgeAudioProcessor();
}
