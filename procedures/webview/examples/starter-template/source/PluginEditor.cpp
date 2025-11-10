#include "PluginEditor.h"
#include "ParameterIDs.h"
#include <WebViewFiles.h>

// DOC: 02-project-setup.md # PluginEditor Implementation

namespace {
    // DOC: 07-distribution.md # Helper Functions
    // PATTERN: Convert InputStream to byte vector
    std::vector<std::byte> streamToVector(juce::InputStream& stream) {
        const auto sizeInBytes = static_cast<size_t>(stream.getTotalLength());
        std::vector<std::byte> result(sizeInBytes);
        stream.setPosition(0);
        stream.read(result.data(), result.size());
        return result;
    }

    // PATTERN: Get MIME type for file extension
    const char* getMimeForExtension(const juce::String& extension) {
        static const std::unordered_map<juce::String, const char*> mimeMap = {
            {{"html"}, "text/html"},
            {{"js"},   "text/javascript"},
            {{"css"},  "text/css"},
            {{"json"}, "application/json"}
        };

        if (const auto it = mimeMap.find(extension.toLowerCase()); it != mimeMap.end())
            return it->second;

        return "application/octet-stream";
    }

    // DOC: 07-distribution.md # Read from Zip
    // PATTERN: Get file from embedded zip
    std::vector<std::byte> getWebViewFileAsBytes(const juce::String& filepath) {
        juce::MemoryInputStream zipStream{
            webview_files::webview_files_zip,
            webview_files::webview_files_zipSize,
            false  // Don't copy data
        };

        juce::ZipFile zipFile{zipStream};

        // Find entry with prefix (e.g., "public/index.html")
        if (auto* zipEntry = zipFile.getEntry(ZIPPED_FILES_PREFIX + filepath)) {
            const std::unique_ptr<juce::InputStream> entryStream{
                zipFile.createStreamForEntry(*zipEntry)
            };

            if (entryStream != nullptr)
                return streamToVector(*entryStream);
        }

        return {};  // File not found
    }

    // For development: use local server for hot reloading
    constexpr auto LOCAL_DEV_SERVER_ADDRESS = "http://127.0.0.1:8080";
}

// DOC: 04-parameter-binding.md # Initialization Order
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      // CRITICAL: Initialize relay BEFORE webView
      webGainRelay{id::GAIN.getParamID()},
      // DOC: 02-project-setup.md # WebView Setup
      webView(juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(
              juce::WebBrowserComponent::Options::WinWebView2{}
                  .withBackgroundColour(juce::Colours::white)
                  // CRITICAL: Avoids permission issues in some DAWs
                  .withUserDataFolder(juce::File::getSpecialLocation(
                      juce::File::SpecialLocationType::tempDirectory)))
          // DOC: 03-communication-patterns.md # Native Integration
          .withNativeIntegrationEnabled()
          // DOC: 03-communication-patterns.md # Resource Provider
          .withResourceProvider(
              [this](const auto& url) { return getResource(url); },
              // DOC: 06-development-workflow.md # CORS for Dev Server
              juce::URL{LOCAL_DEV_SERVER_ADDRESS}.getOrigin())
          // DOC: 04-parameter-binding.md # Pass Relay Options
          .withOptionsFrom(webGainRelay)),
      // CRITICAL: Initialize attachment AFTER webView
      webGainSliderAttachment{
          *processorRef.getState().getParameter(id::GAIN.getParamID()),
          webGainRelay,
          nullptr  // No undo manager
      }
{
    addAndMakeVisible(webView);

    // PRODUCTION: Serve from embedded files
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    // DEVELOPMENT: Uncomment to use dev server for hot reloading
    // webView.goToURL(LOCAL_DEV_SERVER_ADDRESS);

    setSize(400, 300);
    setResizable(true, true);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

void AudioPluginAudioProcessorEditor::resized() {
    webView.setBounds(getBounds());
}

// DOC: 03-communication-patterns.md # Resource Provider Implementation
auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    // Parse requested resource path
    const auto resourceToRetrieve =
        url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

    // DOC: 07-distribution.md # Serve Embedded Files
    // PATTERN: Read file from embedded zip
    const auto resource = getWebViewFileAsBytes(resourceToRetrieve);
    if (!resource.empty()) {
        const auto extension =
            resourceToRetrieve.fromLastOccurrenceOf(".", false, false);
        return Resource{std::move(resource), getMimeForExtension(extension)};
    }

    return std::nullopt;  // 404 Not Found
}
