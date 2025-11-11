#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout TapeAgeAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // drive - Tape saturation amount
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "drive", 1 },
        "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f, 1.0f),  // 0-100%, linear
        0.5f  // Default: 50%
    ));

    // age - Tape degradation amount
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "age", 1 },
        "Age",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f, 1.0f),  // 0-100%, linear
        0.25f  // Default: 25%
    ));

    // mix - Dry/wet blend
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "mix", 1 },
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f, 1.0f),  // 0-100%, linear
        1.0f  // Default: 100% wet
    ));

    return layout;
}

TapeAgeAudioProcessor::TapeAgeAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , oversampler(2, 1, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple)  // 2x oversampling, 1 stage, FIR filters
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

TapeAgeAudioProcessor::~TapeAgeAudioProcessor()
{
}

void TapeAgeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Prepare DSP spec
    currentSpec.sampleRate = sampleRate;
    currentSpec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    currentSpec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    // Phase 4.1: Prepare oversampling engine
    oversampler.initProcessing(static_cast<size_t>(samplesPerBlock));
    oversampler.reset();
}

void TapeAgeAudioProcessor::releaseResources()
{
    // Phase 4.1: Reset DSP components
    oversampler.reset();
}

void TapeAgeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Clear unused channels
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Phase 4.1: Core Saturation Processing
    // Processing chain:
    // 1. Read drive parameter and calculate gain
    // 2. Upsample 2x
    // 3. Apply tanh saturation manually (drive controls gain scaling)
    // 4. Downsample

    // Read drive parameter (0.0 to 1.0)
    auto* driveParam = parameters.getRawParameterValue("drive");
    float drive = driveParam->load();

    // Progressive curve mapping (architecture.md):
    // 0-30%: Very subtle (multiply by 1-2 before tanh)
    // 30-70%: Moderate warmth (multiply by 2-8)
    // 70-100%: Heavy saturation (multiply by 8-20)
    float gain;
    if (drive <= 0.3f)
    {
        // Subtle range: linear interpolation from 1 to 2
        gain = 1.0f + (drive / 0.3f) * 1.0f;
    }
    else if (drive <= 0.7f)
    {
        // Moderate range: linear interpolation from 2 to 8
        gain = 2.0f + ((drive - 0.3f) / 0.4f) * 6.0f;
    }
    else
    {
        // Heavy range: linear interpolation from 8 to 20
        gain = 8.0f + ((drive - 0.7f) / 0.3f) * 12.0f;
    }

    // Create AudioBlock for DSP processing
    juce::dsp::AudioBlock<float> block(buffer);

    // Upsample
    auto oversampledBlock = oversampler.processSamplesUp(block);

    // Apply tanh saturation manually in oversampled domain
    for (size_t channel = 0; channel < oversampledBlock.getNumChannels(); ++channel)
    {
        auto* channelData = oversampledBlock.getChannelPointer(channel);
        for (size_t sample = 0; sample < oversampledBlock.getNumSamples(); ++sample)
        {
            channelData[sample] = std::tanh(gain * channelData[sample]);
        }
    }

    // Downsample back to original sample rate
    oversampler.processSamplesDown(block);
}

juce::AudioProcessorEditor* TapeAgeAudioProcessor::createEditor()
{
    return new TapeAgeAudioProcessorEditor(*this);
}

void TapeAgeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TapeAgeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Factory function
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapeAgeAudioProcessor();
}
