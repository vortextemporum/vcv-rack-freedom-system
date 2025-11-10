# JUCE 8 WebView Plugin System - Overview

## What is a WebView Plugin?

A JUCE WebView plugin embeds web technologies (HTML, CSS, JavaScript) as the UI for an audio plugin while maintaining C++ audio processing. The WebView runs alongside traditional JUCE C++ GUI components.

## Key Concepts

### WebView Component
`juce::WebBrowserComponent` - The container that displays web content in your plugin UI. It can:
- Display arbitrary URLs (web or local dev server)
- Serve files from C++ via a resource provider
- Execute JavaScript from C++
- Expose C++ functions to JavaScript
- Emit and receive events across the C++/JS boundary

### Architecture Pattern

```
Audio Processing (C++)
    ↓
Audio Parameters (AudioProcessorValueTreeState)
    ↓
Parameter Attachments
    ↓
Relay Classes ← → WebBrowserComponent ← → JavaScript Frontend
    ↓
JavaScript State Objects (SliderState, ToggleState, ComboBoxState)
    ↓
HTML UI Elements (input, checkbox, select, etc.)
```

### Two-Way Communication

**C++ → JavaScript:**
- Evaluate JavaScript code
- Emit events
- Expose native functions
- Pass initialization data
- Serve resources (files, JSON, etc.)

**JavaScript → C++:**
- Call native functions
- Emit events
- Fetch resources from backend

### Platform Backends

- **Windows:** WebView2 (Edge/Chromium) - requires runtime installation
- **macOS/iOS:** WKWebView (WebKit) - built-in, no extra setup
- **Linux:** libwebkit2gtk - requires package installation
- **Android:** Android WebView - built-in

## When to Use WebViews

**Good Use Cases:**
- Rich, modern UIs with complex layouts
- Data visualization (charts, graphs, spectrograms)
- Leveraging existing web frameworks (React, Vue, Svelte)
- Rapid UI iteration during development
- Cross-platform UI consistency

**Considerations:**
- Larger binary size (especially with static linking)
- Additional runtime dependencies (Windows)
- Slightly more complex setup than pure JUCE GUI
- Web security model restrictions

## Workflow Comparison

### Traditional JUCE Plugin
1. Create AudioParameter in processor
2. Create JUCE component (Slider, Button, etc.) in editor
3. Create ParameterAttachment to bind them
4. Recompile for every UI change

### WebView Plugin
1. Create AudioParameter in processor
2. Create Relay class and ParameterAttachment in editor
3. Create HTML element in web UI
4. Bind JavaScript state to HTML element
5. **Hot reload UI changes without recompiling** (during development)

## Project Structure

```
juce-webview-plugin/
├── CMakeLists.txt              # Root CMake configuration
├── plugin/
│   ├── CMakeLists.txt          # Plugin target definition
│   ├── include/
│   │   └── PluginName/
│   │       ├── PluginProcessor.h
│   │       ├── PluginEditor.h
│   │       └── ParameterIDs.h
│   ├── source/
│   │   ├── PluginProcessor.cpp
│   │   └── PluginEditor.cpp
│   └── ui/
│       └── public/             # Web UI files
│           ├── index.html
│           └── js/
│               ├── index.js
│               └── juce/       # JUCE frontend library (auto-copied)
└── libs/
    └── juce/                   # JUCE as submodule or CPM dependency
```

## Build Process Overview

1. CMake generates project files
2. JUCE downloads automatically (via CPM)
3. Web UI files are zipped (production) or served via dev server (development)
4. Plugin binary is built with embedded resources (production build)
5. Plugin installed to system directories (VST3, AU, etc.)

## Next Steps

Continue to:
- **02-project-setup.md** - Complete CMake and project configuration
- **03-communication-patterns.md** - C++ ↔ JavaScript interaction
- **04-parameter-binding.md** - Control audio parameters from web UI
- **05-audio-visualization.md** - Display audio data in web UI
- **06-development-workflow.md** - Hot reloading and debugging
- **07-distribution.md** - Packaging and shipping plugins

## Quick Reference

### Essential Includes
```cpp
#include <juce_gui_extra/juce_gui_extra.h>  // WebBrowserComponent
#include <juce_audio_processors/juce_audio_processors.h>
```

### Essential CMake Flags
```cmake
JUCE_WEB_BROWSER=1                          # Enable WebView
JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1 # Windows only
needs_web_view_2=TRUE                        # Windows only (in juce_add_plugin)
```

### Essential JavaScript Imports
```javascript
import * as Juce from "./juce/index.js";
```

### Core Classes

**C++ Side:**
- `juce::WebBrowserComponent` - The WebView container
- `juce::WebSliderRelay` - Exposes slider state to JavaScript
- `juce::WebToggleButtonRelay` - Exposes toggle state to JavaScript
- `juce::WebComboBoxRelay` - Exposes combobox state to JavaScript
- `juce::WebSliderParameterAttachment` - Binds parameter to web slider
- `juce::WebToggleButtonParameterAttachment` - Binds parameter to web toggle
- `juce::WebComboBoxParameterAttachment` - Binds parameter to web combobox

**JavaScript Side:**
- `Juce.getSliderState(paramId)` - Get slider state object
- `Juce.getToggleState(paramId)` - Get toggle state object
- `Juce.getComboBoxState(paramId)` - Get combobox state object
- `Juce.getNativeFunction(name)` - Get callable C++ function
- `Juce.getBackendResourceAddress(path)` - Get C++ resource URL
- `window.__JUCE__.backend` - Event system
- `window.__JUCE__.initialisationData` - Startup data from C++
