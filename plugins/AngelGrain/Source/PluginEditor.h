#pragma once
#include "PluginProcessor.h"

class AngelGrainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AngelGrainAudioProcessorEditor(AngelGrainAudioProcessor&);
    ~AngelGrainAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AngelGrainAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AngelGrainAudioProcessorEditor)
};
