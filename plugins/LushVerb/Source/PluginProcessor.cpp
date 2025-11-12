#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout LushVerbAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // SIZE - Reverb tail length (0.5-20.0s, default 2.5s, logarithmic)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "SIZE", 1 },
        "Size",
        juce::NormalisableRange<float>(0.5f, 20.0f, 0.1f, 0.3f),  // Logarithmic skew for time-based parameter
        2.5f,
        "s",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + "s"; },
        [](const juce::String& text) { return text.dropLastCharacters(1).getFloatValue(); }
    ));

    // DAMPING - High-frequency rolloff (0-100%, default 30%, linear)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "DAMPING", 1 },
        "Damping",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        30.0f,
        "%",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value)) + "%"; },
        [](const juce::String& text) { return text.dropLastCharacters(1).getFloatValue(); }
    ));

    // SHIMMER - +1 octave pitch-shifted signal amount (0-100%, default 30%, linear)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "SHIMMER", 1 },
        "Shimmer",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        30.0f,
        "%",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value)) + "%"; },
        [](const juce::String& text) { return text.dropLastCharacters(1).getFloatValue(); }
    ));

    // MIX - Dry/wet blend (0-100%, default 30%, linear)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "MIX", 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        30.0f,
        "%",
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(static_cast<int>(value)) + "%"; },
        [](const juce::String& text) { return text.dropLastCharacters(1).getFloatValue(); }
    ));

    return layout;
}

LushVerbAudioProcessor::LushVerbAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

LushVerbAudioProcessor::~LushVerbAudioProcessor()
{
}

void LushVerbAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Store sample rate for LFO calculations
    currentSampleRate = sampleRate;

    // Prepare DSP spec (Pattern #17: Modern juce::dsp API)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    // Prepare DSP components
    reverb.prepare(spec);
    dryWetMixer.prepare(spec);

    // Phase 4.2: Prepare modulation delay line
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(static_cast<int>(sampleRate * 0.05)); // 50ms max delay
    delayLine.reset();

    // Initialize LFO phases
    leftLFOPhase = 0.0f;
    rightLFOPhase = 0.0f;

    // Reset components to initial state
    reverb.reset();
    dryWetMixer.reset();
}

void LushVerbAudioProcessor::releaseResources()
{
    // Cleanup will be added in Stage 4
}

void LushVerbAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Clear unused channels
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Read parameters (atomic, real-time safe)
    auto* sizeParam = parameters.getRawParameterValue("SIZE");
    auto* dampingParam = parameters.getRawParameterValue("DAMPING");
    auto* mixParam = parameters.getRawParameterValue("MIX");

    float sizeValue = sizeParam->load();       // 0.5-20.0s
    float dampingValue = dampingParam->load(); // 0-100%
    float mixValue = mixParam->load();         // 0-100%

    // Map SIZE to reverb roomSize (0.3-1.0 range)
    // SIZE parameter: 0.5s (small) → 20.0s (huge)
    // Reverb roomSize: 0.3 (tight) → 1.0 (massive)
    float roomSize = juce::jmap(sizeValue, 0.5f, 20.0f, 0.3f, 1.0f);

    // Map DAMPING from percentage (0-100%) to normalized (0.0-1.0)
    float damping = dampingValue / 100.0f;

    // Set reverb parameters
    juce::dsp::Reverb::Parameters reverbParams;
    reverbParams.roomSize = roomSize;
    reverbParams.damping = damping;
    reverbParams.wetLevel = 1.0f;  // DryWetMixer handles dry/wet blend
    reverbParams.dryLevel = 0.0f;  // DryWetMixer handles dry/wet blend
    reverbParams.width = 1.0f;     // Full stereo width
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters(reverbParams);

    // Set dry/wet mix (percentage to normalized)
    dryWetMixer.setWetMixProportion(mixValue / 100.0f);

    // Process audio using modern juce::dsp API (Pattern #17)
    juce::dsp::AudioBlock<float> block(buffer);

    // Push dry signal to mixer
    dryWetMixer.pushDrySamples(block);

    // Process wet signal through reverb
    juce::dsp::ProcessContextReplacing<float> context(block);
    reverb.process(context);

    // Phase 4.2: Apply modulation to reverb tail (post-reverb, pre-dry/wet mix)
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // LFO frequencies (hardcoded per spec)
    const float leftLFOFreq = 0.3f;   // Hz
    const float rightLFOFreq = 0.5f;  // Hz
    const float modulationDepth = 0.03f; // ±3% pitch variation

    // Constants
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float baseDelaySamples = 10.0f; // Center delay (10 samples)

    // Process each channel with its own LFO
    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        float& lfoPhase = (channel == 0) ? leftLFOPhase : rightLFOPhase;
        const float lfoFreq = (channel == 0) ? leftLFOFreq : rightLFOFreq;

        // Calculate phase increment
        const float phaseIncrement = (lfoFreq * twoPi) / static_cast<float>(currentSampleRate);

        // Process each sample
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Calculate LFO output (sine wave)
            float lfoOutput = std::sin(lfoPhase);

            // Modulate delay time (±3% around base delay)
            float delayTimeSamples = baseDelaySamples + (lfoOutput * modulationDepth * baseDelaySamples);

            // Write current sample to delay line
            delayLine.pushSample(channel, channelData[sample]);

            // Read modulated sample from delay line
            float delayedSample = delayLine.popSample(channel, delayTimeSamples);

            // Replace with modulated signal
            channelData[sample] = delayedSample;

            // Increment LFO phase
            lfoPhase += phaseIncrement;

            // Wrap phase to prevent accumulation
            if (lfoPhase >= twoPi)
                lfoPhase -= twoPi;
        }
    }

    // Mix dry and wet signals
    dryWetMixer.mixWetSamples(block);
}

juce::AudioProcessorEditor* LushVerbAudioProcessor::createEditor()
{
    return new LushVerbAudioProcessorEditor(*this);
}

void LushVerbAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void LushVerbAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LushVerbAudioProcessor();
}
