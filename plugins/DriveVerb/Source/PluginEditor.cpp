#include "PluginEditor.h"

DriveVerbAudioProcessorEditor::DriveVerbAudioProcessorEditor(DriveVerbAudioProcessor& p)
    : AudioProcessorEditor(p), processorRef(p)
{
    setSize(400, 300);
}

DriveVerbAudioProcessorEditor::~DriveVerbAudioProcessorEditor()
{
}

void DriveVerbAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("DriveVerb - Stage 3", getLocalBounds(), juce::Justification::centred, 1);

    g.setFont(14.0f);
    g.drawFittedText("6 parameters implemented",
                     getLocalBounds().reduced(20).removeFromBottom(30),
                     juce::Justification::centred, 1);
}

void DriveVerbAudioProcessorEditor::resized()
{
    // Layout will be added in Stage 5
}
