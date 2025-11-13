#include "PluginEditor.h"

AutoClipAudioProcessorEditor::AutoClipAudioProcessorEditor(AutoClipAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(300, 500);
}

AutoClipAudioProcessorEditor::~AutoClipAudioProcessorEditor()
{
}

void AutoClipAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("AutoClip - Stage 2", getLocalBounds(), juce::Justification::centred, 1);

    g.setFont(14.0f);
    g.drawFittedText("2 parameters implemented",
                     getLocalBounds().reduced(20).removeFromBottom(30),
                     juce::Justification::centred, 1);
}

void AutoClipAudioProcessorEditor::resized()
{
    // Layout will be added in Stage 4 (GUI)
}
