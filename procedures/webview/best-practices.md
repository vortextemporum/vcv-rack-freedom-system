# JUCE 8 WebView Best Practices

Critical patterns and practices to ensure WebView plugins work reliably. Based on real-world issues and the working example code.

---

## Table of Contents

- [Member Declaration Order (Critical)](#member-declaration-order-critical)
- [Constructor Initialization Order](#constructor-initialization-order)
- [WebView Options Configuration](#webview-options-configuration)
- [Resource Provider Implementation](#resource-provider-implementation)
- [Development vs Production Builds](#development-vs-production-builds)
- [Error Handling Patterns](#error-handling-patterns)
- [Testing Checklist](#testing-checklist)
- [Component Lifecycle Patterns](#component-lifecycle-patterns)
- [JavaScript Structure Patterns](#javascript-structure-patterns)
- [CMake Configuration Essentials](#cmake-configuration-essentials)
- [File Structure Organization](#file-structure-organization)
- [Anti-Patterns to Avoid](#anti-patterns-to-avoid)
- [Summary: Golden Rules](#summary-golden-rules)

---

## Member Declaration Order (Critical)

**This is the #1 cause of release build crashes.**

### The Rule

Members are destroyed in **reverse order** of declaration. Declare dependencies AFTER what they depend on:

```cpp
// PluginEditor.h
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
private:
    AudioPluginAudioProcessor& processorRef;

    // 1️⃣ RELAYS FIRST (no dependencies)
    juce::WebSliderRelay webGainRelay;
    juce::WebToggleButtonRelay webBypassRelay;
    juce::WebComboBoxRelay webTypeRelay;

    // 2️⃣ WEBVIEW SECOND (depends on relays via withOptionsFrom)
    juce::WebBrowserComponent webView;

    // 3️⃣ ATTACHMENTS LAST (depend on both relays and webView)
    juce::WebSliderParameterAttachment webGainSliderAttachment;
    juce::WebToggleButtonParameterAttachment webBypassToggleAttachment;
    juce::WebComboBoxParameterAttachment webTypeComboBoxAttachment;
};
```

### Why This Matters

**Destruction sequence when plugin reloads:**

✅ **Correct order:**
1. Attachments destroyed first → stop using relay/webView
2. WebView destroyed second → safe (attachments gone)
3. Relays destroyed last → safe (nothing using them)

❌ **Wrong order (attachment before webView):**
1. WebView destroyed first
2. Attachment destroyed second → tries to call `evaluateJavascript()` on destroyed WebView
3. **CRASH** (only in release builds with optimization)

### Multi-Parameter Pattern

```cpp
private:
    AudioPluginAudioProcessor& processorRef;

    // All relays together (independent)
    juce::WebSliderRelay webGainRelay;
    juce::WebSliderRelay webFreqRelay;
    juce::WebSliderRelay webResonanceRelay;
    juce::WebToggleButtonRelay webBypassRelay;

    // WebView (uses all relays)
    juce::WebBrowserComponent webView;

    // All attachments together (depend on relays + webView)
    juce::WebSliderParameterAttachment webGainAttachment;
    juce::WebSliderParameterAttachment webFreqAttachment;
    juce::WebSliderParameterAttachment webResonanceAttachment;
    juce::WebToggleButtonParameterAttachment webBypassAttachment;
};
```

---

## Constructor Initialization Order

### The Rule

Initialize members in **same order** as declaration. C++ initializes in declaration order regardless of initializer list order, but matching prevents confusion.

### Correct Pattern

```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      // 1. Relays
      webGainRelay{id::GAIN.getParamID()},
      webBypassRelay{id::BYPASS.getParamID()},
      // 2. WebView
      webView(juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(
              juce::WebBrowserComponent::Options::WinWebView2{}
                  .withBackgroundColour(juce::Colours::white)
                  .withUserDataFolder(juce::File::getSpecialLocation(
                      juce::File::SpecialLocationType::tempDirectory)))
          .withNativeIntegrationEnabled()
          .withResourceProvider(
              [this](const auto& url) { return getResource(url); },
              juce::URL{LOCAL_DEV_SERVER_ADDRESS}.getOrigin())
          .withOptionsFrom(webGainRelay)  // Register relay with WebView
          .withOptionsFrom(webBypassRelay)),
      // 3. Attachments
      webGainSliderAttachment{
          *processorRef.getState().getParameter(id::GAIN.getParamID()),
          webGainRelay,
          nullptr  // No undo manager
      },
      webBypassToggleAttachment{
          *processorRef.getState().getParameter(id::BYPASS.getParamID()),
          webBypassRelay,
          nullptr
      }
{
    addAndMakeVisible(webView);
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    setSize(400, 300);
}
```

### Key Points

1. **Initialize relays with parameter ID**
   ```cpp
   webGainRelay{id::GAIN.getParamID()}
   ```

2. **Register each relay with WebView**
   ```cpp
   .withOptionsFrom(webGainRelay)
   .withOptionsFrom(webBypassRelay)
   ```

3. **Create attachments with parameter pointer + relay**
   ```cpp
   webGainSliderAttachment{
       *processorRef.getState().getParameter(id::GAIN.getParamID()),
       webGainRelay,
       nullptr
   }
   ```

4. **Navigate to resource provider root**
   ```cpp
   webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
   ```

---

## WebView Options Configuration

### Recommended Options (macOS/cross-platform)

```cpp
webView(juce::WebBrowserComponent::Options{}
    // Backend selection (Windows only - omit on macOS)
    .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)

    // Windows-specific options (omit on macOS)
    .withWinWebView2Options(
        juce::WebBrowserComponent::Options::WinWebView2{}
            .withBackgroundColour(juce::Colours::white)
            // CRITICAL: Prevents permission issues in some DAWs
            .withUserDataFolder(juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::tempDirectory)))

    // CRITICAL: Required for JUCE frontend library
    .withNativeIntegrationEnabled()

    // Resource provider with CORS allowance for dev server
    .withResourceProvider(
        [this](const auto& url) { return getResource(url); },
        juce::URL{LOCAL_DEV_SERVER_ADDRESS}.getOrigin())

    // FL Studio fix: prevent blank screen on focus loss
    .withKeepPageLoadedWhenBrowserIsHidden()

    // Register parameter relays
    .withOptionsFrom(webGainRelay)
    .withOptionsFrom(webBypassRelay))
```

### Platform-Specific Configuration

**macOS-only build:**
```cpp
webView(juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled()
    .withResourceProvider([this](const auto& url) { return getResource(url); })
    .withKeepPageLoadedWhenBrowserIsHidden()  // FL Studio fix
    .withOptionsFrom(webGainRelay))
```

**Windows-specific additions:**
```cpp
#ifdef _WIN32
    .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
    .withWinWebView2Options(
        juce::WebBrowserComponent::Options::WinWebView2{}
            .withBackgroundColour(juce::Colours::white)
            .withUserDataFolder(juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::tempDirectory)))
#endif
```

### When to Use Each Option

| Option | When to Use |
|--------|-------------|
| `.withNativeIntegrationEnabled()` | **Always** (required for parameter binding) |
| `.withResourceProvider(...)` | **Always** (JUCE 8 required pattern) |
| `.withOptionsFrom(relay)` | **Always** (for each parameter relay) |
| `.withKeepPageLoadedWhenBrowserIsHidden()` | FL Studio users, expensive JS initialization |
| `.withBackend(webview2)` | Windows builds only |
| `.withWinWebView2Options(...)` | Windows builds only (prevents DAW permission issues) |
| `.withUserDataFolder(tempDirectory)` | **Critical on Windows** (avoids sandboxing issues) |

---

## Resource Provider Implementation

### Complete Pattern

```cpp
// PluginEditor.h
using Resource = juce::WebBrowserComponent::Resource;
std::optional<Resource> getResource(const juce::String& url) const;

// PluginEditor.cpp
auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    // 1. Parse URL (handle root "/" as index.html)
    const auto resourceToRetrieve =
        url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

    // 2. Debug logging (remove for release)
    // std::cout << "Resource requested: " << resourceToRetrieve << std::endl;

    // 3. Serve embedded files from zip
    const auto resource = getWebViewFileAsBytes(resourceToRetrieve);
    if (!resource.empty()) {
        const auto extension =
            resourceToRetrieve.fromLastOccurrenceOf(".", false, false);
        return Resource{std::move(resource), getMimeForExtension(extension)};
    }

    // 4. Return 404
    return std::nullopt;
}
```

### Helper Functions (Anonymous Namespace)

```cpp
namespace {
    // Convert InputStream to byte vector
    std::vector<std::byte> streamToVector(juce::InputStream& stream) {
        const auto sizeInBytes = static_cast<size_t>(stream.getTotalLength());
        std::vector<std::byte> result(sizeInBytes);
        stream.setPosition(0);
        stream.read(result.data(), result.size());
        return result;
    }

    // Get MIME type for file extension
    const char* getMimeForExtension(const juce::String& extension) {
        static const std::unordered_map<juce::String, const char*> mimeMap = {
            {{"html"}, "text/html"},
            {{"js"},   "text/javascript"},
            {{"css"},  "text/css"},
            {{"json"}, "application/json"},
            {{"png"},  "image/png"},
            {{"jpg"},  "image/jpeg"},
            {{"svg"},  "image/svg+xml"}
        };

        if (const auto it = mimeMap.find(extension.toLowerCase()); it != mimeMap.end())
            return it->second;

        return "application/octet-stream";
    }

    // Read file from embedded zip
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
}
```

### Critical Points

1. **Always handle "/" → "index.html"**
   ```cpp
   const auto resourceToRetrieve =
       url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);
   ```

2. **Return correct MIME types**
   - Wrong MIME = browser won't execute JavaScript
   - Use lookup table, not hardcoded strings

3. **Use `std::move()` for resource bytes**
   ```cpp
   return Resource{std::move(resource), getMimeForExtension(extension)};
   ```

4. **Return `std::nullopt` for 404**
   - Don't crash on missing files
   - Browser will show 404 in DevTools

---

## Development vs Production Builds

### Recommended Pattern

```cpp
// PluginEditor.cpp

namespace {
    // For development: use local server for hot reloading
    constexpr auto LOCAL_DEV_SERVER_ADDRESS = "http://127.0.0.1:8080";
}

// In constructor body:
{
    addAndMakeVisible(webView);

    // PRODUCTION: Serve from embedded files
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    // DEVELOPMENT: Uncomment to use dev server for hot reloading
    // webView.goToURL(LOCAL_DEV_SERVER_ADDRESS);

    setSize(400, 300);
}
```

### Using Preprocessor Flags

**Better approach (automatic switching):**

```cpp
// In constructor body:
{
    addAndMakeVisible(webView);

#if JUCE_DEBUG
    // Development: Load from dev server (hot reload)
    webView.goToURL(LOCAL_DEV_SERVER_ADDRESS);
#else
    // Production: Load from embedded files
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#endif

    setSize(400, 300);
}
```

**Note:** When using dev server, native functions won't work (no `window.__JUCE__`). Need to mock for development.

### Dev Server Setup

```bash
cd ui/public
npx http-server -p 8080

# Hot reload workflow:
# 1. Build plugin once in debug mode
# 2. Edit HTML/JS files
# 3. Hard refresh WebView (Cmd+Shift+R)
# 4. See changes instantly (no C++ recompilation)
```

---

## Error Handling Patterns

### Resource Provider Error Handling

```cpp
auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    try {
        const auto resourceToRetrieve =
            url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

        const auto resource = getWebViewFileAsBytes(resourceToRetrieve);
        if (!resource.empty()) {
            const auto extension =
                resourceToRetrieve.fromLastOccurrenceOf(".", false, false);
            return Resource{std::move(resource), getMimeForExtension(extension)};
        }
    } catch (const std::exception& e) {
        // Log error but don't crash
        std::cerr << "Resource provider error: " << e.what() << std::endl;
    }

    return std::nullopt;
}
```

### JavaScript Error Handling

**In JavaScript:**
```javascript
// Global error handler
window.addEventListener('error', (e) => {
    console.error('JavaScript error:', e.message, e.filename, e.lineno);
});

// Async error handler
window.addEventListener('unhandledrejection', (e) => {
    console.error('Unhandled promise rejection:', e.reason);
});

// Safely call native functions
async function callNativeSafely(funcName, ...args) {
    try {
        const nativeFunc = Juce.getNativeFunction(funcName);
        return await nativeFunc(...args);
    } catch (error) {
        console.error(`Failed to call ${funcName}:`, error);
        return null;
    }
}
```

---

## Testing Checklist

### Build Testing

- [ ] **Debug build compiles** without warnings
- [ ] **Release build compiles** without warnings
- [ ] **Debug build runs** (test member order logic)
- [ ] **Release build runs** (catches member order crash)
- [ ] Plugin loads in standalone
- [ ] Plugin loads in at least one DAW

### UI Testing

- [ ] WebView renders (not blank/encoded/error)
- [ ] All controls visible
- [ ] Right-click → Inspect works
- [ ] Console shows no JavaScript errors
- [ ] `window.__JUCE__` object exists
- [ ] `window.__JUCE__.backend` defined
- [ ] Parameter state objects accessible via `Juce.getSliderState()` etc.

### Parameter Binding Testing

- [ ] Moving UI slider changes audio (verify in DAW)
- [ ] Changing parameter in DAW updates UI
- [ ] Parameter values persist after reload
- [ ] Multiple parameters sync independently
- [ ] Automation works (record parameter changes)

### Reload/Stability Testing

- [ ] Reload plugin window 10 times (no crash)
- [ ] Reset to default state works (standalone)
- [ ] Switch presets (if implemented)
- [ ] Rapid open/close cycles (no crash)
- [ ] Focus loss and restore (controls still work)

### DAW-Specific Testing (macOS)

- [ ] **Logic Pro:** Right-click WebView 10 times (check for crash)
- [ ] **Ableton 12:** Press Escape key (check for crash)
- [ ] **FL Studio:** Press Tab to cycle windows (check for blank screen)
- [ ] **All DAWs:** Verify click-through works (no "dead click" to focus)

### Cross-Platform Testing (if applicable)

- [ ] Windows: WebView2 runtime detected/bundled
- [ ] Windows: Temp directory permissions work
- [ ] Linux: webkit2gtk found
- [ ] All platforms: UI looks consistent

---

## Component Lifecycle Patterns

### Resizing the WebView

**Always make WebView fill the editor bounds:**

```cpp
// PluginEditor.cpp
void AudioPluginAudioProcessorEditor::resized()
{
    webView.setBounds(getBounds());
}
```

**Key points:**
- WebView should resize with editor window
- Use `getBounds()` not `getLocalBounds()` (already relative to parent)
- Don't call `webView.setBounds(getLocalBounds())` - creates offset

### Timer Pattern for Real-Time Updates

**For audio visualization or periodic updates:**

**CRITICAL: Use atomics for shared data between audio and GUI threads:**
```cpp
// PluginProcessor.h
class AudioPluginAudioProcessor : public juce::AudioProcessor
{
public:
    // THREAD-SAFE: Atomic for audio → GUI communication
    std::atomic<float> outputLevelLeft{-100.0f};  // dB
    std::atomic<float> outputLevelRight{-100.0f};

private:
    // ... other members
};

// PluginProcessor.cpp - processBlock() runs on AUDIO THREAD
void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    // Measure audio (envelope follower, RMS, peak, etc.)
    float leftLevel = calculateLevel(buffer, 0);
    float rightLevel = calculateLevel(buffer, 1);

    // SAFE: Atomic store from audio thread
    outputLevelLeft.store(leftLevel);
    outputLevelRight.store(rightLevel);
}
```

**Timer implementation:**

```cpp
// PluginEditor.h
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer  // Inherit Timer
{
public:
    // ... constructor ...

private:
    void timerCallback() override;
    // ... other members
};

// PluginEditor.cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      // ... initialize members ...
{
    // ... setup code ...

    // Start timer: 60ms = ~16 updates/second (good for visualizations)
    startTimer(60);
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    // GUI THREAD: Safe to read atomics
    // Emit event to trigger JavaScript to fetch data
    webView.emitEventIfBrowserIsVisible("audioUpdate", juce::var{});
}

// Resource provider serves the atomic data as JSON
auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    if (url.contains("audioData.json")) {
        juce::DynamicObject::Ptr data{new juce::DynamicObject{}};

        // SAFE: Atomic load on GUI thread
        data->setProperty("leftLevel", processorRef.outputLevelLeft.load());
        data->setProperty("rightLevel", processorRef.outputLevelRight.load());

        const auto jsonString = juce::JSON::toString(data.get());
        juce::MemoryInputStream stream{jsonString.getCharPointer(),
                                       jsonString.getNumBytesAsUTF8(), false};
        return Resource{streamToVector(stream), "application/json"};
    }
    // ... handle other resources
}
```

**Timer intervals:**
- **30ms** (33 fps) - Smooth animations
- **60ms** (16 fps) - Audio level meters
- **100ms** (10 fps) - Less critical updates
- **1000ms** (1 fps) - Slow polling

**Don't overdo it:**
- WebView rendering is expensive
- Use `emitEventIfBrowserIsVisible()` (skips if hidden)
- JavaScript should throttle/debounce updates if needed

---

## JavaScript Structure Patterns

### Module Import Pattern

**index.html:**
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <script type="module" src="js/index.js"></script>
    <title>My Plugin</title>
</head>
<body>
    <main id="app">
        <!-- UI elements -->
    </main>
</body>
</html>
```

**js/index.js:**
```javascript
// Import JUCE frontend library
import * as Juce from "./juce/index.js";

// Verify connection
console.log("JUCE backend connected:", window.__JUCE__.backend);

// Get parameter states
const gainState = Juce.getSliderState("GAIN");
const bypassState = Juce.getToggleState("BYPASS");

// Setup UI
setupControls();
setupEventListeners();

function setupControls() {
    // Initialize sliders, buttons, etc.
}

function setupEventListeners() {
    // Listen for C++ events
    window.__JUCE__.backend.addEventListener("audioUpdate", handleAudioUpdate);
}

async function handleAudioUpdate() {
    // Fetch real-time data
    const response = await fetch(Juce.getBackendResourceAddress("audioData.json"));
    const data = await response.json();
    updateVisualization(data);
}
```

**Key points:**
- Use ES6 modules (`type="module"`)
- Import JUCE library first
- Verify `window.__JUCE__` exists
- Separate setup, event handling, visualization

### Error Handling in JavaScript

**Always wrap JUCE calls:**

```javascript
// Check if JUCE is available
if (!window.__JUCE__) {
    console.error("JUCE backend not available");
    document.body.innerHTML = "<h1>Error: JUCE backend not connected</h1>";
}

// Wrap native function calls
async function callNativeSafely(funcName, ...args) {
    try {
        const nativeFunc = Juce.getNativeFunction(funcName);
        return await nativeFunc(...args);
    } catch (error) {
        console.error(`Native call failed: ${funcName}`, error);
        return null;
    }
}

// Global error handlers
window.addEventListener('error', (e) => {
    console.error('JS Error:', e.message, e.filename, e.lineno);
});

window.addEventListener('unhandledrejection', (e) => {
    console.error('Unhandled Promise:', e.reason);
});
```

---

## CMake Configuration Essentials

### Required Compile Definitions

```cmake
target_compile_definitions(YourPlugin PRIVATE
    # Enable WebView
    JUCE_WEB_BROWSER=1

    # macOS: Bundle identifier (required for AU)
    JUCE_BUNDLE_ID="com.yourcompany.yourplugin"

    # Embedded files zip prefix
    ZIPPED_FILES_PREFIX="public/"

    # Plugin info (optional, useful for JS)
    JUCE_COMPANY_NAME="${COMPANY_NAME}"
    JUCE_PRODUCT_NAME="${PRODUCT_NAME}"
    JUCE_PRODUCT_VERSION="${PROJECT_VERSION}"
)
```

### Windows-Specific (omit on macOS)

```cmake
if (MSVC)
    target_compile_definitions(YourPlugin PRIVATE
        # Use WebView2 with static linking
        JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
    )
endif()
```

### Required Modules

```cmake
target_link_libraries(YourPlugin PRIVATE
    juce::juce_gui_extra      # WebBrowserComponent
    juce::juce_audio_utils    # Plugin interfaces
    juce::juce_dsp            # Audio processing (optional)
    WebViewFiles              # Embedded web files
)
```

---

## File Structure Organization

### Recommended Project Layout

```
plugin/
├── CMakeLists.txt
├── include/
│   ├── ParameterIDs.h          # Parameter ID constants
│   ├── PluginEditor.h          # Editor (WebView)
│   └── PluginProcessor.h       # Audio processor
├── source/
│   ├── PluginEditor.cpp
│   └── PluginProcessor.cpp
└── ui/
    └── public/                  # Web UI files
        ├── index.html
        ├── css/
        │   └── styles.css
        └── js/
            ├── index.js         # Main entry point
            └── juce/            # JUCE frontend library (copied from JUCE)
                └── index.js
```

### ParameterIDs.h Pattern

```cpp
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace id {
    inline constexpr auto GAIN = juce::ParameterID{"gain", 1};
    inline constexpr auto FREQUENCY = juce::ParameterID{"frequency", 1};
    inline constexpr auto BYPASS = juce::ParameterID{"bypass", 1};
    inline constexpr auto TYPE = juce::ParameterID{"type", 1};
}
```

**Why centralize IDs:**
- Single source of truth
- Used in Processor (create params) and Editor (bind to UI)
- Easy to refactor/rename
- Compile-time constants

---

## Anti-Patterns to Avoid

### ❌ Wrong Member Order
```cpp
// WRONG - will crash in release build
juce::WebSliderParameterAttachment webGainAttachment;  // First
juce::WebBrowserComponent webView;  // Second
juce::WebSliderRelay webGainRelay;  // Third
```

### ❌ Hardcoded MIME Types
```cpp
// WRONG - doesn't scale
if (resourceToRetrieve.endsWith(".html"))
    return Resource{resource, "text/html"};
if (resourceToRetrieve.endsWith(".js"))
    return Resource{resource, "text/javascript"};
// ... 50 more if statements
```

### ❌ Missing Native Integration
```cpp
// WRONG - JUCE library won't work
webView(juce::WebBrowserComponent::Options{}
    .withResourceProvider([this](const auto& url) { return getResource(url); }))
    // Missing: .withNativeIntegrationEnabled()
```

### ❌ Forgetting to Register Relays
```cpp
// WRONG - parameter won't sync
webGainRelay{id::GAIN.getParamID()},
webView(juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled()
    .withResourceProvider([this](const auto& url) { return getResource(url); }))
    // Missing: .withOptionsFrom(webGainRelay)
```

### ❌ Not Handling "/" Root
```cpp
// WRONG - index.html won't load
const auto resourceToRetrieve = url.fromFirstOccurrenceOf("/", false, false);
// Doesn't handle url == "/"
```

---

## Summary: Golden Rules

1. **Member order:** Relay → WebView → Attachment (prevents 99% of crashes)
2. **Always enable native integration:** `.withNativeIntegrationEnabled()`
3. **Always register relays:** `.withOptionsFrom(relay)` for each parameter
4. **Always handle "/":** Convert to "index.html" in resource provider
5. **Always return correct MIME types:** Use lookup table, not hardcoded
6. **Always use atomics:** `std::atomic<T>` for audio → GUI data
7. **Always test release builds:** Member order crashes only show there
8. **Always test reload:** Close/reopen window 10+ times
9. **Always check DevTools console:** No JavaScript errors before marking complete
10. **Never call WebView APIs from audio thread:** Only GUI thread

**Thread Safety Rule:**
- Audio thread writes to `std::atomic<T>`
- GUI thread reads from `std::atomic<T>`
- Resource provider serves atomic data as JSON
- JavaScript fetches JSON on timer events

Follow these patterns and your WebView plugin will work reliably across all platforms and DAWs.
