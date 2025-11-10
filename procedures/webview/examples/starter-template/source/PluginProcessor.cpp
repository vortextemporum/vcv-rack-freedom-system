#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"

// DOC: 02-project-setup.md # PluginProcessor Implementation

AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
    #if !JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
    #endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    // DOC: 04-parameter-binding.md # Initialize APVTS
    state{*this, nullptr, "STATE", createParameterLayout(parameters)}
{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

void AudioPluginAudioProcessor::prepareToPlay(double, int) {
    // Initialize audio processing here
}

void AudioPluginAudioProcessor::releaseResources() {
    // Release resources when playback stops
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // Support mono or stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    // Input must match output
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages) {
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // Clear extra output channels
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // DOC: 04-parameter-binding.md # Use Parameter in Processing
    // PATTERN: Apply gain from parameter
    buffer.applyGain(parameters.gain->get());
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
    return new AudioPluginAudioProcessorEditor(*this);
}

void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    // Save parameters to memory block
    auto stateTree = state.copyState();
    std::unique_ptr<juce::XmlElement> xml(stateTree.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    // Restore parameters from memory block
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(state.state.getType()))
        state.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// DOC: 04-parameter-binding.md # Create Parameters
juce::AudioProcessorValueTreeState::ParameterLayout
AudioPluginAudioProcessor::createParameterLayout(Parameters& parameters) {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // PATTERN: Create float parameter with normalizable range
    auto gainParam = std::make_unique<juce::AudioParameterFloat>(
        id::GAIN,                                    // Parameter ID
        "Gain",                                      // Name
        juce::NormalisableRange<float>{
            0.0f,   // min
            1.0f,   // max
            0.01f,  // step
            0.9f    // skew (for perceptual scaling)
        },
        1.0f  // default value
    );

    parameters.gain = gainParam.get();  // Store raw pointer
    layout.add(std::move(gainParam));

    return layout;
}

// CRITICAL: Required by JUCE plugin format
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new AudioPluginAudioProcessor();
}
