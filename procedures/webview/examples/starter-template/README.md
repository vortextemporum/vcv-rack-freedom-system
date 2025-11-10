# JUCE WebView Starter Template

Minimal working WebView plugin with one float parameter (gain).

## Features

- ✅ Single float parameter (gain) with web UI control
- ✅ Basic WebView setup
- ✅ Resource provider with embedded files
- ✅ Production-ready build
- ✅ Hot reloading support
- ✅ ~200 lines of code total

## What's Included

```
starter-template/
├── README.md (this file)
├── CMakeLists.txt (plugin + build config)
├── include/
│   ├── ParameterIDs.h
│   ├── PluginProcessor.h
│   └── PluginEditor.h
├── source/
│   ├── PluginProcessor.cpp
│   └── PluginEditor.cpp
└── ui/
    └── public/
        ├── index.html
        └── js/
            ├── index.js
            └── juce/ (auto-copied)
```

## Quick Start

### 1. Build

```bash
# From this directory
cmake -B build
cmake --build build
```

### 2. Install

**macOS:**
```bash
cp -R build/plugin/WebViewStarter_artefacts/Debug/VST3/WebViewStarter.vst3 \
    ~/Library/Audio/Plug-Ins/VST3/

cp -R build/plugin/WebViewStarter_artefacts/Debug/AU/WebViewStarter.component \
    ~/Library/Audio/Plug-Ins/Components/
```

**Windows:**
```powershell
Copy-Item -Recurse build\plugin\WebViewStarter_artefacts\Debug\VST3\WebViewStarter.vst3 `
    "C:\Program Files\Common Files\VST3\"
```

### 3. Test

1. Open your DAW (Ableton, Reaper, Logic, etc.)
2. Load "WebViewStarter" plugin
3. Move the gain slider - should control audio level

## Customize for Your Plugin

### 1. Rename Plugin

**CMakeLists.txt:**
```cmake
project(YourPluginName VERSION 1.0.0)

juce_add_plugin(${PROJECT_NAME}
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE YRCM  # 4 uppercase letters
    PLUGIN_CODE YPGN               # 4 characters
    PRODUCT_NAME "Your Plugin Name"
)
```

### 2. Add More Parameters

**include/ParameterIDs.h:**
```cpp
const juce::ParameterID GAIN{"GAIN", 1};
const juce::ParameterID YOUR_PARAM{"YOUR_PARAM", 1};  // Add this
```

**source/PluginProcessor.cpp (in createParameterLayout):**
```cpp
layout.add(std::make_unique<juce::AudioParameterFloat>(
    id::YOUR_PARAM, "Your Param",
    juce::NormalisableRange<float>{0.0f, 1.0f}, 0.5f));
```

**Follow full example in:** `../complete-example/` or see docs: `../../04-parameter-binding.md`

### 3. Add Audio Processing

**source/PluginProcessor.cpp (processBlock):**
```cpp
// After line: buffer.applyGain(parameters.gain->get());

// Add your processing here:
// - Filters
// - Distortion
// - Reverb
// - Whatever you want!
```

### 4. Customize UI

**ui/public/index.html:**
```html
<!-- Change styles, add elements, etc. -->
<style>
    body { background: #1a1a1a; color: white; }
</style>
```

**ui/public/js/index.js:**
```javascript
// Add visualizations, effects, interactions
```

## Hot Reloading (Development)

**Start dev server:**
```bash
cd ui/public
npx http-server  # Opens at http://127.0.0.1:8080
```

**In source/PluginEditor.cpp, line ~80:**
```cpp
// Comment out:
// webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

// Uncomment:
webView.goToURL("http://127.0.0.1:8080");
```

**Rebuild once:**
```bash
cmake --build build
```

**Now:** Edit HTML/JS → Refresh WebView (Ctrl+Shift+R) → See changes!

**See:** `../../06-development-workflow.md` for full guide

## File Structure Explained

### CMakeLists.txt
- Defines plugin metadata
- Sets up WebView flags
- Zips and embeds web UI files
- Links JUCE modules

### include/ParameterIDs.h
- Defines parameter IDs
- Shared between C++ and JavaScript

### include/PluginProcessor.h
- Audio processor declaration
- Parameter storage

### source/PluginProcessor.cpp
- Audio processing logic
- Parameter creation

### include/PluginEditor.h
- UI editor declaration
- WebView component

### source/PluginEditor.cpp
- WebView setup
- Parameter bindings
- Resource provider

### ui/public/index.html
- Web UI structure

### ui/public/js/index.js
- JavaScript logic
- Parameter bindings

## What's Different from Complete Example?

| Feature | Starter Template | Complete Example |
|---------|-----------------|------------------|
| Parameters | 1 (gain) | 3 (gain, bypass, distortion) |
| Audio Processing | Gain only | Gain + 2 distortion types |
| Visualization | None | Envelope follower + Plotly |
| C++ GUI | None | Hybrid (C++ + WebView) |
| Communication | Basic (2 patterns) | All 6 patterns |
| Lines of Code | ~200 | ~700 |

**When to use this:**
- Starting a new plugin from scratch
- Learning WebView basics
- Want minimal code to build on

**When to use complete example:**
- Need reference for specific pattern
- Want to see all features
- Building complex plugin

## Troubleshooting

### Build Errors

**"WebView2 not found" (Windows):**
- Install WebView2 Runtime or run NuGet commands from CMake output

**"gtk/gtk.h not found" (Linux):**
```bash
sudo apt-get install libgtk-3-dev libwebkit2gtk-4.1-dev
```

### Runtime Errors

**Blank WebView:**
- Right-click WebView → Inspect → Check console for errors
- Verify JUCE library copied to `ui/public/js/juce/`

**Slider doesn't work:**
- Check parameter ID matches: `GAIN` in both C++ and JavaScript
- Check DevTools console for errors

## Next Steps

1. **Build and test** this template
2. **Customize** for your needs
3. **Add features** from complete example
4. **Read docs** in `../../` for detailed guides

**Need help?**
- Complete example: `../complete-example/`
- Code map: `../REFERENCE-MAP.md`
- Docs index: `../../README.md`
- Quick reference: `../../QUICK-REFERENCE.md`
