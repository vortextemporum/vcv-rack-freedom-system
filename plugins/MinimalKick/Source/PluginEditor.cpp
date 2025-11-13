#include "PluginEditor.h"
#include "BinaryData.h"

MinimalKickAudioProcessorEditor::MinimalKickAudioProcessorEditor(MinimalKickAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Create WebView with resource provider
    webView = std::make_unique<juce::WebBrowserComponent>(
        juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            .withResourceProvider([this](const auto& url) { return getResource(url); })
    );

    addAndMakeVisible(*webView);

    // Set editor size (730×280px from mockup)
    setSize(730, 280);
    setResizable(false, false);

    // Load UI
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
}

MinimalKickAudioProcessorEditor::~MinimalKickAudioProcessorEditor()
{
}

void MinimalKickAudioProcessorEditor::paint(juce::Graphics& g)
{
    // WebView handles all painting
}

void MinimalKickAudioProcessorEditor::resized()
{
    // Fill entire editor area
    webView->setBounds(getLocalBounds());
}

std::optional<juce::WebBrowserComponent::Resource>
MinimalKickAudioProcessorEditor::getResource(const juce::String& url)
{
    // Helper lambda for creating resource vectors
    auto makeVector = [](const char* data, int size) {
        return std::vector<std::byte>(
            reinterpret_cast<const std::byte*>(data),
            reinterpret_cast<const std::byte*>(data) + size
        );
    };

    // Pattern #8: Explicit URL mapping (NO generic loops)
    if (url == "/" || url == "/index.html") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::index_html, BinaryData::index_htmlSize),
            juce::String("text/html")
        };
    }

    if (url == "/js/juce/index.js") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::index_js, BinaryData::index_jsSize),
            juce::String("text/javascript")
        };
    }

    if (url == "/js/juce/check_native_interop.js") {
        return juce::WebBrowserComponent::Resource {
            makeVector(BinaryData::check_native_interop_js, BinaryData::check_native_interop_jsSize),
            juce::String("text/javascript")
        };
    }

    return std::nullopt;  // 404
}
