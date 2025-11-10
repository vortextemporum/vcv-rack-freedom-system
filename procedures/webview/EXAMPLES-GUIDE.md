# Quick Guide to Examples

## Structure

```
docs/
├── examples/
│   ├── README.md (comprehensive guide)
│   ├── REFERENCE-MAP.md (code → docs mapping)
│   ├── complete-example/ → ../../plugin (symlink to working code)
│   └── starter-template/ (minimal starting point)
├── 01-overview.md through 07-distribution.md (documentation)
├── QUICK-REFERENCE.md (cheat sheet)
└── EXAMPLES-GUIDE.md (this file)
```

## What's What

### Complete Example (`examples/complete-example/`)

**Symlink to:** `../../plugin/` (the actual working code in this repository)

**Features:**
- All 3 parameter types (float, bool, choice)
- Audio processing (gain + 2 distortion types)
- Real-time visualization (envelope follower + Plotly)
- All 6 communication patterns
- Hybrid UI (C++ + WebView side-by-side)
- ~700 lines of code

**Use for:**
- Reference implementation of all patterns
- Studying complete working code
- Extracting specific patterns

### Starter Template (`examples/starter-template/`)

**Standalone template** ready to copy and customize.

**Features:**
- Single float parameter (gain)
- Basic WebView setup
- Production-ready build
- ~200 lines of code

**Use for:**
- Starting a new plugin from scratch
- Learning minimal requirements
- Clean slate to build on

## Quick Start Paths

### Path 1: I want to understand WebView plugins

```bash
# 1. Read overview
cat docs/01-overview.md

# 2. Study complete example
cd docs/examples/complete-example
cat README.md
cat source/PluginEditor.cpp  # See WebView setup

# 3. Cross-reference with docs
# Use docs/examples/REFERENCE-MAP.md to find doc sections
```

### Path 2: I want to build my first WebView plugin

```bash
# 1. Copy starter template
cp -R docs/examples/starter-template ../MyPlugin
cd ../MyPlugin

# 2. Customize
# - Edit CMakeLists.txt (project name, company)
# - Edit ui/public/index.html (your UI)
# - Add parameters (see docs/04-parameter-binding.md)

# 3. Build
cmake -B build
cmake --build build

# 4. Install and test
cp -R build/plugin/*_artefacts/Debug/VST3/*.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

### Path 3: I need a specific pattern

```bash
# 1. Find pattern in REFERENCE-MAP
grep "Float Parameter" docs/examples/REFERENCE-MAP.md

# 2. Look at code location
# Example: PluginEditor.cpp:104

# 3. Copy pattern to your project

# 4. Read docs for details
# Example: docs/04-parameter-binding.md
```

## Code Annotations Explained

Examples use inline comments with these tags:

- **DOC:** Links to documentation section
  ```cpp
  // DOC: 02-project-setup.md # WebView Options
  .withNativeIntegrationEnabled()
  ```

- **PATTERN:** Reusable code pattern
  ```cpp
  // PATTERN: Convert InputStream to byte vector
  std::vector<std::byte> streamToVector(juce::InputStream& stream)
  ```

- **CRITICAL:** Must-follow requirement
  ```cpp
  // CRITICAL: Relay must initialize before webView
  webGainRelay{id::GAIN.getParamID()},
  webView(Options{}.withOptionsFrom(webGainRelay))
  ```

- **NOTE:** Additional context
  ```javascript
  // NOTE: Step size calculated from parameter properties
  slider.step = 1 / sliderState.properties.numSteps;
  ```

## Common Tasks

### Add Float Parameter

**Files to modify:**
1. `include/ParameterIDs.h` - Add ID
2. `source/PluginProcessor.cpp` - Create parameter
3. `include/PluginEditor.h` - Add relay + attachment
4. `source/PluginEditor.cpp` - Initialize relay + attachment
5. `ui/public/index.html` - Add `<input type="range">`
6. `ui/public/js/index.js` - Bind JavaScript

**Code to copy:** `complete-example/` lines shown in `REFERENCE-MAP.md # Float Parameter`

**Docs:** `04-parameter-binding.md # Float Parameter`

### Add Audio Visualization

**Files to modify:**
1. `include/PluginProcessor.h` - Add atomic variable
2. `source/PluginProcessor.cpp` - Measure audio data
3. `include/PluginEditor.h` - Inherit from Timer
4. `source/PluginEditor.cpp` - Timer callback, serve JSON
5. `ui/public/index.html` - Add chart container
6. `ui/public/js/index.js` - Fetch and visualize

**Code to copy:** `complete-example/` envelope follower pattern

**Docs:** `05-audio-visualization.md`

### Enable Hot Reloading

**Files to modify:**
1. `source/PluginEditor.cpp` - Change goToURL line

**Command to run:**
```bash
cd ui/public
npx http-server
```

**Rebuild once, then:** Edit HTML/JS → Refresh WebView → See changes!

**Docs:** `06-development-workflow.md`

## Building Examples

### Complete Example

```bash
cd docs/examples/complete-example

# Configure (choose one preset)
cmake --preset default   # Ninja, Debug
cmake --preset release   # Ninja, Release
cmake --preset Xcode     # Xcode project

# Build
cmake --build --preset default
```

### Starter Template

```bash
cd docs/examples/starter-template

cmake -B build
cmake --build build
```

## Installation Paths

### macOS

```bash
# VST3
~/Library/Audio/Plug-Ins/VST3/YourPlugin.vst3

# AU (Audio Unit)
~/Library/Audio/Plug-Ins/Components/YourPlugin.component
```

### Windows

```
C:\Program Files\Common Files\VST3\YourPlugin.vst3
```

### Linux

```bash
~/.vst3/YourPlugin.vst3
```

## Troubleshooting

### Build Fails

| Error | Solution |
|-------|----------|
| "WebView2 not found" | Run NuGet commands from CMake output (Windows) |
| "gtk/gtk.h not found" | `sudo apt install libgtk-3-dev libwebkit2gtk-4.1-dev` (Linux) |
| "JUCE not found" | Check CPM.cmake exists, or add JUCE as submodule |

### Runtime Issues

| Issue | Solution |
|-------|----------|
| Blank WebView | Check getResource() returns files, see DevTools console |
| Parameter not syncing | Check initialization order, verify parameter ID matches |
| CORS error (dev server) | Add `allowedOriginIn` to `.withResourceProvider()` |

**Full debugging guide:** `06-development-workflow.md # Debugging Techniques`

## Documentation Map

| I want to... | Read this doc | See code in |
|--------------|---------------|-------------|
| Understand WebView basics | 01-overview.md | - |
| Set up project | 02-project-setup.md | starter-template/CMakeLists.txt |
| Send data C++↔JS | 03-communication-patterns.md | complete-example/PluginEditor.cpp |
| Control parameters | 04-parameter-binding.md | complete-example/* (all files) |
| Visualize audio | 05-audio-visualization.md | complete-example/PluginProcessor.cpp |
| Enable hot reload | 06-development-workflow.md | complete-example/PluginEditor.cpp:166 |
| Package for release | 07-distribution.md | complete-example/CMakeLists.txt:60-90 |

## Next Steps

1. **Choose your path** (understand, build, or extract pattern)
2. **Follow quick start** from above
3. **Use REFERENCE-MAP.md** to find specific code
4. **Read relevant docs** for details
5. **Build and test** your plugin

**Need help?** Check:
- `examples/README.md` - Comprehensive guide
- `QUICK-REFERENCE.md` - Code cheat sheet
- `README.md` - Main documentation index
