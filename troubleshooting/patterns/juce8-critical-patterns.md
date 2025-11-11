# JUCE 8.x Critical Patterns - REQUIRED READING

**Purpose:** Prevent repeat mistakes across all plugin implementations.

**When to read:** Before ANY code generation in Stages 2-5.

---

## 1. CMakeLists.txt - Header Generation (ALWAYS REQUIRED)

### ❌ WRONG (Will fail with "JuceHeader.h not found")
```cmake
target_link_libraries(MyPlugin
    PRIVATE
        juce::juce_audio_processors
)

target_compile_definitions(MyPlugin
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
)
```

### ✅ CORRECT
```cmake
target_link_libraries(MyPlugin
    PRIVATE
        juce::juce_audio_processors
)

# CRITICAL: Generate JuceHeader.h (JUCE 8 requirement)
juce_generate_juce_header(MyPlugin)

target_compile_definitions(MyPlugin
    PUBLIC
        JUCE_VST3_CAN_REPLACE_VST2=0
)
```

**Why:** JUCE 8.x does NOT ship a pre-built JuceHeader.h. The `juce_generate_juce_header()` function:
1. Scans linked JUCE modules
2. Auto-generates JuceHeader.h in build artifacts
3. Adds it to compiler include paths

**Placement:** MUST come after `target_link_libraries()`, BEFORE `target_compile_definitions()`

**Documented in:** `troubleshooting/build-failures/juceheader-not-found-cmake-JUCE-20251106.md`

---

## 2. Include Style - Prefer Module Headers

### ✅ PREFERRED (Modern JUCE 8)
```cpp
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
```

### ⚠️ ACCEPTABLE (But requires juce_generate_juce_header())
```cpp
#include <JuceHeader.h>
```

**Recommendation:** Use individual module headers. They're explicit, don't require CMake generation, and match JUCE 8 best practices.

---

## 3. WebView UI - Module Requirements

### Required CMake Configuration
```cmake
target_link_libraries(MyPlugin
    PRIVATE
        juce::juce_gui_extra  # REQUIRED for WebBrowserComponent
)

target_compile_definitions(MyPlugin
    PUBLIC
        JUCE_WEB_BROWSER=1     # Enable WebView
        JUCE_USE_CURL=0        # Disable CURL (not needed for local HTML)
)
```

### Required Includes
```cpp
#include <juce_gui_extra/juce_gui_extra.h>  // For WebBrowserComponent
```

**Documented in:** Stage 5 (GUI integration)

---

## 4. Bus Configuration - Effects vs Instruments

### Effects (Audio In → Audio Out)
```cpp
AudioProcessor(BusesProperties()
    .withInput("Input", juce::AudioChannelSet::stereo(), true)
    .withOutput("Output", juce::AudioChannelSet::stereo(), true))
```

### Instruments (MIDI In → Audio Out)
```cpp
AudioProcessor(BusesProperties()
    .withOutput("Output", juce::AudioChannelSet::stereo(), true))
```

**Common mistake:** Adding input bus to instruments causes "missing input" errors in DAWs.

**Documented in:** `troubleshooting/runtime-issues/vst3-bus-config-effect-missing-input-Shake-20251106.md`

---

## 5. Threading - UI ↔ Audio Thread

### ❌ WRONG (Thread violation - will crash)
```cpp
// In PluginEditor (UI thread)
button.onClick = [this] {
    audioProcessor.processBlock(...);  // ILLEGAL
};
```

### ✅ CORRECT
```cpp
// In PluginEditor (UI thread) - use parameters or atomic flags
button.onClick = [this] {
    audioProcessor.getAPVTS().getParameter("trigger")->setValueNotifyingHost(1.0f);
};
```

**Rule:** NEVER call audio processing code from UI thread. Use AudioProcessorValueTreeState (APVTS) for communication.

**Documented in:** `troubleshooting/build-failures/ui-trigger-button-processblock-invocation-thread-violation-JUCE-20251106.md`

---

## 6. Rotary Sliders - Bounds vs setBounds

### ❌ WRONG (Non-interactive sliders)
```cpp
slider.setBounds(x, y, w, h);  // Absolute positioning breaks mouse tracking
```

### ✅ CORRECT
```cpp
addAndMakeVisible(slider);
// In resized():
slider.setBounds(x, y, w, h);  // OK in resized()

// OR use FlexBox/Grid layout
```

**Rule:** Sliders need proper parent component hierarchy for mouse events. Absolute positioning works in `resized()` but not during construction.

**Documented in:** `troubleshooting/api-usage/rotary-slider-absolute-positioning-interaction-TapeMachine-20251107.md`

---

## 7. WebView ↔ Parameter Binding

### ❌ WRONG (Parameters not updating)
```cpp
// JavaScript sends:
{ type: "parameterChanged", id: "gain", value: 0.5 }

// C++ expects:
{ type: "parameter_change", ... }
```

### ✅ CORRECT
```cpp
// Standardized event format:
// JS → C++: { type: "parameter_change", id: "gain", value: 0.5 }
// C++ → JS: { type: "parameter_update", id: "gain", value: 0.5 }
```

**Rule:** WebView integration requires exact event type matching between JS and C++.

**Documented in:** `troubleshooting/api-usage/webview-parameter-not-updating-wrong-event-format-JUCE-20251108.md`

---

## 8. WebView Resource Provider - Explicit URL Mapping (ALWAYS REQUIRED)

### ❌ WRONG (Generic loop - breaks resource loading)
```cpp
std::optional<juce::WebBrowserComponent::Resource>
getResource(const juce::String& url)
{
    auto path = url.substring(1);  // Remove leading slash

    // Generic loop - hard to debug, easy to break
    for (int i = 0; i < BinaryData::namedResourceListSize; ++i) {
        if (path == BinaryData::namedResourceList[i]) {
            // ... conversion logic ...
        }
    }
    return std::nullopt;
}
```

**Problem:** BinaryData flattens paths (`js/juce/index.js` → `index_js`), but HTML requests `/js/juce/index.js`. Generic loop can't match.

### ✅ CORRECT (Explicit URL mapping)
```cpp
std::optional<juce::WebBrowserComponent::Resource>
getResource(const juce::String& url)
{
    auto makeVector = [](const char* data, int size) {
        return std::vector<std::byte>(
            reinterpret_cast<const std::byte*>(data),
            reinterpret_cast<const std::byte*>(data) + size
        );
    };

    // Explicit mapping - clear, debuggable, reliable
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

    return std::nullopt;  // 404
}
```

**Why:**
- BinaryData converts paths to valid C++ identifiers (`index.js` → `index_js`)
- HTML/JS use original paths (`./js/juce/index.js`)
- Explicit mapping bridges this gap clearly
- Easy to debug (can log URL mismatches)

**Placement:** PluginEditor.cpp private method

**Documented in:** `troubleshooting/gui-issues/webview-frame-load-interrupted-TapeAge-20251111.md`

---

## 9. CMakeLists.txt - NEEDS_WEB_BROWSER for VST3 (ALWAYS REQUIRED)

### ❌ WRONG (VST3 won't appear in DAW)
```cmake
juce_add_plugin(MyPlugin
    COMPANY_NAME "YourCompany"
    PLUGIN_CODE Plug
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "My Plugin"
    # Missing NEEDS_WEB_BROWSER - VST3 will be built but won't load
)
```

### ✅ CORRECT
```cmake
juce_add_plugin(MyPlugin
    COMPANY_NAME "YourCompany"
    PLUGIN_CODE Plug
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "My Plugin"
    NEEDS_WEB_BROWSER TRUE  # REQUIRED for VST3 WebView support
)
```

**Why:** VST3 format requires explicit WebView flag even if AU works without it. Missing this flag causes:
- VST3 builds successfully
- Binary exists in build artifacts
- But plugin doesn't appear in DAW VST3 scanner
- Only AU format visible

**When:** ANY plugin using WebBrowserComponent for UI

**Documented in:** `troubleshooting/gui-issues/webview-frame-load-interrupted-TapeAge-20251111.md`

---

## 10. Testing GUI Changes - Always Install to System (CRITICAL WORKFLOW)

### ❌ WRONG (Tests stale cached builds)
```bash
# Build but DON'T install
./scripts/build-and-install.sh MyPlugin --no-install

# Test in DAW → loads OLD version from system folders
```

**Result:** DAW loads stale plugins from `~/Library/Audio/Plug-Ins/`, not fresh builds.

### ✅ CORRECT
```bash
# Build AND install to system folders
./scripts/build-and-install.sh MyPlugin

# Script automatically:
# 1. Builds fresh binaries
# 2. Removes old versions from system
# 3. Installs new versions
# 4. Clears DAW caches (Ableton DB, AU cache)
# 5. Verifies installation
```

**Then restart DAW** - Required for plugin rescan after cache clear.

**Why:**
- DAWs load plugins from system folders (`~/Library/Audio/Plug-Ins/`), NOT build directories
- Using `--no-install` means testing old builds while developing new code
- All "fixes" appear broken when you're testing stale binaries

**When:** ANY time you modify:
- PluginEditor code (UI changes)
- CMakeLists.txt (configuration)
- Resource files (HTML/CSS/JS)
- Parameter bindings

**Documented in:** `troubleshooting/gui-issues/webview-frame-load-interrupted-TapeAge-20251111.md`

---

## 11. WebView Member Initialization - Use std::unique_ptr (REQUIRED)

### ❌ WRONG (Raw members - initialization order issues)
```cpp
class MyPluginEditor : public juce::AudioProcessorEditor {
private:
    juce::WebSliderRelay gainRelay;
    juce::WebBrowserComponent webView;
    juce::WebSliderParameterAttachment gainAttachment;
};
```

### ✅ CORRECT
```cpp
class MyPluginEditor : public juce::AudioProcessorEditor {
private:
    // Order: Relays → WebView → Attachments
    std::unique_ptr<juce::WebSliderRelay> gainRelay;
    std::unique_ptr<juce::WebBrowserComponent> webView;
    std::unique_ptr<juce::WebSliderParameterAttachment> gainAttachment;
};
```

**Constructor:**
```cpp
MyPluginEditor::MyPluginEditor(MyProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // 1. Create relays FIRST
    gainRelay = std::make_unique<juce::WebSliderRelay>("GAIN");

    // 2. Create WebView with relay options
    webView = std::make_unique<juce::WebBrowserComponent>(
        juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            .withResourceProvider([this](auto& url) { return getResource(url); })
            .withOptionsFrom(*gainRelay)
    );

    // 3. Create attachments LAST
    gainAttachment = std::make_unique<juce::WebSliderParameterAttachment>(
        *processorRef.parameters.getParameter("GAIN"), *gainRelay
    );

    addAndMakeVisible(*webView);
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
}
```

**Why:**
- Ensures correct construction order
- Prevents release build crashes (90% of member order issues)
- Destruction order is automatic (reverse of declaration)
- Matches JUCE examples and GainKnob reference implementation

**When:** ALL WebView-based plugin editors

**Documented in:** `troubleshooting/gui-issues/webview-frame-load-interrupted-TapeAge-20251111.md`

---

## Usage Instructions

### For Subagents (foundation-agent, shell-agent, dsp-agent, gui-agent)

Add to your prompt:

```markdown
## CRITICAL: Before Implementation

Read and internalize JUCE 8 critical patterns:
@troubleshooting/patterns/juce8-critical-patterns.md

These are non-negotiable requirements. Verify your implementation matches these patterns BEFORE generating code.
```

### For Human Developers

Review this file before:
- Creating new plugins
- Modifying CMakeLists.txt
- Adding WebView UI
- Debugging build/runtime issues

---

## References

All patterns documented with full context in:
- `troubleshooting/build-failures/`
- `troubleshooting/runtime-issues/`
- `troubleshooting/api-usage/`
- `troubleshooting/gui-issues/`

See individual issue files for:
- Full problem descriptions
- What didn't work
- Why the solution works
- Prevention strategies
