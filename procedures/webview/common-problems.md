# Common Problems and Solutions

Comprehensive troubleshooting guide for JUCE 8 WebView plugins. Problems organized by symptom.

---

## Table of Contents

- [WebView Shows Encoded HTML Text](#webview-shows-encoded-html-text)
- ["The URL can't be shown" Error](#the-url-cant-be-shown-error)
- [Blank White WebView](#blank-white-webview)
- [JUCE Object is Undefined](#juce-object-is-undefined)
- [Parameters Not Syncing](#parameters-not-syncing)
- [CORS Errors from Dev Server](#cors-errors-from-dev-server)
- [WebView2 Not Found (Windows)](#webview2-not-found-windows)
- [Version-Specific API Changes](#version-specific-api-changes)
- [Validation Gaps](#validation-gaps)
- [WebSliderRelay Crash in Release Builds](#websliderrelay-crash-in-release-builds)
- [FL Studio Blank Screen on Focus Loss](#fl-studio-blank-screen-on-focus-loss)
- [Must Click Before Interaction (macOS)](#must-click-before-interaction-macos)
- [JavaScript Debugging Challenges](#javascript-debugging-challenges)
- [Right-Click Crash in Logic Pro](#right-click-crash-in-logic-pro)
- [Escape Key Crash in Ableton](#escape-key-crash-in-ableton)

---

## WebView Shows Encoded HTML Text

### Symptom
UI displays percent-encoded text instead of rendering HTML:
```
%3C!DOCTYPE%20html%3E%0A%3Chtml%3E%0A%3Chead%3E...
```

### Root Cause
Data URL with percent-encoding causes WebView to display HTML as plaintext instead of parsing it.

### Bad Code (JUCE 7 pattern)
```cpp
// DON'T DO THIS - Displays encoded text
webView.goToURL("data:text/html;charset=utf-8," +
                juce::URL::addEscapeChars(htmlContent, false));
```

**Why this fails:**
1. `URL::addEscapeChars()` converts special characters to `%XX` format
2. WebView receives a data URL containing encoded text
3. WebView displays the text literally instead of parsing as HTML

### Solution: Use JUCE 8 Resource Provider Pattern

**PluginEditor.h:**
```cpp
juce::WebBrowserComponent webView{
    juce::WebBrowserComponent::Options{}
        .withNativeIntegrationEnabled()
        .withResourceProvider([this](const juce::String& path) {
            return getResource(path);
        })
};

std::optional<juce::WebBrowserComponent::Resource>
getResource(const juce::String& path) const;
```

**PluginEditor.cpp:**
```cpp
// Constructor
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView(/* options above */)
{
    addAndMakeVisible(webView);
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    setSize(800, 600);
}

// Resource provider implementation
std::optional<juce::WebBrowserComponent::Resource>
AudioPluginAudioProcessorEditor::getResource(const juce::String& path) const
{
    const auto resourceName =
        path == "/" ? "index.html" : path.fromFirstOccurrenceOf("/", false, false);

    if (resourceName == "index.html") {
        juce::MemoryInputStream stream(BinaryData::index_html,
                                       BinaryData::index_htmlSize,
                                       false);
        return juce::WebBrowserComponent::Resource{
            streamToVector(stream),
            "text/html"
        };
    }

    return std::nullopt;
}

// Helper function
static std::vector<std::byte> streamToVector(juce::InputStream& stream)
{
    std::vector<std::byte> result(static_cast<size_t>(stream.getTotalLength()));
    stream.setPosition(0);
    [[maybe_unused]] const auto bytesRead =
        stream.read(result.data(), result.size());
    jassert(bytesRead == static_cast<ssize_t>(result.size()));
    return result;
}
```

### Alternative (if you must use data URLs)

**Option A: Raw HTML without encoding**
```cpp
// Works but not recommended for large HTML
webView.goToURL("data:text/html;charset=utf-8," + htmlContent);
```

**Option B: Base64 encoding**
```cpp
auto base64 = juce::Base64::toBase64(htmlContent);
webView.goToURL("data:text/html;charset=utf-8;base64," + base64);
```

**Why resource provider is better:**
- Works consistently across all platforms
- Supports multiple files (CSS, JS, images)
- Easier to serve dynamic data
- Recommended JUCE 8 pattern

### Related Docs
- **02-project-setup.md** - WebView Options
- **03-communication-patterns.md** - Resource Provider Pattern
- **07-distribution.md** - Embedding Files in Binary

---

## "The URL can't be shown" Error

### Symptom
WebView displays system error message or blank screen with console error.

### Root Cause
JUCE 8 uses WKWebView on macOS which blocks `file://` URLs for security reasons. WKWebView runs in a sandboxed process and prevents filesystem access.

### Bad Code (JUCE 7 pattern)
```cpp
// DON'T DO THIS - Blocked by WKWebView security
auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
auto htmlFile = tempDir.getChildFile("TapeDelayUI.html");
htmlFile.replaceWithText(htmlContent);
webView.goToURL(htmlFile.getFullPathName());  // file:///path/to/file.html
```

**Why this fails:**
1. JUCE 8's WebBrowserComponent uses WKWebView on macOS
2. WKWebView has strict security policies blocking `file://` URLs by default
3. Native API `loadFileURL:allowingReadAccessTo:` exists but JUCE 8 doesn't expose it
4. Result: "The URL can't be shown" error

### Security Rationale
- WKWebView runs in separate process with sandboxing
- `file://` access could allow reading arbitrary files from disk
- Apple's security model requires all web content go through network protocols

### Solution: Use Resource Provider Pattern

Same solution as "Encoded HTML Text" above. Resource provider:
- Uses custom `juce://` scheme (macOS) instead of `file://`
- Serves content from memory via callback (no filesystem access)
- Satisfies WKWebView's security requirements
- Official JUCE 8 pattern

**Technical flow:**
1. `getResourceProviderRoot()` returns `juce://`
2. WebView navigates to `juce://`
3. WKWebView requests `"/"` path
4. `getResource("/")` callback fires
5. Callback returns HTML as `std::vector<std::byte>` with MIME type
6. WKWebView parses and renders HTML

### Related Docs
- **02-project-setup.md** - Project Setup
- **03-communication-patterns.md** - Resource Provider Pattern
- **07-distribution.md** - Serving Embedded Files

---

## Blank White WebView

### Symptom
WebView component renders but shows nothing (blank white or transparent).

### Debugging Checklist

**1. Open Browser DevTools**
- Right-click WebView → Inspect (Windows/Linux)
- Right-click WebView → Inspect Element (macOS)
- Check console for JavaScript errors

**2. Verify Native Integration Enabled**
```cpp
// REQUIRED for JUCE frontend library
webView(juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled()  // ← Must be present
    .withResourceProvider([this](const auto& url) { return getResource(url); }))
```

**3. Add Debug Logging to Resource Provider**
```cpp
std::optional<juce::WebBrowserComponent::Resource>
AudioPluginAudioProcessorEditor::getResource(const juce::String& path) const
{
    std::cout << "Resource requested: [" << path << "]" << std::endl;  // DEBUG

    const auto resourceName =
        path == "/" ? "index.html" : path.fromFirstOccurrenceOf("/", false, false);

    std::cout << "  → Resolved to: [" << resourceName << "]" << std::endl;  // DEBUG

    // ... rest of implementation
}
```

**Expected output:**
```
Resource requested: [/]
  → Resolved to: [index.html]
Resource requested: [js/index.js]
  → Resolved to: [js/index.js]
```

**4. Verify goToURL Called**
```cpp
// After webView initialization
webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
```

**5. Check JUCE Object in Browser Console**
```javascript
console.log(window.__JUCE__);  // Should exist if native integration enabled
```

### Common Causes

| Issue | Solution |
|-------|----------|
| Resource provider returns `std::nullopt` for index.html | Implement handler for "/" → "index.html" |
| Missing `withNativeIntegrationEnabled()` | Add to Options builder |
| Wrong URL passed to `goToURL()` | Use `getResourceProviderRoot()` |
| BinaryData files not embedded | Check CMakeLists.txt zip + binary data setup |
| MIME type incorrect | Return "text/html" for HTML, "application/javascript" for JS |

### Testing Resource Provider in Isolation

**Create test endpoint:**
```cpp
std::optional<juce::WebBrowserComponent::Resource>
AudioPluginAudioProcessorEditor::getResource(const juce::String& path) const
{
    // Test with hardcoded HTML
    if (path == "/") {
        const juce::String testHtml = R"(
            <!DOCTYPE html>
            <html>
            <head><title>Test</title></head>
            <body>
                <h1>Resource Provider Works!</h1>
            </body>
            </html>
        )";

        juce::MemoryInputStream stream(testHtml.toRawUTF8(),
                                       testHtml.getNumBytesAsUTF8(),
                                       false);
        return juce::WebBrowserComponent::Resource{
            streamToVector(stream),
            "text/html"
        };
    }

    return std::nullopt;
}
```

If this works, problem is in your file loading logic (BinaryData, file paths, etc).

### Related Docs
- **06-development-workflow.md** - Debugging Techniques
- **03-communication-patterns.md** - Resource Provider Pattern

---

## JUCE Object is Undefined

### Symptom
JavaScript error: `Uncaught ReferenceError: window.__JUCE__ is undefined`

### Root Cause
Native integration not enabled in WebBrowserComponent options.

### Solution
```cpp
// In WebBrowserComponent::Options
.withNativeIntegrationEnabled()  // ← CRITICAL
```

**Complete example:**
```cpp
webView(juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled()  // Enables window.__JUCE__
    .withResourceProvider([this](const auto& url) { return getResource(url); }))
```

### Verification
In browser console:
```javascript
console.log(window.__JUCE__);
// Should output: { backend: {...}, initialisationData: {...} }
```

### Related Docs
- **02-project-setup.md** - WebView Options
- **03-communication-patterns.md** - Native Integration

---

## Parameters Not Syncing

### Symptom
- Moving UI slider doesn't change audio parameter
- Changing parameter in DAW doesn't update UI

### Root Cause 1: Wrong Initialization Order

**Relay must initialize BEFORE webView:**

**Bad (relay after webView):**
```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView(Options{}...),  // ← webView first (WRONG)
      webGainRelay{id::GAIN.getParamID()}  // ← relay second (WRONG)
```

**Good (relay before webView):**
```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webGainRelay{id::GAIN.getParamID()},  // ← relay first (CORRECT)
      webView(Options{}
          .withNativeIntegrationEnabled()
          .withOptionsFrom(webGainRelay)  // ← Uses relay options
          .withResourceProvider([this](const auto& url) { return getResource(url); }))
```

**Why this matters:**
- Relay registers JavaScript state objects during construction
- `withOptionsFrom()` copies relay's options into webView
- If relay doesn't exist yet, options aren't registered

### Root Cause 2: Parameter ID Mismatch

**C++ parameter ID must match JavaScript:**

**ParameterIDs.h:**
```cpp
namespace id {
    inline constexpr auto GAIN = juce::ParameterID{"gain", 1};
}
```

**JavaScript:**
```javascript
// Must match exactly (case-sensitive)
const sliderState = Juce.getSliderState("gain");  // ← Must be "gain"
```

### Root Cause 3: Missing Parameter Attachment

**Header:**
```cpp
// In PluginEditor.h
juce::WebSliderParameterAttachment webGainAttachment;
```

**Constructor:**
```cpp
// In constructor initializer list
webGainAttachment{*processorRef.apvts.getParameter(id::GAIN.getParamID()),
                  webGainRelay}
```

**Note:** Attachment must be initialized AFTER both relay and webView.

### Debugging Parameter Binding

**1. Check JavaScript state object exists:**
```javascript
const sliderState = Juce.getSliderState("gain");
console.log(sliderState);  // Should output object with value, min, max, etc.
```

**2. Check parameter value updates:**
```javascript
sliderState.valueChanged = (newValue) => {
    console.log("Value changed:", newValue);  // Should fire when DAW changes param
};
```

**3. Check slider input updates parameter:**
```javascript
slider.addEventListener("input", (e) => {
    const value = parseFloat(e.target.value);
    console.log("Setting value:", value);
    sliderState.setValue(value);  // Should update audio parameter
});
```

### Related Docs
- **04-parameter-binding.md** - Complete Parameter Binding Guide
- **04-parameter-binding.md (Initialization Order)** - Critical section

---

## CORS Errors from Dev Server

### Symptom
Browser console shows CORS errors when loading from local dev server (`http://localhost:8080`).

### Root Cause
Resource provider needs to allow cross-origin requests from dev server.

### Solution
Add second parameter to `withResourceProvider()`:

```cpp
.withResourceProvider(
    [this](const auto& url) { return getResource(url); },
    "http://localhost:8080")  // ← Allow this origin
```

**Complete example:**
```cpp
webView(juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled()
    .withResourceProvider(
        [this](const auto& url) { return getResource(url); },
        "http://localhost:8080"))  // Allow dev server

// Then load from dev server
webView.goToURL("http://localhost:8080");
```

### Hot Reloading Workflow

**1. Start dev server:**
```bash
cd ui/public
npx http-server
```

**2. Change code to load from dev server:**
```cpp
// Development (comment out for production)
webView.goToURL("http://localhost:8080");

// Production (comment out for development)
// webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
```

**3. Rebuild once, then:**
- Edit HTML/JS files
- Hard refresh WebView (Cmd+Shift+R / Ctrl+Shift+R)
- See changes instantly (no C++ recompilation)

### Related Docs
- **06-development-workflow.md** - Hot Reloading Setup
- **06-development-workflow.md (Local Dev Server)** - Complete guide

---

## WebView2 Not Found (Windows)

### Symptom
Windows build error: `WebView2 SDK not found` or runtime error about missing WebView2.

### Root Cause
WebView2 requires either:
1. WebView2 Runtime installed on system, OR
2. WebView2 SDK NuGet package added to project

### Solution 1: Install WebView2 Runtime (End Users)
Download from: https://developer.microsoft.com/microsoft-edge/webview2/

### Solution 2: Add NuGet Package (Development)

**CMakeLists.txt:**
```cmake
# Windows WebView2 setup
if (MSVC)
    # NuGet package manager
    find_program(NUGET_EXE NAMES nuget)
    if (NOT NUGET_EXE)
        message(WARNING "NuGet not found. Install from https://www.nuget.org/downloads")
    endif()

    # Install WebView2 package
    execute_process(
        COMMAND ${NUGET_EXE} install Microsoft.Web.WebView2
                -Version 1.0.2210.55
                -OutputDirectory ${CMAKE_BINARY_DIR}/packages
    )

    # Link WebView2
    target_include_directories(YourPlugin PRIVATE
        ${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2.1.0.2210.55/build/native/include)
    target_link_libraries(YourPlugin PRIVATE
        ${CMAKE_BINARY_DIR}/packages/Microsoft.Web.WebView2.1.0.2210.55/build/native/${CMAKE_VS_PLATFORM_NAME}/WebView2LoaderStatic.lib)
endif()
```

**Manual NuGet commands:**
```bash
# From project root
nuget install Microsoft.Web.WebView2 -Version 1.0.2210.55 -OutputDirectory build/packages
```

### Verification
Build should complete without errors. When opening plugin in DAW, WebView should render (if Runtime installed).

### Distribution
Include WebView2 Runtime installer with your plugin installer, or require users to install separately.

### Related Docs
- **02-project-setup.md (Windows Setup)** - Complete Windows configuration
- **07-distribution.md** - Packaging for end users

---

## Version-Specific API Changes

### JUCE 7 vs JUCE 8 Breaking Changes

JUCE 8 introduced **mandatory resource provider pattern** to replace less secure approaches.

| What | JUCE 7 | JUCE 8 | Why Changed |
|------|--------|--------|-------------|
| **Data URLs** | ✅ `data:text/html,...` | ❌ Use resource provider | Security & cross-platform consistency |
| **File URLs** | ✅ `file:///path/to/file.html` | ❌ Use resource provider | WKWebView sandboxing blocks `file://` |
| **Direct HTML** | ✅ `setHTML()` method | ❌ Use resource provider | Unified API across platforms |
| **Loading pattern** | Multiple methods | Single pattern (resource provider) | Consistency |

### Check Your JUCE Version

**CMakeLists.txt:**
```cmake
find_package(JUCE CONFIG REQUIRED)
message(STATUS "JUCE Version: ${JUCE_VERSION}")
```

**Build output:**
```
-- JUCE Version: 8.0.6
```

### Migration from JUCE 7 to JUCE 8

**Before (JUCE 7):**
```cpp
webView.goToURL("data:text/html," + htmlContent);
// or
webView.goToURL("file://" + htmlFile.getFullPathName());
```

**After (JUCE 8):**
```cpp
// 1. Add resource provider to options
webView(juce::WebBrowserComponent::Options{}
    .withResourceProvider([this](const auto& url) { return getResource(url); }))

// 2. Implement getResource()
std::optional<juce::WebBrowserComponent::Resource>
getResource(const juce::String& path) const
{
    // Serve HTML, CSS, JS from memory or embedded binary data
}

// 3. Navigate to resource provider root
webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
```

### Why Resource Provider is Better

1. **Security**: No filesystem access, memory-only serving
2. **Cross-platform**: Works identically on Windows/macOS/Linux
3. **Dynamic data**: Can serve real-time audio data as JSON
4. **Single pattern**: One way to do things (no confusion)
5. **Future-proof**: Official JUCE 8+ pattern

### Related Docs
- **02-project-setup.md** - Complete JUCE 8 setup
- **03-communication-patterns.md** - Resource Provider Pattern

---

## Validation Gaps

### The "It Compiles" Fallacy

**Problem:** Plugin passes all build checks but UI is completely broken.

**Scenario:**
1. Implement WebView integration
2. Build succeeds ✓
3. pluginval passes ✓
4. Mark as "Working" ✓
5. User opens plugin → **UI doesn't render**

### What Stage 6 Validation Currently Checks

- ✅ pluginval tests (DSP, automation, thread safety)
- ✅ Build artifacts exist (VST3, AU binaries)
- ✅ Presets created (JSON files)
- ✅ CHANGELOG exists

### What Stage 6 Doesn't Check

- ❌ UI rendering (no screenshot verification)
- ❌ WebView content loading (no DOM inspection)
- ❌ Visual correctness (no comparison to mockup)
- ❌ Parameter binding (no UI ↔ DSP verification)

### Recommended Validation Enhancements

**1. Manual UI Test (Immediate)**

Before marking plugin complete:
- Build and install plugin
- Open in DAW
- Verify UI renders (not blank/error)
- Move a slider, verify audio changes
- Take screenshot for documentation

**2. Screenshot Validation (Automated)**

**Add to build script:**
```bash
# Take screenshot of plugin UI
./scripts/screenshot-plugin.sh YourPlugin.vst3

# Compare to mockup (if available)
./scripts/compare-screenshots.sh screenshots/YourPlugin.png mockups/YourPlugin.png
```

**3. Parameter Binding Test (Semi-Automated)**

```cpp
// In unit test or standalone runner
auto plugin = createPluginInstance();
auto editor = plugin->createEditor();

// Programmatically set parameter
plugin->setParameter(0, 0.75f);

// Verify UI updates (requires WebView automation)
// This is complex but possible with platform-specific APIs
```

**4. Validation Checklist**

**Add to Stage 6:**
- [ ] Plugin opens in DAW without crash
- [ ] UI renders (not blank/error/encoded text)
- [ ] All parameter controls visible
- [ ] Moving slider changes audio (verify in DAW)
- [ ] Changing parameter in DAW updates UI
- [ ] Screenshot matches mockup (visual diff < 5%)

### Runtime Failures Not Caught at Compile Time

| Issue | Compile | Runtime |
|-------|---------|---------|
| Wrong JUCE API pattern (JUCE 7 vs 8) | ✅ Passes | ❌ UI broken |
| Resource provider returns nullopt | ✅ Passes | ❌ Blank screen |
| Parameter ID mismatch (C++ vs JS) | ✅ Passes | ❌ No sync |
| Missing `withNativeIntegrationEnabled()` | ✅ Passes | ❌ JS error |
| WKWebView blocks file:// URLs | ✅ Passes | ❌ "URL can't be shown" |

**Key insight:** UI rendering failures don't show up at compile time. Manual verification required.

### Related Docs
- **06-development-workflow.md** - Testing in DAWs
- **02-project-setup.md** - Complete setup to avoid issues

---

## WebSliderRelay Crash in Release Builds

### Symptom
Plugin crashes when:
- Clicking "Reset to default state" in standalone
- Reloading plugin window in DAW
- Any WebView reload event

**Critical:** Only occurs in **release builds** with optimization enabled. Works fine in debug builds.

### Root Cause
Member initialization order violation. `WebSliderParameterAttachment` gets initialized before `WebBrowserComponent`, causing the attachment to call `evaluateJavascript()` on a destroyed WebView during reload.

**From JUCE team:**
> "WebSliderParameterAttachment is initialised before the WebBrowserComponent. During WebView destruction and reconstruction, the attachment tries to evaluate JavaScript on a non-existent component."

### The Problem Code

**Bad (causes crash):**
```cpp
// PluginEditor.h
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    // ... constructor ...

private:
    AudioPluginAudioProcessor& processorRef;

    // ❌ WRONG ORDER - attachment before webView
    juce::WebSliderParameterAttachment webGainAttachment;
    juce::WebBrowserComponent webView;
    juce::WebSliderRelay webGainRelay;
};
```

### Solution: Fix Member Declaration Order

**Correct order:**
```cpp
// PluginEditor.h
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    // ... constructor ...

private:
    AudioPluginAudioProcessor& processorRef;

    // ✅ CORRECT ORDER
    // 1. Relay first (no dependencies)
    juce::WebSliderRelay webGainRelay;

    // 2. WebView second (depends on relay options)
    juce::WebBrowserComponent webView;

    // 3. Attachment last (depends on both relay and webView)
    juce::WebSliderParameterAttachment webGainAttachment;
};
```

### Why This Matters

**C++ destruction order:**
- Members destroyed in **reverse order** of declaration
- If attachment declared first, it's destroyed last
- During reload, WebView destroyed → attachment tries to use it → crash

**Correct destruction sequence:**
1. `webGainAttachment` destroyed first (stops using relay/webView)
2. `webView` destroyed second (safe - attachment no longer using it)
3. `webGainRelay` destroyed last (safe - nothing using it)

### Constructor Initialization Order

**Also ensure correct order in initializer list:**
```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webGainRelay{id::GAIN.getParamID()},  // 1. Relay first
      webView(juce::WebBrowserComponent::Options{}
          .withNativeIntegrationEnabled()
          .withOptionsFrom(webGainRelay)  // 2. WebView uses relay
          .withResourceProvider([this](const auto& url) { return getResource(url); })),
      webGainAttachment{*processorRef.apvts.getParameter(id::GAIN.getParamID()),
                        webGainRelay}  // 3. Attachment last
{
    addAndMakeVisible(webView);
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    setSize(800, 600);
}
```

### How to Verify

**1. Build in release mode:**
```bash
cmake --preset release
cmake --build --preset release
```

**2. Test reload scenarios:**
- Open plugin in DAW
- Close and reopen plugin window multiple times
- If using standalone, click "Reset to default state"

**3. Should not crash** if order is correct.

### Multiple Parameters

**For multiple relays and attachments:**
```cpp
private:
    AudioPluginAudioProcessor& processorRef;

    // All relays first
    juce::WebSliderRelay webGainRelay;
    juce::WebSliderRelay webFreqRelay;
    juce::WebToggleButtonRelay webBypassRelay;

    // WebView second
    juce::WebBrowserComponent webView;

    // All attachments last
    juce::WebSliderParameterAttachment webGainAttachment;
    juce::WebSliderParameterAttachment webFreqAttachment;
    juce::WebToggleButtonParameterAttachment webBypassAttachment;
};
```

### Related Issues
- **JUCE Version:** 8.0.1+
- **GitHub Issue:** #1415
- **Status:** Documented by JUCE team, tutorial examples being updated

### Prevention
Always declare members in dependency order:
1. Independent components (relays)
2. Dependent components (webView uses relays)
3. Most dependent components (attachments use both)

---

## FL Studio Blank Screen on Focus Loss

### Symptom
In FL Studio 25 on macOS:
- Pressing **Tab** to cycle windows causes WebView to go blank/white
- WebView loads "about:blank" instead of your UI
- Happens every time plugin loses focus

### Root Cause
WKWebView gets destroyed and recreated when plugin window loses focus. FL Studio's window focus management triggers this behavior.

**From JUCE forums:**
> "The native web view instance is being destroyed and re-created when the plugin window loses and regains focus."

### Solution: Keep Page Loaded

**Add option to WebBrowserComponent:**
```cpp
webView(juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled()
    .withKeepPageLoadedWhenBrowserIsHidden()  // ← Add this
    .withResourceProvider([this](const auto& url) { return getResource(url); }))
```

**What this does:**
- Prevents page unloading when WebView hidden
- Keeps WKWebView instance alive during focus changes
- Maintains JavaScript state across window switches

### FL Studio-Specific Workaround

**Alternative:** Use FL Studio's "detached" window mode
- Right-click plugin in FL Studio
- Select "Detached" window mode
- Eliminates both blank screen and pointer event issues

### Trade-offs

**Pros:**
- Fixes blank screen issue completely
- Preserves JavaScript state (no re-initialization)
- No performance impact

**Cons:**
- Uses slightly more memory (page always loaded)
- May keep resources allocated when plugin hidden
- Could affect battery life on laptops

### When to Use

**Use `.withKeepPageLoadedWhenBrowserIsHidden()` if:**
- Targeting FL Studio users
- JavaScript initialization is expensive
- UI maintains complex state (animations, WebGL, etc.)

**Don't use if:**
- Memory usage is critical
- Page loads quickly
- No expensive JavaScript state

### Other DAWs

This option is safe to use in other DAWs and won't cause issues. It's a defensive option that prevents page unloading behavior.

### Related Issues
- **Affects:** FL Studio 25, macOS
- **Forum Thread:** "FL Studio 25 broken behavior with a WebBrowserComponent on macOS"
- **Status:** Official workaround available

---

## Must Click Before Interaction (macOS)

### Symptom
On macOS, after plugin loses focus:
- User must click plugin window once before any controls work
- Sliders, buttons don't respond to first click
- Second click (after focus restored) works normally

### Root Cause
macOS `acceptsFirstMouse` behavior not implemented for WKWebView. System requires explicit focus restoration before WebView accepts input events.

### Solution: Enable Click-Through

**JUCE 8 (June 2024+) enables this by default.**

If using older JUCE or need to disable:
```cpp
// Option added in JUCE 8 juce8 branch (June 10, 2024)
webView(juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled()
    // Click-through enabled by default
    // Can disable with: .withClickThroughDisabled()  (if API available)
    .withResourceProvider([this](const auto& url) { return getResource(url); }))
```

### JUCE Version Check

**To verify you have the fix:**
```bash
# Check JUCE version
git log --oneline --all --grep="click-through"

# Look for commit around June 10, 2024:
# "WebBrowserComponent: Enable click-through behaviour for WkWebView"
```

### If Using Older JUCE

**Workaround 1: Update JUCE**
```bash
cd path/to/JUCE
git pull origin juce8
# Or update to JUCE 8.0.6+
```

**Workaround 2: Manual Focus Management**
```cpp
// In PluginEditor.cpp
void AudioPluginAudioProcessorEditor::parentHierarchyChanged()
{
    // Force focus when plugin becomes visible
    if (isShowing())
        grabKeyboardFocus();
}
```

### Expected Behavior After Fix

- First click on plugin activates controls immediately
- No "dead click" to restore focus
- Matches behavior of native JUCE components

### Related Issues
- **JUCE Version:** Fixed in JUCE 8 (June 2024)
- **Forum Thread:** "WebViewPluginDemo JUCE8 focus issue"
- **Based on:** choc library's click-through implementation

---

## JavaScript Debugging Challenges

### Problem
Can't use traditional breakpoint debugging when JavaScript calls native C++ functions:
- Right-click → Inspect works, but only shows console
- No step-through debugging like VS Code
- Source maps don't load when serving via ResourceProvider

### Root Cause
**Windows:** WebView2 doesn't load external source maps referenced by files served through ResourceProvider API.

**macOS:** Limited debugging tools for WKWebView in plugin context.

### Solution 1: Development Server (Recommended)

**Load from localhost during development:**

```cpp
// PluginEditor.cpp constructor

#if JUCE_DEBUG
    // Development: Load from local server
    webView.goToURL("http://localhost:8080");
#else
    // Production: Load from ResourceProvider
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
#endif
```

**Don't forget CORS allowance:**
```cpp
webView(juce::WebBrowserComponent::Options{}
    .withNativeIntegrationEnabled()
    .withResourceProvider(
        [this](const auto& url) { return getResource(url); },
        "http://localhost:8080")  // ← Allow dev server origin
```

**Start dev server:**
```bash
cd ui/public
npx http-server -p 8080

# Hot reload workflow:
# 1. Build plugin once in debug mode
# 2. Edit HTML/JS files
# 3. Hard refresh WebView (Cmd+Shift+R)
# 4. See changes instantly
```

**Benefits:**
- Full VS Code debugging with breakpoints
- Source maps load correctly
- Hot reload (no C++ recompilation)
- All browser DevTools features

**Trade-offs:**
- Native functions won't work (window.__JUCE__ undefined when loading externally)
- Must mock native functions for dev
- Need to remember to switch back for production builds

### Solution 2: Inline Source Maps

**TypeScript configuration:**
```json
// tsconfig.json
{
  "compilerOptions": {
    "inlineSourceMap": true,  // ← Embed source maps in JS files
    "inlineSources": true,
    "sourceMap": false  // Don't generate separate .map files
  }
}
```

**What this does:**
- Embeds source map data directly in compiled JavaScript
- ResourceProvider serves everything in one file
- Debugging works in browser DevTools

**Trade-offs:**
- Increases JavaScript file size (~50-100%)
- Slower initial load (more data to transfer)
- Fine for development, remove for production

### Solution 3: Process Source Maps in ResourceProvider

**Advanced: Serve source maps from C++:**
```cpp
std::optional<juce::WebBrowserComponent::Resource>
AudioPluginAudioProcessorEditor::getResource(const juce::String& path) const
{
    // Serve .js.map files
    if (path.endsWith(".js.map")) {
        auto mapFile = path.fromFirstOccurrenceOf("/", false, false);

        // Check if embedded in BinaryData
        int dataSize = 0;
        const char* data = BinaryData::getNamedResource(
            mapFile.toRawUTF8(), dataSize);

        if (data != nullptr) {
            juce::MemoryInputStream stream(data, dataSize, false);
            return juce::WebBrowserComponent::Resource{
                streamToVector(stream),
                "application/json"  // Source maps are JSON
            };
        }
    }

    // ... handle other resources
}
```

**Requires:** Adding source maps to BinaryData in CMakeLists.txt

### macOS-Specific Debugging

**Use Safari Web Inspector (limited):**
1. Enable Develop menu in Safari → Preferences → Advanced
2. Open plugin in DAW
3. Safari → Develop → [Your Machine] → [Plugin WebView]
4. Limited breakpoint support

**Xcode console logging:**
```bash
# Build with Xcode instead of Ninja
cmake --preset Xcode
open build/YourPlugin.xcodeproj

# Run with debugger, check console for JavaScript errors
```

### Debugging Without Native Functions

**Mock JUCE API for development:**
```javascript
// mock-juce.js (only load from dev server)
if (!window.__JUCE__) {
    window.__JUCE__ = {
        backend: {
            addEventListener: (event, callback) => {
                console.log('[MOCK] addEventListener:', event);
            },
            emitEvent: (event, data) => {
                console.log('[MOCK] emitEvent:', event, data);
            }
        },
        initialisationData: {
            pluginName: "DevPlugin",
            vendor: "DevVendor"
        }
    };

    // Mock native functions
    window.getNativeFunction = (name) => {
        return async (...args) => {
            console.log('[MOCK] Native function called:', name, args);
            return "mock-result";
        };
    };
}
```

**Load mock in dev only:**
```html
<!-- index.html -->
<script>
    // Only load mock when served from localhost
    if (window.location.hostname === 'localhost') {
        import('./mock-juce.js');
    }
</script>
<script type="module" src="js/index.js"></script>
```

### Recommended Workflow

**Development:**
1. Use localhost dev server
2. Mock native functions
3. Full debugging in browser DevTools
4. Hot reload for fast iteration

**Testing native integration:**
1. Switch to ResourceProvider (rebuild)
2. Use `console.log()` for debugging
3. Check browser DevTools console (right-click → Inspect)

**Production:**
1. Use inline source maps OR remove source maps
2. Serve from ResourceProvider
3. Test in release build

### Related Issues
- **Forum Thread:** "Debugging Javascript code for WebView"
- **Affects:** All platforms, but WebView2 (Windows) most restrictive

---

## Right-Click Crash in Logic Pro

### Symptom
**Logic Pro on macOS:**
- Right-clicking WebView repeatedly crashes AU plugin
- Error: "An Audio Unit plug-in reported a problem..."
- First right-click usually works, subsequent clicks crash
- Larger context menus (e.g., on images) crash more reliably

### Root Cause
Bug in JUCE accessibility integration commit `d8c282e`. NSAccessibility framework conflicts with WKWebView context menu presentation.

**Stack trace shows:**
```
AccessibilityHandler::AccessibilityNativeImpl::AccessibilityElement::getAccessibilityWindow
```

### Affected Versions
- **Broken:** JUCE 7.0.11, 7.0.12, JUCE 8.x (early versions)
- **Working:** JUCE 7.0.5 (before accessibility integration)

### Workaround: Revert JUCE Version

**If crashes are critical:**
```bash
cd path/to/JUCE
git checkout 7.0.5
# Or use commit before d8c282e29396473dbb54468532ab2036b1ce49ef
```

**Trade-off:** Lose accessibility features added in 7.0.11+

### Status

**GitHub Issue #1376** - Marked as closed/completed by JUCE team.

**To check if your JUCE version has the fix:**
```bash
cd path/to/JUCE
git log --oneline --all | grep -i "accessibility.*webview\|right.*click.*crash"
```

### Temporary Prevention

**Disable context menus in WebView (not ideal):**
```javascript
// In your JavaScript
document.addEventListener('contextmenu', (e) => {
    e.preventDefault();  // Prevents right-click menu
});
```

**Trade-off:** Users lose right-click functionality (copy/paste, inspect, etc.)

### Testing

**To verify if you're affected:**
1. Build plugin with your JUCE version
2. Load as AU in Logic Pro
3. Right-click WebView element 5-10 times rapidly
4. If crashes → you have the bug

### Related Issues
- **Platform:** macOS only (Logic Pro AU)
- **GitHub Issue:** #1376
- **Introduced:** JUCE 7.0.11 (commit d8c282e)
- **Status:** Fixed (verify your JUCE version)

---

## Escape Key Crash in Ableton

### Symptom
Pressing **Escape key** while WebView plugin open in Ableton 12 (AU format) crashes the plugin.

### Affected Versions
- **JUCE:** 8.0.6
- **DAW:** Ableton Live 12
- **Platform:** macOS

### Root Cause
Unknown. Possibly related to Ableton's modal dialog handling conflicting with WKWebView keyboard events.

### Suggested Workaround

**From JUCE team:**
> "Ensure you're running the latest available release of Ableton Live"

**Check Ableton version:**
- Ableton → About Ableton Live
- Update to latest 12.x if available

### Temporary Prevention

**Intercept Escape key in JavaScript:**
```javascript
// In your JavaScript
document.addEventListener('keydown', (e) => {
    if (e.key === 'Escape') {
        e.preventDefault();
        e.stopPropagation();
        // Handle escape yourself (e.g., close modal, reset state)
    }
});
```

**Trade-off:** Prevents system Escape handling, may break expected behavior

### Status
**Unresolved** as of January 2025. Multiple users confirmed issue but no definitive fix documented.

### Testing
**To verify if you're affected:**
1. Build plugin with JUCE 8.0.6+
2. Load as AU in Ableton Live 12
3. Press Escape key
4. If crashes → you have the bug

### Related Issues
- **Forum Thread:** "Crash in Ableton 12 on Mac with Juce 8.0.6 Webview plugin AU by pressing escape key"
- **First Reported:** February 2025
- **Status:** Open

### Alternative

If critical, consider:
1. Testing with VST3 format instead of AU
2. Updating to latest JUCE develop branch
3. Reporting additional details to JUCE forum if you can reproduce consistently

---

## Quick Reference by Symptom

| Symptom | Problem | Section |
|---------|---------|---------|
| `%3C!DOCTYPE%20html%3E...` displayed | Percent-encoding | [WebView Shows Encoded HTML](#webview-shows-encoded-html-text) |
| "The URL can't be shown" | file:// URLs blocked | ["The URL can't be shown" Error](#the-url-cant-be-shown-error) |
| Blank white screen | Resource provider issue | [Blank White WebView](#blank-white-webview) |
| `window.__JUCE__ is undefined` | Native integration not enabled | [JUCE Object is Undefined](#juce-object-is-undefined) |
| Slider doesn't control audio | Parameter binding broken | [Parameters Not Syncing](#parameters-not-syncing) |
| CORS errors (dev server) | Missing allowedOriginIn | [CORS Errors from Dev Server](#cors-errors-from-dev-server) |
| WebView2 not found (Windows) | NuGet package missing | [WebView2 Not Found](#webview2-not-found-windows) |
| Plugin compiles but UI broken | Validation gap | [Validation Gaps](#validation-gaps) |
| **Crash on reload/reset (release build)** | **Member initialization order** | [WebSliderRelay Crash](#websliderrelay-crash-in-release-builds) |
| **FL Studio blank screen on Tab** | **WKWebView destroyed on focus loss** | [FL Studio Blank Screen](#fl-studio-blank-screen-on-focus-loss) |
| **Must click before controls work (macOS)** | **acceptsFirstMouse not enabled** | [Must Click Before Interaction](#must-click-before-interaction-macos) |
| **Can't debug JavaScript with breakpoints** | **Source maps not loading** | [JavaScript Debugging](#javascript-debugging-challenges) |
| **Right-click crash in Logic** | **Accessibility integration bug** | [Right-Click Crash](#right-click-crash-in-logic-pro) |
| **Escape key crashes Ableton** | **Unknown keyboard event conflict** | [Escape Key Crash](#escape-key-crash-in-ableton) |

---

## Prevention Checklist

Before marking WebView plugin as complete:

### Build Phase
- [ ] JUCE version verified (8.0+)
- [ ] Using resource provider pattern (not data URLs or file:// URLs)
- [ ] `withNativeIntegrationEnabled()` present
- [ ] **Relay → webView → attachment order in header** (critical for release builds)
- [ ] Parameter IDs match between C++ and JavaScript
- [ ] `.withKeepPageLoadedWhenBrowserIsHidden()` if targeting FL Studio

### Test Phase
- [ ] Build succeeds without warnings
- [ ] **Test in RELEASE build** (member order crashes only show in release)
- [ ] Install plugin to system directory
- [ ] Open in at least one DAW (test Logic, Ableton if possible)
- [ ] UI renders (not blank/error/encoded)
- [ ] Move slider → audio changes
- [ ] Change parameter in DAW → UI updates
- [ ] **Reload plugin window 5+ times** (test for crash)
- [ ] **Test focus loss** (click outside plugin, click back, verify controls work)
- [ ] Screenshot taken for documentation

### Code Review Phase
- [ ] No JUCE 7 patterns (data URLs, file:// URLs)
- [ ] Resource provider returns files for all paths
- [ ] Debug logging added to resource provider
- [ ] Browser DevTools accessible (right-click → Inspect)
- [ ] JavaScript errors checked in console

### DAW-Specific Tests (macOS)
- [ ] Logic Pro: Right-click WebView multiple times (check for crash)
- [ ] Ableton 12: Press Escape key (check for crash)
- [ ] FL Studio: Press Tab to cycle windows (check for blank screen)

---

## Need More Help?

| Topic | Doc |
|-------|-----|
| Initial setup | **02-project-setup.md** |
| Resource provider details | **03-communication-patterns.md** |
| Parameter binding | **04-parameter-binding.md** |
| Development workflow | **06-development-workflow.md** |
| Debugging techniques | **06-development-workflow.md (Debugging)** |
| Packaging and distribution | **07-distribution.md** |

---

**Remember:** "It compiles" ≠ "It works". Always test UI visually in a DAW before marking complete.
