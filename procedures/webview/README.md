# JUCE 8 WebView Plugin Documentation

Complete guide to building audio plugins with web-based UIs using JUCE 8.

## 📚 Documentation Series

### [01 - Overview](./01-overview.md)
**What you'll learn:**
- What WebView plugins are and how they work
- Architecture and key concepts
- When to use WebViews vs traditional JUCE GUI
- Platform backends and requirements
- Project structure overview

**Start here if:** You're new to JUCE WebView or want to understand the big picture.

---

### [02 - Project Setup](./02-project-setup.md)
**What you'll learn:**
- Complete CMake configuration
- JUCE integration (submodule or CPM)
- Windows WebView2 setup
- Linux/macOS platform configuration
- Initial plugin code structure
- Build and installation process

**Start here if:** You're ready to create your first WebView plugin project.

---

### [03 - Communication Patterns](./03-communication-patterns.md)
**What you'll learn:**
- **C++ → JavaScript:** Evaluate JS, emit events, native functions, init data
- **JavaScript → C++:** Call native functions, emit events
- Resource provider pattern (serving files from C++)
- Complete bidirectional communication examples
- Thread safety and best practices
- Debugging techniques

**Start here if:** You need to pass data between C++ backend and JavaScript frontend.

---

### [04 - Parameter Binding](./04-parameter-binding.md)
**What you'll learn:**
- The Relay Pattern (core concept for parameter binding)
- **Float parameters (Slider):** `WebSliderRelay`, `Juce.getSliderState()`
- **Bool parameters (Checkbox):** `WebToggleButtonRelay`, `Juce.getToggleState()`
- **Choice parameters (ComboBox):** `WebComboBoxRelay`, `Juce.getComboBoxState()`
- Complete multi-parameter examples
- Initialization order (critical!)
- Common issues and fixes

**Start here if:** You want UI controls (sliders, checkboxes, dropdowns) to control audio parameters.

---

### [05 - Audio Visualization](./05-audio-visualization.md)
**What you'll learn:**
- Measuring audio data (envelope following, FFT, etc.)
- Exposing data via resource provider
- Timed events pattern (60ms update loop)
- Using Plotly.js for charts
- Canvas-based visualization for performance
- FFT spectrum analyzer example
- Performance optimization techniques

**Start here if:** You want to display real-time audio data (meters, spectrograms, waveforms).

---

### [06 - Development Workflow](./06-development-workflow.md)
**What you'll learn:**
- Setting up local development server (http-server)
- Hot reloading workflow (change HTML/JS without recompiling C++)
- Hard refresh to bypass cache
- Browser DevTools debugging
- Logging C++ ↔ JavaScript communication
- Testing in DAWs (Ableton, Reaper, Logic)
- Production vs development builds
- Common development issues and solutions

**Start here if:** You want to iterate quickly on UI without recompiling C++ every time.

---

### [07 - Distribution](./07-distribution.md)
**What you'll learn:**
- Embedding web UI files in plugin binary (Zip + Binary Data)
- Implementing resource provider for embedded files
- Platform runtime requirements:
  - **Windows:** WebView2 runtime installation
  - **macOS:** No extra setup (built-in)
  - **Linux:** libwebkit2gtk package
- Creating installers (NSIS, Packages, .deb)
- Code signing and notarization
- Complete CMake build configuration
- Testing and verification

**Start here if:** You're ready to package and ship your plugin to users.

---

### [Common Problems](./common-problems.md)
**What you'll learn:**
- Troubleshooting WebView rendering issues
- JUCE 7 vs JUCE 8 API migration
- Solving encoded HTML, blank screens, and URL errors
- Parameter binding debugging
- Development environment setup issues
- Validation gaps ("it compiles" ≠ "it works")
- Prevention checklist

**Start here if:** Your plugin isn't rendering correctly or you're hitting errors.

---

### [Best Practices](./best-practices.md)
**What you'll learn:**
- Critical member declaration order (prevents release build crashes)
- Constructor initialization patterns
- WebView options configuration
- Resource provider implementation patterns
- Development vs production build switching
- Error handling patterns
- Complete testing checklist

**Start here if:** You want to ensure your plugin works reliably like the example code.

---

## 🚀 Quick Start

### 1. Complete Beginner
Read in order: **01 → 02 → 03 → 04 → 06 → 07**

Skip 05 (Audio Visualization) if you don't need real-time meters/graphs.

### 2. Experienced JUCE Developer
Jump to: **02** (Project Setup) → **03** (Communication) → **04** (Parameters)

You already understand audio plugins, so focus on WebView-specific concepts.

### 3. Web Developer New to JUCE
Start with: **01** (Overview) → **02** (Project Setup) → **03** (Communication)

Then read JUCE documentation on audio processing before continuing.

### 4. Specific Problem Solving

| Problem | Doc to Read |
|---------|-------------|
| "How do I bind a slider to a parameter?" | 04 - Parameter Binding |
| "What's the correct member order?" | **Best Practices** (Member Declaration Order) |
| "Plugin crashes on reload in release build" | **Best Practices** + **Common Problems** |
| "WebView shows encoded HTML text" | **Common Problems** |
| "The URL can't be shown error" | **Common Problems** |
| "WebView shows blank white screen" | **Common Problems** or 06 - Development Workflow |
| "Parameters not syncing" | **Common Problems** or 04 - Parameter Binding |
| "CORS error from dev server" | **Common Problems** or 06 - Development Workflow |
| "How do I display audio levels?" | 05 - Audio Visualization |
| "Plugin works in dev but not release" | 07 - Distribution |
| "WebView2 not found error" | **Common Problems** or 02 - Project Setup |
| "How to call C++ from JavaScript?" | 03 - Communication Patterns |
| "Plugin compiles but UI broken" | **Common Problems** (Validation Gaps) |
| "How should I structure my code?" | **Best Practices** (Complete patterns) |

---

## 🎯 Key Concepts

### The Relay Pattern
WebView parameters use **relay classes** to bridge C++ and JavaScript:

```
AudioParameter ↔ ParameterAttachment ↔ Relay ↔ JavaScript State Object ↔ HTML Element
```

**Three relay types:**
- `WebSliderRelay` - For float parameters
- `WebToggleButtonRelay` - For bool parameters
- `WebComboBoxRelay` - For choice parameters

### Resource Provider
Serves files to WebView from C++ (like a local web server):

```cpp
.withResourceProvider([this](const auto& url) {
    // Return HTML, CSS, JS, JSON, images, etc.
    return getResource(url);
})
```

### Native Integration
Enables C++ ↔ JavaScript communication:

```cpp
.withNativeIntegrationEnabled()  // CRITICAL for JUCE frontend library
```

Must import JUCE library in JavaScript:
```javascript
import * as Juce from "./juce/index.js";
```

---

## 📖 Code Examples

### Minimal Working Plugin

**PluginEditor.cpp:**
```cpp
#include "PluginEditor.h"

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView(juce::WebBrowserComponent::Options{}
          .withNativeIntegrationEnabled()
          .withResourceProvider([this](const auto& url) { return getResource(url); }))
{
    addAndMakeVisible(webView);
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    setSize(800, 600);
}

auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    // Serve embedded web files (see Distribution docs for implementation)
    return std::nullopt;
}
```

**index.html:**
```html
<!DOCTYPE html>
<html>
<head>
    <script type="module" src="js/index.js"></script>
</head>
<body>
    <h1>Hello JUCE WebView!</h1>
</body>
</html>
```

**index.js:**
```javascript
import * as Juce from "./juce/index.js";
console.log("JUCE backend connected:", window.__JUCE__.backend);
```

---

## 🔧 Platform Support

| Platform | Backend | Runtime | Notes |
|----------|---------|---------|-------|
| **Windows** | WebView2 (Chromium) | Requires WebView2 Runtime | Static or dynamic linking |
| **macOS** | WKWebView | Built-in (10.11+) | No extra setup |
| **Linux** | WebKit2GTK | Requires libwebkit2gtk-4.1 | User must install |
| **iOS** | WKWebView | Built-in (iOS 8+) | No extra setup |
| **Android** | Android WebView | Built-in (5.0+) | No extra setup |

---

## 🐛 Common Issues

**For detailed troubleshooting, see [common-problems.md](./common-problems.md)**

### Quick Symptom Lookup

| Symptom | Quick Fix | Detailed Docs |
|---------|-----------|---------------|
| UI shows `%3C!DOCTYPE%20html%3E...` | Use resource provider, not data URLs | [common-problems.md](#webview-shows-encoded-html-text) |
| "The URL can't be shown" error | Use resource provider, not `file://` URLs | [common-problems.md](#the-url-cant-be-shown-error) |
| Blank white WebView | Check resource provider returns files | [common-problems.md](#blank-white-webview) |
| `window.__JUCE__ is undefined` | Add `.withNativeIntegrationEnabled()` | [common-problems.md](#juce-object-is-undefined) |
| Parameters not syncing | Check initialization order (relay before webView) | [common-problems.md](#parameters-not-syncing) |
| CORS errors (dev server) | Add `allowedOriginIn` to resource provider | [common-problems.md](#cors-errors-from-dev-server) |
| WebView2 not found (Windows) | Run NuGet commands from CMake output | [common-problems.md](#webview2-not-found-windows) |

### Most Common Issues

**1. Using JUCE 7 patterns in JUCE 8**
- ❌ Data URLs: `webView.goToURL("data:text/html,...")`
- ❌ File URLs: `webView.goToURL("file:///...")`
- ✅ Resource provider: `webView.goToURL(getResourceProviderRoot())`

**2. "It compiles" ≠ "It works"**
- Compilation success doesn't guarantee UI renders
- Always test visually in a DAW before marking complete
- See [common-problems.md (Validation Gaps)](#validation-gaps)

**3. Wrong initialization order**
- Relay must initialize BEFORE webView in constructor
- See [04-parameter-binding.md (Initialization Order)](./04-parameter-binding.md)

---

## 📦 Repository Structure

```
juce-webview-tutorial/
├── docs/                   # This documentation
│   ├── README.md          # This file
│   ├── 01-overview.md
│   ├── 02-project-setup.md
│   ├── 03-communication-patterns.md
│   ├── 04-parameter-binding.md
│   ├── 05-audio-visualization.md
│   ├── 06-development-workflow.md
│   └── 07-distribution.md
├── plugin/                 # Plugin source code
│   ├── CMakeLists.txt
│   ├── include/
│   ├── source/
│   └── ui/public/         # Web UI files
├── transcripts/           # Original tutorial transcripts
├── CMakeLists.txt
└── README.md             # Project README
```

---

## 🎓 Learning Path

### Beginner Path (First WebView Plugin)
1. Read **01-overview.md** - Understand concepts
2. Follow **02-project-setup.md** - Build basic plugin
3. Study **03-communication-patterns.md** - Log "Hello from C++" in browser console
4. Complete **04-parameter-binding.md** - Add a gain slider
5. Use **06-development-workflow.md** - Set up hot reloading
6. Package with **07-distribution.md** - Ship it!

### Intermediate Path (Add WebView to Existing Plugin)
1. Skim **01-overview.md** - Refresh on architecture
2. Jump to **02-project-setup.md** - Add WebView to your CMakeLists.txt
3. Implement **04-parameter-binding.md** - Convert existing parameters to WebView
4. Add **06-development-workflow.md** - Set up dev workflow
5. Deploy with **07-distribution.md** - Update your installer

### Advanced Path (Complex Audio Visualizations)
1. Review **03-communication-patterns.md** - Resource provider pattern
2. Master **05-audio-visualization.md** - FFT, envelope following
3. Optimize with **06-development-workflow.md** - Profile and debug
4. Canvas rendering techniques from **05-audio-visualization.md**

---

## 🔗 External Resources

### JUCE
- Official Docs: https://docs.juce.com/
- Forum: https://forum.juce.com/
- GitHub: https://github.com/juce-framework/JUCE

### WebView Platforms
- **WebView2 (Windows):** https://docs.microsoft.com/en-us/microsoft-edge/webview2/
- **WKWebView (macOS/iOS):** https://developer.apple.com/documentation/webkit/wkwebview
- **WebKit2GTK (Linux):** https://webkitgtk.org/

### Web Technologies
- MDN Web Docs: https://developer.mozilla.org/
- JavaScript Modules: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Modules
- Canvas API: https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API

### Visualization Libraries
- **Plotly.js:** https://plotly.com/javascript/
- **D3.js:** https://d3js.org/
- **Chart.js:** https://www.chartjs.org/

---

## 💡 Tips for LLMs Reading This

### When User Asks About...

**"How do I create a WebView plugin?"**
→ Point to **02-project-setup.md** (complete setup guide)

**"How to control parameters from web UI?"**
→ Point to **04-parameter-binding.md** (relay pattern)

**"WebView is blank / not loading"**
→ Point to **06-development-workflow.md** (debugging section)

**"How to pass data C++ ↔ JavaScript?"**
→ Point to **03-communication-patterns.md** (all communication methods)

**"How to display audio meter / spectrum?"**
→ Point to **05-audio-visualization.md** (complete examples)

**"How to package for release?"**
→ Point to **07-distribution.md** (zip + binary data pattern)

### Key Architecture Points

1. **Relay classes** bridge parameters to JavaScript (not direct binding)
2. **Resource provider** serves files from C++ (like local web server)
3. **Native integration** must be enabled for JUCE library to work
4. **Initialization order matters:** relay → webView → attachment
5. **Thread safety:** All callbacks on GUI thread, use atomics for audio data

### Common Mistakes to Watch For

- Forgetting `.withNativeIntegrationEnabled()`
- Wrong parameter ID (C++ vs JavaScript mismatch)
- Relay initialized after webView (constructor order)
- Dev server URL in production build
- Missing `allowedOriginIn` for dev server CORS
- Not calling hard refresh after JS changes

---

## 📝 Document Conventions

### Code Blocks

**C++ code:**
```cpp
// Always shows complete, compilable examples
// Includes necessary headers and namespaces
```

**JavaScript code:**
```javascript
// Uses modern ES6+ syntax
// Shows complete working examples
```

**CMake code:**
```cmake
# Shows exact placement in CMakeLists.txt
# Comments explain why each line is needed
```

### Emphasis

- **CRITICAL:** Must follow this or plugin won't work
- **Important:** Strongly recommended, avoids common issues
- **Note:** Additional context or explanation

### File Paths

All file paths use **absolute paths** from project root:
```
/plugin/source/PluginEditor.cpp  (not: ../source/PluginEditor.cpp)
```

---

## 🎯 Next Steps

**New to WebView plugins?**
→ Start with **01-overview.md**

**Ready to build?**
→ Jump to **02-project-setup.md**

**Have a specific question?**
→ Check the "Specific Problem Solving" table above

**Want to see it in action?**
→ Explore the example code in `/plugin/` directory

---

**Good luck building amazing audio plugins with WebView UIs!** 🚀
