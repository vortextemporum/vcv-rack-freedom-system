#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Parameter Layout (BEFORE constructor)
//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout AutoClipAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // clipThreshold - Float (0-100%, linear)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "clipThreshold", 1 },
        "Clip Threshold",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f, 1.0f),  // min, max, step, skew
        0.0f,
        "%"
    ));

    // soloClipped - Bool (default: false)
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "soloClipped", 1 },
        "Clip Solo",
        false
    ));

    return layout;
}

//==============================================================================
// Constructor
//==============================================================================
AutoClipAudioProcessor::AutoClipAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "Parameters", createParameterLayout())
{
}

AutoClipAudioProcessor::~AutoClipAudioProcessor()
{
}

//==============================================================================
// Audio Processing
//==============================================================================
void AutoClipAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // DSP initialization will be added in Stage 3
    juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void AutoClipAudioProcessor::releaseResources()
{
    // DSP cleanup will be added in Stage 3
}

void AutoClipAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    // Parameter access (for Stage 3 DSP implementation):
    // auto* clipThresholdParam = parameters.getRawParameterValue("clipThreshold");
    // float clipThresholdValue = clipThresholdParam->load();  // Atomic read (real-time safe)
    //
    // auto* soloClippedParam = parameters.getRawParameterValue("soloClipped");
    // bool soloClippedValue = soloClippedParam->load() > 0.5f;

    // Pass-through for Stage 2 (DSP implementation happens in Stage 3)
    // Audio routing is already handled by JUCE
}

int AutoClipAudioProcessor::getLatencySamples() const
{
    // 5ms lookahead at 48kHz = 240 samples (placeholder)
    // Stage 3 will calculate dynamically: static_cast<int>(0.005 * getSampleRate())
    return 240;
}

//==============================================================================
// Editor Creation
//==============================================================================
juce::AudioProcessorEditor* AutoClipAudioProcessor::createEditor()
{
    return new AutoClipAudioProcessorEditor(*this);
}

//==============================================================================
// State Management
//==============================================================================
void AutoClipAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AutoClipAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// Factory function
//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AutoClipAudioProcessor();
}
