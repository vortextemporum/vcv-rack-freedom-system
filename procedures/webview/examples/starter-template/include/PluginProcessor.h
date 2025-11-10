#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// DOC: 02-project-setup.md # PluginProcessor Header

class AudioPluginAudioProcessor : public juce::AudioProcessor {
public:
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // DOC: 04-parameter-binding.md # Expose State Accessor
    juce::AudioProcessorValueTreeState& getState() { return state; }

private:
    // DOC: 04-parameter-binding.md # Parameters Struct Pattern
    struct Parameters {
        juce::AudioParameterFloat* gain{nullptr};
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout
    createParameterLayout(Parameters&);

    Parameters parameters;
    juce::AudioProcessorValueTreeState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
