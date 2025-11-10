#pragma once

#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

// DOC: 02-project-setup.md # PluginEditor Header

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void resized() override;

private:
    // DOC: 03-communication-patterns.md # Resource Provider
    using Resource = juce::WebBrowserComponent::Resource;
    std::optional<Resource> getResource(const juce::String& url) const;

    AudioPluginAudioProcessor& processorRef;

    // DOC: 04-parameter-binding.md # WebView Relay + Attachment
    // CRITICAL: Relay must be declared BEFORE webView
    juce::WebSliderRelay webGainRelay;

    // WebView component
    juce::WebBrowserComponent webView;

    // CRITICAL: Attachment must be declared AFTER webView
    juce::WebSliderParameterAttachment webGainSliderAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
