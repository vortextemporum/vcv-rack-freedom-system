# JUCE WebView Examples

This directory contains working code examples demonstrating all patterns from the documentation.

## Directory Structure

```
examples/
├── README.md (this file)
├── REFERENCE-MAP.md (maps code → docs)
├── complete-example/ (full-featured plugin)
│   ├── README.md
│   ├── CMakeLists.txt
│   ├── include/
│   ├── source/
│   └── ui/
└── starter-template/ (minimal starting point)
    ├── README.md
    ├── CMakeLists.txt
    ├── include/
    ├── source/
    └── ui/
```

## Examples Included

### 1. Complete Example (`complete-example/`)

**Full-featured plugin demonstrating:**
- ✅ All three parameter types (float, bool, choice)
- ✅ C++/JavaScript bidirectional communication
- ✅ Audio visualization (envelope follower + Plotly)
- ✅ Audio processing (distortion types)
- ✅ Hybrid UI (C++ GUI + WebView side-by-side)
- ✅ Resource provider with embedded files
- ✅ Hot reloading support (dev server)
- ✅ Production build ready

**Reference implementation for:**
- 02-project-setup.md
- 03-communication-patterns.md
- 04-parameter-binding.md
- 05-audio-visualization.md
- 06-development-workflow.md
- 07-distribution.md

### 2. Starter Template (`starter-template/`)

**Minimal plugin showing:**
- ✅ Basic WebView setup
- ✅ Single float parameter (gain)
- ✅ Simple HTML/JS frontend
- ✅ Resource provider
- ✅ Ready to build and extend

**Best for:**
- Starting a new WebView plugin from scratch
- Learning the minimal requirements
- Clean slate without extra features

---

## How to Use These Examples

### Option 1: Study the Complete Example

**Learn from working code:**

```bash
cd examples/complete-example

# Study code structure
cat README.md                              # Overview
cat include/PluginProcessor.h              # Audio parameters
cat source/PluginEditor.cpp                # WebView setup
cat ui/public/js/index.js                  # JavaScript bindings

# Build it
cmake -B build
cmake --build build

# Install for testing
./install.sh  # macOS
install.ps1   # Windows
```

**Cross-reference with docs:**

See `REFERENCE-MAP.md` for exact documentation locations for each pattern.

### Option 2: Start from Template

**Create new plugin:**

```bash
# Copy template
cp -R examples/starter-template ../MyNewPlugin
cd ../MyNewPlugin

# Customize
# 1. Edit CMakeLists.txt - change PROJECT_NAME, COMPANY_NAME
# 2. Rename namespace in source files
# 3. Add your parameters
# 4. Build UI

cmake -B build
cmake --build build
```

### Option 3: Extract Specific Patterns

**Copy just what you need:**

```javascript
// From complete-example/ui/public/js/index.js

// PATTERN: Float parameter binding
const slider = document.getElementById("gainSlider");
const sliderState = Juce.getSliderState("GAIN");
slider.oninput = () => sliderState.setNormalisedValue(slider.value);
sliderState.valueChangedEvent.addListener(() => {
    slider.value = sliderState.getNormalisedValue();
});
```

Copy this pattern into your own project.

---

## Code Annotations

All example code includes **inline comments** explaining:

### C++ Annotations

```cpp
// DOC: 02-project-setup.md - "WebView Options"
// PATTERN: Native integration enables JUCE frontend library
.withNativeIntegrationEnabled()

// DOC: 04-parameter-binding.md - "Initialization Order"
// CRITICAL: Relay MUST initialize before webView
webGainRelay{id::GAIN.getParamID()},
webView(Options{}.withOptionsFrom(webGainRelay)),

// DOC: 03-communication-patterns.md - "Resource Provider"
// PATTERN: Serve dynamic JSON data from C++
if (resourceToRetrieve == "outputLevel.json") {
    // ...return JSON
}
```

### JavaScript Annotations

```javascript
// DOC: 03-communication-patterns.md - "JavaScript → C++"
// PATTERN: Call native function with async/await
const nativeFunction = Juce.getNativeFunction("nativeFunction");
const result = await nativeFunction("arg1", 2, null);

// DOC: 04-parameter-binding.md - "Bool Parameter"
// PATTERN: Two-way checkbox binding
checkbox.oninput = () => toggleState.setValue(checkbox.checked);
toggleState.valueChangedEvent.addListener(() => {
    checkbox.checked = toggleState.getValue();
});
```

**DOC:** Reference to documentation section
**PATTERN:** Reusable code pattern
**CRITICAL:** Important requirement
**NOTE:** Additional context

---

## Building Examples

### Prerequisites

- CMake 3.22+
- C++20 compiler
- JUCE 8.0+ (auto-downloaded via CPM in complete example)

### Complete Example

```bash
cd examples/complete-example

# Configure
cmake --preset default  # or: release, Xcode

# Build
cmake --build --preset default

# Install for testing
# macOS:
cp -R build/plugin/*_artefacts/Debug/VST3/*.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -R build/plugin/*_artefacts/Debug/AU/*.component ~/Library/Audio/Plug-Ins/Components/

# Windows:
Copy-Item -Recurse build\plugin\*_artefacts\Debug\VST3\*.vst3 `
    "C:\Program Files\Common Files\VST3\"
```

### Starter Template

```bash
cd examples/starter-template
cmake -B build
cmake --build build
# Install same as above
```

---

## Development Workflow (Hot Reloading)

Both examples support hot reloading for rapid UI development.

```bash
cd examples/complete-example  # or starter-template

# Start dev server
cd ui/public
npx http-server  # Starts at http://127.0.0.1:8080

# In PluginEditor.cpp, switch line:
// webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
webView.goToURL("http://127.0.0.1:8080");  // Use dev server

# Rebuild once
cmake --build build

# Now: Edit HTML/JS → Refresh WebView (Ctrl+Shift+R) → See changes!
```

**See:** 06-development-workflow.md for full hot reloading guide.

---

## Example Features Comparison

| Feature | Complete Example | Starter Template |
|---------|-----------------|------------------|
| **Parameters** | 3 (float, bool, choice) | 1 (float) |
| **Audio Processing** | Gain + distortion types | Gain only |
| **Visualization** | Envelope follower + Plotly | None |
| **C++ GUI** | Hybrid (C++ + WebView) | WebView only |
| **Communication Patterns** | All 6 methods | Basic (2 methods) |
| **Production Ready** | Yes (zipped files) | Yes (simple) |
| **Lines of Code** | ~700 (C++ + JS) | ~200 (C++ + JS) |

---

## Common Patterns Quick Reference

### Pattern: Add Float Parameter

**1. Define ID (ParameterIDs.h):**
```cpp
const juce::ParameterID GAIN{"GAIN", 1};
```

**2. Create parameter (PluginProcessor.cpp):**
```cpp
auto param = std::make_unique<juce::AudioParameterFloat>(
    id::GAIN, "Gain",
    juce::NormalisableRange<float>{0.0f, 1.0f, 0.01f},
    1.0f);
parameters.gain = param.get();
layout.add(std::move(param));
```

**3. Add relay + attachment (PluginEditor.h):**
```cpp
juce::WebSliderRelay webGainRelay;
juce::WebSliderParameterAttachment webGainSliderAttachment;
```

**4. Initialize (PluginEditor.cpp constructor):**
```cpp
webGainRelay{id::GAIN.getParamID()},
webView(Options{}.withOptionsFrom(webGainRelay)),
webGainSliderAttachment{
    *processorRef.getState().getParameter(id::GAIN.getParamID()),
    webGainRelay, nullptr
}
```

**5. HTML (index.html):**
```html
<input type="range" id="gainSlider">
```

**6. JavaScript (index.js):**
```javascript
const slider = document.getElementById("gainSlider");
const sliderState = Juce.getSliderState("GAIN");
slider.oninput = () => sliderState.setNormalisedValue(slider.value);
sliderState.valueChangedEvent.addListener(() => {
    slider.value = sliderState.getNormalisedValue();
});
```

**See:** `complete-example/` for full implementation.

---

## Troubleshooting Examples

### Build Errors

**"WebView2 not found" (Windows):**
```bash
# Run NuGet commands from CMake output, or update JUCE
```

**"gtk/gtk.h not found" (Linux):**
```bash
sudo apt-get install libgtk-3-dev libwebkit2gtk-4.1-dev
```

### Runtime Errors

**Blank WebView:**
- Check `getResource()` returns files
- Verify JUCE library copied to `ui/public/js/juce/`
- Look at console: Right-click WebView → Inspect

**Parameter not syncing:**
- Check initialization order in constructor
- Verify parameter ID matches C++ and JavaScript
- Check DevTools console for errors

**CORS errors (dev server):**
- Verify `allowedOriginIn` parameter in `.withResourceProvider()`

**See:** 06-development-workflow.md (Debugging) for full guide.

---

## Extending Examples

### Add Audio Processing

```cpp
// In processBlock()
buffer.applyGain(parameters.gain->get());

// Add effects, filters, distortion, etc.
```

**See:** `complete-example/source/PluginProcessor.cpp:127-178`

### Add Visualization

1. Add atomic variable to processor
2. Update in `processBlock()`
3. Expose via resource provider
4. Fetch from JavaScript
5. Render with Canvas/Plotly

**See:** `complete-example/` for envelope follower example
**Docs:** 05-audio-visualization.md

### Add More Parameters

Follow "Pattern: Add Float Parameter" above, adjusting for:
- **Bool:** Use `AudioParameterBool`, `WebToggleButtonRelay`, `getToggleState()`
- **Choice:** Use `AudioParameterChoice`, `WebComboBoxRelay`, `getComboBoxState()`

**See:** `complete-example/source/PluginProcessor.cpp:204-234`

---

## Next Steps

1. **New to WebView?** → Start with `starter-template/`
2. **Learning patterns?** → Study `complete-example/` with `REFERENCE-MAP.md`
3. **Building plugin?** → Copy pattern you need from `complete-example/`
4. **Need context?** → See main docs in `../` directory

**Questions?** Check:
- `REFERENCE-MAP.md` - Maps code to documentation
- `../QUICK-REFERENCE.md` - Cheat sheet
- `../README.md` - Full documentation index
