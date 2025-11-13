#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>  // Required for WebBrowserComponent
#include "PluginProcessor.h"

class MinimalKickAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit MinimalKickAudioProcessorEditor(MinimalKickAudioProcessor&);
    ~MinimalKickAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    MinimalKickAudioProcessor& processorRef;

    // WebView component (Pattern #11: std::unique_ptr)
    std::unique_ptr<juce::WebBrowserComponent> webView;

    // Resource provider for serving HTML/JS files
    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MinimalKickAudioProcessorEditor)
};
