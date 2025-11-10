# Project Setup - Complete Guide

## Prerequisites

- CMake 3.22 or later
- C++20 compatible compiler
- Git (for JUCE submodule or CPM)
- **Windows only:** PowerShell (for WebView2 NuGet package)

## Step 1: Initialize Git Repository

```bash
git init
git submodule add https://github.com/juce-framework/JUCE.git libs/juce
git submodule update --init --recursive
```

**Alternative: Use CPM (C++ Package Manager)**
```cmake
# In CMakeLists.txt
include(cmake/CPM.cmake)
CPMAddPackage("gh:juce-framework/JUCE#8.0.6")
```

## Step 2: Root CMakeLists.txt

Create `CMakeLists.txt` in project root:

```cmake
cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

project(JuceWebViewPlugin VERSION 0.1.0)

# Set C++ standard BEFORE adding JUCE
set(CMAKE_CXX_STANDARD 20)

# Add JUCE
add_subdirectory(libs/juce)

# Add plugin subdirectory
add_subdirectory(plugin)
```

## Step 3: Plugin CMakeLists.txt

Create `plugin/CMakeLists.txt`:

```cmake
# Define plugin target
juce_add_plugin(JuceWebViewPlugin
    COMPANY_NAME "YourCompany"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE YRCM  # 4 uppercase letters
    PLUGIN_CODE JWVP               # 4 characters
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "JuceWebViewPlugin"

    # CRITICAL: Enable WebView2 on Windows
    NEEDS_WEB_VIEW2 TRUE
)

# Add source files
target_sources(JuceWebViewPlugin PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
)

# Add include directories
target_include_directories(JuceWebViewPlugin PRIVATE
    include
)

# CRITICAL: Enable WebView and configure WebView2
target_compile_definitions(JuceWebViewPlugin PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1  # Windows: static linking
)

# Link JUCE modules
target_link_libraries(JuceWebViewPlugin PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_extra  # Required for WebBrowserComponent
    juce::juce_dsp        # If using audio processing
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
```

### Windows-Specific WebView2 Setup

The `NEEDS_WEB_VIEW2 TRUE` flag tells JUCE to find WebView2. On first build:

```powershell
# PowerShell commands JUCE will show if WebView2 not found:
nuget sources add -Name "Nuget official package source" `
    -Source "https://api.nuget.org/v3/index.json"

nuget install Microsoft.Web.WebView2 -Version 1.0.2792.45 `
    -OutputDirectory [path]
```

**JUCE handles this automatically in newer versions**, but if you get WebView2 errors, run these manually.

## Step 4: Create Header Files

### plugin/include/PluginName/PluginProcessor.h

```cpp
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

class AudioPluginAudioProcessor : public juce::AudioProcessor
{
public:
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "PluginName"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // CRITICAL: Public accessor for parameter state
    juce::AudioProcessorValueTreeState& getState() { return state; }

private:
    juce::AudioProcessorValueTreeState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
```

### plugin/include/PluginName/PluginEditor.h

```cpp
#pragma once
#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void resized() override;
    void timerCallback() override;

private:
    // Resource provider for serving files to WebView
    using Resource = juce::WebBrowserComponent::Resource;
    std::optional<Resource> getResource(const juce::String& url) const;

    AudioPluginAudioProcessor& processorRef;

    // The WebView component
    juce::WebBrowserComponent webView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
```

## Step 5: Implement Basic Plugin Editor

### plugin/source/PluginEditor.cpp

```cpp
#include "PluginName/PluginEditor.h"

namespace {
    // Helper: Convert stream to byte vector
    std::vector<std::byte> streamToVector(juce::InputStream& stream) {
        const auto sizeInBytes = static_cast<size_t>(stream.getTotalLength());
        std::vector<std::byte> result(sizeInBytes);
        stream.setPosition(0);
        [[maybe_unused]] const auto bytesRead = stream.read(result.data(), result.size());
        jassert(bytesRead == static_cast<ssize_t>(sizeInBytes));
        return result;
    }

    // Helper: Get MIME type for file extension
    const char* getMimeForExtension(const juce::String& extension) {
        static const std::unordered_map<juce::String, const char*> mimeMap = {
            {{"html"}, "text/html"},
            {{"js"}, "text/javascript"},
            {{"css"}, "text/css"},
            {{"json"}, "application/json"},
            {{"png"}, "image/png"},
            {{"jpg"}, "image/jpeg"},
            {{"svg"}, "image/svg+xml"},
            {{"woff2"}, "font/woff2"}
        };
        if (const auto it = mimeMap.find(extension.toLowerCase()); it != mimeMap.end())
            return it->second;
        jassertfalse;
        return "application/octet-stream";
    }
}

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView(juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(
              juce::WebBrowserComponent::Options::WinWebView2{}
                  .withBackgroundColour(juce::Colours::white)
                  // CRITICAL on Windows: Avoid permission issues in some DAWs
                  .withUserDataFolder(juce::File::getSpecialLocation(
                      juce::File::SpecialLocationType::tempDirectory)))
          .withNativeIntegrationEnabled()
          .withResourceProvider(
              [this](const auto& url) { return getResource(url); }))
{
    addAndMakeVisible(webView);

    // Go to resource provider (serves files from C++)
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    setResizable(true, true);
    setSize(800, 600);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

void AudioPluginAudioProcessorEditor::resized() {
    webView.setBounds(getBounds());
}

void AudioPluginAudioProcessorEditor::timerCallback() {
    // Implement later for parameter updates
}

auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    // Determine which file is requested
    const auto resourceToRetrieve =
        url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

    // TODO: Implement file serving (see Distribution docs)
    // For now, return empty - will add in later steps
    return std::nullopt;
}
```

## Step 6: Create Basic Web UI

### plugin/ui/public/index.html

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <script type="module" src="js/index.js"></script>
    <title>JUCE WebView Plugin</title>
</head>
<body>
    <main>
        <h1>Hello from WebView!</h1>
        <p>JUCE backend is connected.</p>
    </main>
</body>
</html>
```

### plugin/ui/public/js/index.js

```javascript
import * as Juce from "./juce/index.js";

console.log("JUCE frontend library loaded");
console.log("Backend object:", window.__JUCE__.backend);
```

## Step 7: Copy JUCE Frontend Library

The JUCE frontend JavaScript library must be copied from JUCE to your UI folder.

### Add to plugin/CMakeLists.txt:

```cmake
# Copy JUCE frontend library to UI folder
file(COPY
    "${JUCE_MODULES_DIR}/juce_gui_extra/native/javascript/"
    DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/ui/public/js/juce/"
)
```

**Location:** `${JUCE_MODULES_DIR}/juce_gui_extra/native/javascript/`

**Note:** The path is `javascript/` not `javaScriptInterface/`. JUCE_MODULES_DIR is set by JUCE's CMake configuration.

This copies:
- `index.js` - Main JUCE library
- `check_native_interop.js` - Connection validator

## Step 8: Generate and Build

```bash
# Generate project (use appropriate preset)
cmake --preset default  # Or: Xcode, vs, release

# Build
cmake --build --preset default

# Install to system plugin folders (see Installation section below)
```

### CMake Presets (CMakePresets.json)

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "default",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug"
            }
        },
        {
            "name": "release",
            "inherits": "default",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        },
        {
            "name": "Xcode",
            "generator": "Xcode",
            "binaryDir": "${sourceDir}/xcode-build"
        }
    ],
    "buildPresets": [
        {
            "name": "default",
            "configurePreset": "default"
        }
    ]
}
```

## Step 9: Install Plugin for Testing

### macOS

```bash
# VST3
cp -R build/plugin/YourPlugin_artefacts/Release/VST3/YourPlugin.vst3 \
    ~/Library/Audio/Plug-Ins/VST3/

# AU
cp -R build/plugin/YourPlugin_artefacts/Release/AU/YourPlugin.component \
    ~/Library/Audio/Plug-Ins/Components/
```

### Windows

```powershell
# VST3 (requires admin)
Copy-Item -Recurse build\plugin\YourPlugin_artefacts\Release\VST3\YourPlugin.vst3 `
    "C:\Program Files\Common Files\VST3\"
```

### Linux

```bash
# VST3
cp -R build/plugin/YourPlugin_artefacts/Release/VST3/YourPlugin.vst3 \
    ~/.vst3/
```

## Common Issues and Fixes

### Issue: WebView2 Not Found (Windows)

**Error:** `CMake Warning at ... WebView2 wasn't found in the local NuGet folder`

**Fix:** Run the NuGet commands shown in CMake output, or update to latest JUCE which handles this automatically.

### Issue: gtk/gtk.h Not Found (Linux)

**Error:** `fatal error: gtk/gtk.h: No such file or directory`

**Fix:**
```bash
# Ubuntu/Debian
sudo apt-get install libgtk-3-dev libwebkit2gtk-4.1-dev

# Fedora
sudo dnf install gtk3-devel webkit2gtk4.1-devel
```

### Issue: Blank WebView Window

**Cause:** Resource provider not returning files, or files not found.

**Debug:**
1. Add `std::cout` in `getResource()` to log requested URLs
2. Check that `index.html` exists in `ui/public/`
3. Verify JUCE library was copied to `ui/public/js/juce/`

### Issue: "Native integration features will not work"

**Cause:** `withNativeIntegrationEnabled()` not called, or JUCE library not loaded.

**Fix:** Ensure `.withNativeIntegrationEnabled()` in Options and JUCE library imported in JavaScript.

## Verification Steps

After setup, verify:

1. **Build succeeds** - No CMake or compile errors
2. **Plugin loads** - Standalone app launches
3. **WebView displays** - Window shows HTML content (not blank)
4. **Console works** - Right-click WebView → Inspect → Console shows no errors
5. **JUCE object exists** - Console: `window.__JUCE__` returns object

## Next Steps

Continue to **03-communication-patterns.md** to implement C++ ↔ JavaScript interaction.
