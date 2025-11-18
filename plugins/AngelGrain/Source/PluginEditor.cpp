#include "PluginEditor.h"

AngelGrainAudioProcessorEditor::AngelGrainAudioProcessorEditor(AngelGrainAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(600, 400);
}

AngelGrainAudioProcessorEditor::~AngelGrainAudioProcessorEditor()
{
}

void AngelGrainAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("AngelGrain - Stage 1", getLocalBounds(), juce::Justification::centred, 1);

    g.setFont(14.0f);
    g.drawFittedText("7 parameters implemented",
                     getLocalBounds().reduced(20).removeFromBottom(30),
                     juce::Justification::centred, 1);
}

void AngelGrainAudioProcessorEditor::resized()
{
}
