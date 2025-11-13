#pragma once
#include "PluginProcessor.h"

class AutoClipAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AutoClipAudioProcessorEditor(AutoClipAudioProcessor&);
    ~AutoClipAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AutoClipAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AutoClipAudioProcessorEditor)
};
