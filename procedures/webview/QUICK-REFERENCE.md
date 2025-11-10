# Quick Reference - JUCE WebView Cheat Sheet

## Essential CMake Configuration

```cmake
# Enable WebView in juce_add_plugin()
NEEDS_WEB_VIEW2 TRUE  # Windows only

# Compile definitions
target_compile_definitions(YourPlugin PUBLIC
    JUCE_WEB_BROWSER=1                                # Enable WebView
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1      # Windows: static link
)

# Link modules
target_link_libraries(YourPlugin PRIVATE
    juce::juce_gui_extra      # Required for WebBrowserComponent
    juce::juce_dsp            # Optional: for audio processing
)

# Copy JUCE frontend library
file(COPY
    "${JUCE_MODULES_DIR}/juce_gui_extra/native/javascript/"
    DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/ui/public/js/juce/"
)
```

---

## Basic WebView Setup

### C++ Constructor

```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView(juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withNativeIntegrationEnabled()  // REQUIRED for JUCE library
          .withResourceProvider([this](const auto& url) { return getResource(url); }))
{
    addAndMakeVisible(webView);
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    setSize(800, 600);
}
```

### JavaScript Import

```javascript
import * as Juce from "./juce/index.js";

// Verify connection
console.log(window.__JUCE__.backend);  // Should exist
```

---

## Communication Quick Reference

### C++ → JavaScript

| Method | Use Case | C++ Code | JavaScript Code |
|--------|----------|----------|-----------------|
| **Evaluate** | Run JS code | `webView.evaluateJavascript("alert('Hi');")` | N/A |
| **Emit Event** | Send notification | `webView.emitEventIfBrowserIsVisible("update", value)` | `window.__JUCE__.backend.addEventListener("update", callback)` |
| **Native Function** | Call from JS | `.withNativeFunction("myFunc", callback)` | `const f = Juce.getNativeFunction("myFunc"); f(args)` |
| **Init Data** | Startup values | `.withInitialisationData("key", value)` | `window.__JUCE__.initialisationData.key` |
| **Resource Provider** | Serve files/data | `return Resource{bytes, mime};` | `fetch(Juce.getBackendResourceAddress("file.json"))` |

### JavaScript → C++

| Method | Use Case | JavaScript Code | C++ Code |
|--------|----------|-----------------|----------|
| **Native Function** | Call C++ | `await Juce.getNativeFunction("func")(args)` | `.withNativeFunction("func", callback)` |
| **Emit Event** | Send notification | `window.__JUCE__.backend.emitEvent("event", data)` | `.withEventListener("event", callback)` |

---

## Parameter Binding

### Float Parameter (Slider)

**C++:**
```cpp
// PluginEditor.h
juce::WebSliderRelay webGainRelay;
juce::WebSliderParameterAttachment webGainSliderAttachment;

// PluginEditor.cpp - Constructor initializer list
webGainRelay{id::GAIN.getParamID()},
webView(Options{}.withOptionsFrom(webGainRelay)),
webGainSliderAttachment{
    *processorRef.getState().getParameter(id::GAIN.getParamID()),
    webGainRelay,
    nullptr
}
```

**JavaScript:**
```javascript
const slider = document.getElementById("gainSlider");
const sliderState = Juce.getSliderState("GAIN");

slider.oninput = () => sliderState.setNormalisedValue(slider.value);
sliderState.valueChangedEvent.addListener(() => {
    slider.value = sliderState.getNormalisedValue();
});
```

### Bool Parameter (Checkbox)

**C++:**
```cpp
// PluginEditor.h
juce::WebToggleButtonRelay webBypassRelay;
juce::WebToggleButtonParameterAttachment webBypassToggleAttachment;

// Constructor
webBypassRelay{id::BYPASS.getParamID()},
webView(Options{}.withOptionsFrom(webBypassRelay)),
webBypassToggleAttachment{
    *processorRef.getState().getParameter(id::BYPASS.getParamID()),
    webBypassRelay,
    nullptr
}
```

**JavaScript:**
```javascript
const checkbox = document.getElementById("bypassCheckbox");
const toggleState = Juce.getToggleState("BYPASS");

checkbox.oninput = () => toggleState.setValue(checkbox.checked);
toggleState.valueChangedEvent.addListener(() => {
    checkbox.checked = toggleState.getValue();
});
```

### Choice Parameter (ComboBox)

**C++:**
```cpp
// PluginEditor.h
juce::WebComboBoxRelay webDistortionTypeRelay;
juce::WebComboBoxParameterAttachment webDistortionTypeComboBoxAttachment;

// Constructor
webDistortionTypeRelay{id::DISTORTION_TYPE.getParamID()},
webView(Options{}.withOptionsFrom(webDistortionTypeRelay)),
webDistortionTypeComboBoxAttachment{
    *processorRef.getState().getParameter(id::DISTORTION_TYPE.getParamID()),
    webDistortionTypeRelay,
    nullptr
}
```

**JavaScript:**
```javascript
const comboBox = document.getElementById("distortionTypeComboBox");
const comboBoxState = Juce.getComboBoxState("DISTORTION_TYPE");

// Populate options
comboBoxState.propertiesChangedEvent.addListener(() => {
    comboBox.innerHTML = "";
    comboBoxState.properties.choices.forEach(choice => {
        comboBox.innerHTML += `<option>${choice}</option>`;
    });
});

// Bind selection
comboBox.oninput = () => {
    comboBoxState.setChoiceIndex(comboBox.selectedIndex);
};
comboBoxState.valueChangedEvent.addListener(() => {
    comboBox.selectedIndex = comboBoxState.getChoiceIndex();
});
```

---

## Audio Visualization Pattern

### C++ Side

```cpp
// PluginProcessor.h
std::atomic<float> outputLevelLeft{-100.0f};  // dB

// PluginEditor.h
class PluginEditor : private juce::Timer {
    void timerCallback() override;
};

// PluginEditor.cpp
startTimer(60);  // 60ms updates

void PluginEditor::timerCallback() {
    webView.emitEventIfBrowserIsVisible("audioUpdate", juce::var{});
}

auto PluginEditor::getResource(const juce::String& url) const -> std::optional<Resource> {
    if (url.contains("audioData.json")) {
        juce::DynamicObject::Ptr data{new juce::DynamicObject{}};
        data->setProperty("level", processorRef.outputLevelLeft.load());

        const auto jsonString = juce::JSON::toString(data.get());
        juce::MemoryInputStream stream{jsonString.getCharPointer(),
                                       jsonString.getNumBytesAsUTF8(), false};
        return Resource{streamToVector(stream), "application/json"};
    }
    // ... handle other resources
}
```

### JavaScript Side

```javascript
window.__JUCE__.backend.addEventListener("audioUpdate", async () => {
    const response = await fetch(Juce.getBackendResourceAddress("audioData.json"));
    const data = await response.json();
    updateVisualization(data.level);
});
```

---

## Development Workflow

### Start Local Dev Server

```bash
cd plugin/ui/public
npx http-server  # Starts at http://127.0.0.1:8080
```

### Point WebView to Dev Server

```cpp
// Development (hot reload enabled)
webView.goToURL("http://127.0.0.1:8080");

// CRITICAL: Allow dev server to fetch from C++ backend
.withResourceProvider(
    [this](const auto& url) { return getResource(url); },
    juce::URL{"http://127.0.0.1:8080"}.getOrigin())  // CORS allowance
```

### Hard Refresh (Clear Cache)

- **Windows/Linux:** Ctrl + Shift + R
- **macOS:** Cmd + Shift + R

---

## Distribution (Embed Web Files)

### CMake: Zip Web Files

```cmake
# Get UI directory path
get_filename_component(PUBLIC_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/public
    ABSOLUTE
)

# Create zip
set(WEBVIEW_FILES_ZIP_PATH ${CMAKE_BINARY_DIR}/webview_files.zip)
file(ARCHIVE_CREATE
    OUTPUT ${WEBVIEW_FILES_ZIP_PATH}
    PATHS ${PUBLIC_PATH}
    FORMAT zip
    VERBOSE
)

# Embed as binary data
juce_add_binary_data(WebViewFiles
    HEADER_NAME WebViewFiles.h
    NAMESPACE webview_files
    SOURCES ${WEBVIEW_FILES_ZIP_PATH}
)

# Link to plugin
target_link_libraries(YourPlugin PRIVATE WebViewFiles)

# Define zip prefix
target_compile_definitions(YourPlugin PRIVATE
    ZIPPED_FILES_PREFIX="public/"
)
```

### C++: Read from Zip

```cpp
#include <WebViewFiles.h>

std::vector<std::byte> getWebViewFileAsBytes(const juce::String& filepath) {
    juce::MemoryInputStream zipStream{
        webview_files::webview_files_zip,
        webview_files::webview_files_zipSize,
        false
    };

    juce::ZipFile zipFile{zipStream};

    if (auto* zipEntry = zipFile.getEntry(ZIPPED_FILES_PREFIX + filepath)) {
        const std::unique_ptr<juce::InputStream> entryStream{
            zipFile.createStreamForEntry(*zipEntry)
        };
        return streamToVector(*entryStream);
    }

    return {};
}

auto PluginEditor::getResource(const juce::String& url) const -> std::optional<Resource> {
    const auto resourceToRetrieve =
        url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

    const auto resource = getWebViewFileAsBytes(resourceToRetrieve);
    if (!resource.empty()) {
        const auto extension = resourceToRetrieve.fromLastOccurrenceOf(".", false, false);
        return Resource{std::move(resource), getMimeForExtension(extension)};
    }

    return std::nullopt;
}
```

### Switch to Resource Provider

```cpp
// Production build
webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

// NOT dev server:
// webView.goToURL("http://127.0.0.1:8080");
```

---

## Common MIME Types

```cpp
const char* getMimeForExtension(const juce::String& extension) {
    static const std::unordered_map<juce::String, const char*> mimeMap = {
        {{"html"}, "text/html"},
        {{"js"},   "text/javascript"},
        {{"css"},  "text/css"},
        {{"json"}, "application/json"},
        {{"png"},  "image/png"},
        {{"jpg"},  "image/jpeg"},
        {{"svg"},  "image/svg+xml"},
        {{"woff2"},"font/woff2"}
    };
    if (const auto it = mimeMap.find(extension.toLowerCase()); it != mimeMap.end())
        return it->second;
    return "application/octet-stream";
}
```

---

## Platform Runtime Requirements

| Platform | Requirement | Installation |
|----------|-------------|--------------|
| **Windows** | WebView2 Runtime | Bundle installer with plugin |
| **macOS** | Built-in (WKWebView) | None |
| **Linux** | libwebkit2gtk-4.1 | `sudo apt install libwebkit2gtk-4.1-0` |
| **iOS** | Built-in (WKWebView) | None |
| **Android** | Built-in (WebView) | None |

---

## Debugging Commands

### JavaScript Console

```javascript
// Check JUCE object
console.log(window.__JUCE__);
console.log(window.__JUCE__.backend);
console.log(window.__JUCE__.initialisationData);

// Check parameter state
const state = Juce.getSliderState("GAIN");
console.log(state.properties);
console.log(state.getNormalisedValue());
```

### C++ Logging

```cpp
// Log resource requests
auto getResource(const juce::String& url) const {
    std::cout << "Resource requested: " << url << std::endl;
    // ...
}

// Log event emissions
webView.emitEventIfBrowserIsVisible("event", value);
std::cout << "Emitted event: event" << std::endl;

// Log JavaScript evaluation
webView.evaluateJavascript("console.log('Test');",
    [](juce::WebBrowserComponent::EvaluationResult result) {
        if (result.getResult()) {
            std::cout << "Result: " << result.getResult()->toString() << std::endl;
        } else {
            std::cout << "Error: " << result.getError()->message << std::endl;
        }
    });
```

---

## Installation Paths

### macOS

```bash
# VST3
~/Library/Audio/Plug-Ins/VST3/YourPlugin.vst3

# AU
~/Library/Audio/Plug-Ins/Components/YourPlugin.component
```

### Windows

```
# VST3
C:\Program Files\Common Files\VST3\YourPlugin.vst3
```

### Linux

```bash
# VST3
~/.vst3/YourPlugin.vst3
```

---

## Common Issues Quick Fix

| Issue | Quick Fix |
|-------|-----------|
| Blank WebView | Check resource provider returns files; verify dev server running |
| CORS error | Add `allowedOriginIn` to `.withResourceProvider()` |
| "__JUCE__ undefined" | Add `.withNativeIntegrationEnabled()` to options |
| Parameter not syncing | Check initialization order: relay → webView → attachment |
| Changes don't appear | Hard refresh: Ctrl+Shift+R (Win) / Cmd+Shift+R (Mac) |
| WebView2 not found | Run NuGet install commands from CMake output (Windows) |
| 404 for JUCE library | Copy JUCE frontend lib to `ui/public/js/juce/` |

---

## Initialization Order Template

```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(...)
    : AudioProcessorEditor(&p),
      processorRef(p),
      // 1. C++ component attachments (optional)
      gainSliderAttachment{...},
      bypassButtonAttachment{...},
      // 2. WebView relays (BEFORE webView)
      webGainRelay{id::GAIN.getParamID()},
      webBypassRelay{id::BYPASS.getParamID()},
      // 3. WebView component (references relays)
      webView(Options{}
          .withOptionsFrom(webGainRelay)
          .withOptionsFrom(webBypassRelay)),
      // 4. WebView attachments (AFTER webView)
      webGainSliderAttachment{..., webGainRelay, ...},
      webBypassToggleAttachment{..., webBypassRelay, ...}
{
    // Constructor body
}
```

---

## Helper Functions

### Stream to Vector

```cpp
std::vector<std::byte> streamToVector(juce::InputStream& stream) {
    const auto sizeInBytes = static_cast<size_t>(stream.getTotalLength());
    std::vector<std::byte> result(sizeInBytes);
    stream.setPosition(0);
    stream.read(result.data(), result.size());
    return result;
}
```

### MIME Type Lookup

```cpp
const char* getMimeForExtension(const juce::String& extension);  // See above
```

---

## Build Commands

```bash
# Generate project
cmake --preset default  # or: release, Xcode, vs

# Build
cmake --build --preset default

# Build release
cmake --preset release
cmake --build --preset release
```

---

## Next Steps

- **Full docs:** See [README.md](./README.md) for complete documentation index
- **New to WebView:** Start with [01-overview.md](./01-overview.md)
- **Specific topic:** Check README for topic-specific guides
