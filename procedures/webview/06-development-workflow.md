# Development Workflow - Hot Reloading and Debugging

## Overview

During development, you want to **iterate quickly** on your UI without recompiling C++ for every HTML/CSS/JavaScript change. This guide covers:

1. Local development server setup
2. Hot reloading workflow
3. Debugging techniques
4. Common pitfalls and solutions

---

## Local Development Server Setup

### Step 1: Install http-server (Node.js)

```bash
# Install globally
npm install -g http-server

# Or use npx (no installation needed)
npx http-server
```

### Step 2: Start Server in UI Directory

```bash
cd plugin/ui/public
http-server

# Server starts at http://127.0.0.1:8080
```

**Alternative servers:**
- **Python:** `python -m http.server 8080`
- **PHP:** `php -S 127.0.0.1:8080`
- **VS Code Extension:** "Live Server"

### Step 3: Point WebView to Dev Server

**PluginEditor.cpp**

```cpp
namespace {
    constexpr auto LOCAL_DEV_SERVER_ADDRESS = "http://127.0.0.1:8080";
}

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView(juce::WebBrowserComponent::Options{}
          // ... other options ...

          // CRITICAL: Allow dev server to fetch from C++ backend
          .withResourceProvider(
              [this](const auto& url) { return getResource(url); },
              juce::URL{LOCAL_DEV_SERVER_ADDRESS}.getOrigin()))  // Allow origin
{
    addAndMakeVisible(webView);

    // DEVELOPMENT: Go to local dev server
    webView.goToURL(LOCAL_DEV_SERVER_ADDRESS);

    // PRODUCTION: Uncomment this instead
    // webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    setResizable(true, true);
    setSize(800, 600);
}
```

**Key Points:**
- `.withResourceProvider()` second parameter (`allowedOriginIn`) is **critical**
- Without it, dev server can't fetch resources from C++ (CORS error)
- **Comment out dev server URL for production builds**

---

## Hot Reloading Workflow

### Normal Development Cycle (No Recompile)

1. **Start dev server:** `cd plugin/ui/public && http-server`
2. **Build plugin once:** `cmake --build build`
3. **Launch plugin** (standalone or in DAW)
4. **Edit HTML/CSS/JS** in your editor
5. **Refresh WebView:** Ctrl+Shift+R (Windows/Linux) or Cmd+Shift+R (macOS)
6. **See changes immediately** - no C++ recompile needed!

### When Recompile IS Required

- Changing C++ code (PluginProcessor, PluginEditor)
- Adding/removing parameters
- Changing relay configuration
- Modifying resource provider logic

### Hard Refresh (Bypass Cache)

**Why needed:** Browser caches JavaScript files aggressively.

**How to do it:**
- **Windows/Linux:** Ctrl + Shift + R
- **macOS:** Cmd + Shift + R
- **Programmatically:** Right-click WebView → Inspect → Network tab → "Disable cache"

**When to use:**
- JavaScript changes don't appear
- Old version still loading
- After modifying JUCE frontend library

---

## Debugging Techniques

### 1. Browser Developer Tools

**Enable DevTools:**
1. Right-click WebView
2. Select "Inspect" or "Inspect Element"
3. DevTools window opens (Chrome/Edge DevTools on Windows, Safari DevTools on macOS)

**DevTools Features:**
- **Console:** View `console.log()`, errors, warnings
- **Elements:** Inspect DOM, edit HTML/CSS live
- **Network:** See all HTTP requests (including resource provider)
- **Sources:** Set breakpoints in JavaScript, step through code
- **Performance:** Profile JavaScript execution

### 2. Logging C++ → JavaScript Communication

**Log all resource requests:**

```cpp
auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    std::cout << "[ResourceProvider] Requested: " << url << std::endl;

    // ... handle resource ...
}
```

**Log all event emissions:**

```cpp
void AudioPluginAudioProcessorEditor::timerCallback()
{
    std::cout << "[Timer] Emitting outputLevel event" << std::endl;
    webView.emitEventIfBrowserIsVisible("outputLevel", juce::var{});
}
```

**Log JavaScript evaluations:**

```cpp
webView.evaluateJavascript("console.log('C++ called JavaScript');",
    [](juce::WebBrowserComponent::EvaluationResult result) {
        if (result.getResult()) {
            std::cout << "[Eval] Success: " << result.getResult()->toString() << std::endl;
        } else {
            std::cerr << "[Eval] Error: " << result.getError()->message << std::endl;
        }
    });
```

### 3. Logging JavaScript → C++ Communication

**Log all native function calls:**

```cpp
void AudioPluginAudioProcessorEditor::nativeFunction(
    const juce::Array<juce::var>& args,
    juce::WebBrowserComponent::NativeFunctionCompletion completion)
{
    std::cout << "[NativeFunction] Called with " << args.size() << " args:" << std::endl;
    for (int i = 0; i < args.size(); ++i) {
        std::cout << "  arg[" << i << "]: " << args[i].toString() << std::endl;
    }

    // ... process ...

    completion("OK");
}
```

**Log all JavaScript events:**

```cpp
.withEventListener("myEvent", [](juce::var data) {
    std::cout << "[EventListener] Received myEvent:" << std::endl;
    std::cout << juce::JSON::toString(data, true) << std::endl;
})
```

### 4. JavaScript Console Logging

**Log JUCE object state:**

```javascript
console.log("JUCE object:", window.__JUCE__);
console.log("Backend:", window.__JUCE__.backend);
console.log("Init data:", window.__JUCE__.initialisationData);
```

**Log parameter state changes:**

```javascript
const sliderState = Juce.getSliderState("GAIN");

sliderState.valueChangedEvent.addListener(() => {
    console.log("Slider value changed:", sliderState.getNormalisedValue());
});
```

**Log resource fetches:**

```javascript
fetch(Juce.getBackendResourceAddress("data.json"))
    .then(response => {
        console.log("Fetched data.json, status:", response.status);
        return response.json();
    })
    .then(data => {
        console.log("Data:", data);
    })
    .catch(error => {
        console.error("Fetch failed:", error);
    });
```

### 5. Network Tab Debugging

**In DevTools → Network tab:**

1. **See all requests** to dev server and resource provider
2. **Identify failed requests** (red, 404/500 status)
3. **Check response content** - click request to see payload
4. **Measure timing** - how long each fetch takes

**Example:**
```
http://127.0.0.1:8080/index.html          200 OK  (from dev server)
http://127.0.0.1:8080/js/index.js         200 OK  (from dev server)
juce://backend/outputLevel.json           200 OK  (from C++ resource provider)
```

---

## Common Development Issues

### Issue: CORS Error on Local Dev Server

**Error:** `Access to fetch at 'juce://backend/data.json' from origin 'http://127.0.0.1:8080' has been blocked by CORS policy`

**Cause:** Forgot `allowedOriginIn` parameter in `.withResourceProvider()`

**Fix:**
```cpp
.withResourceProvider(
    [this](const auto& url) { return getResource(url); },
    juce::URL{LOCAL_DEV_SERVER_ADDRESS}.getOrigin())  // ADD THIS
```

### Issue: Changes Don't Appear After Editing JS

**Cause:** Browser cache

**Fix:** Hard refresh (Ctrl+Shift+R / Cmd+Shift+R)

**Prevention:** Disable cache in DevTools → Network tab → "Disable cache" checkbox

### Issue: "Cannot read property '__JUCE__' of undefined"

**Cause:** `withNativeIntegrationEnabled()` not called, or JUCE library not imported

**Fix:**
1. Ensure `.withNativeIntegrationEnabled()` in WebView options
2. Check `import * as Juce from "./juce/index.js"` in JavaScript
3. Verify JUCE library copied to `ui/public/js/juce/`

### Issue: Dev Server Shows 404 for JUCE Library

**Error:** `GET http://127.0.0.1:8080/js/juce/index.js 404 (Not Found)`

**Cause:** JUCE frontend library not copied to UI directory

**Fix:** Add to `plugin/CMakeLists.txt`:
```cmake
file(COPY
    "${JUCE_MODULES_DIR}/juce_gui_extra/native/javascript/"
    DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/ui/public/js/juce/"
)
```

**Note:** The correct path is `javascript/` not `javaScriptInterface/`.

Then regenerate: `cmake --preset default`

### Issue: Plugin Crashes When WebView Opens

**Cause (Windows):** WebView2 runtime not installed, or permissions issue

**Fix:**
1. Install WebView2 runtime: Run NuGet commands from CMake output
2. Ensure `.withUserDataFolder()` set to temp directory:
   ```cpp
   .withWinWebView2Options(
       juce::WebBrowserComponent::Options::WinWebView2{}
           .withUserDataFolder(juce::File::getSpecialLocation(
               juce::File::SpecialLocationType::tempDirectory)))
   ```

### Issue: Blank White Screen in WebView

**Cause:** Resource provider not serving files, or dev server not running

**Debug:**
1. Check console: `std::cout << "Resource requested: " << url << std::endl;`
2. Verify dev server running: Open `http://127.0.0.1:8080` in regular browser
3. Check DevTools Network tab for failed requests

---

## Production vs Development Builds

### Toggle Dev Server Programmatically

**Option 1: Preprocessor Define**

```cmake
# In CMakeLists.txt
target_compile_definitions(YourPlugin PRIVATE
    $<$<CONFIG:Debug>:USE_DEV_SERVER=1>
)
```

```cpp
// In PluginEditor.cpp
#ifdef USE_DEV_SERVER
    constexpr auto SERVER_URL = "http://127.0.0.1:8080";
#else
    constexpr auto SERVER_URL = nullptr;  // Use resource provider
#endif

// In constructor
if (SERVER_URL) {
    webView.goToURL(SERVER_URL);
} else {
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
}
```

**Option 2: Runtime Check**

```cpp
auto AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(...)
{
    // ...

    // Check if dev server is running
    const bool useDevServer = juce::URL("http://127.0.0.1:8080")
        .withPOSTData("")
        .readEntireTextStream(false)
        .isNotEmpty();

    if (useDevServer) {
        DBG("Using dev server");
        webView.goToURL("http://127.0.0.1:8080");
    } else {
        DBG("Using resource provider");
        webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
    }
}
```

### Recommended Workflow

**During Development:**
```cpp
webView.goToURL("http://127.0.0.1:8080");  // Dev server
```

**Before Release:**
```cpp
webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());  // Embedded files
```

**Automate with CMake:**
```cmake
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(YourPlugin PRIVATE DEV_BUILD=1)
endif()
```

---

## Testing in DAW

### Quick Install Script (macOS)

```bash
#!/bin/bash
# install_plugin.sh

BUILD_DIR="build/plugin/YourPlugin_artefacts/Debug"

# VST3
cp -R "$BUILD_DIR/VST3/YourPlugin.vst3" ~/Library/Audio/Plug-Ins/VST3/

# AU
cp -R "$BUILD_DIR/AU/YourPlugin.component" ~/Library/Audio/Plug-Ins/Components/

echo "Plugin installed! Restart DAW to load."
```

### Quick Install Script (Windows PowerShell)

```powershell
# install_plugin.ps1
$BuildDir = "build\plugin\YourPlugin_artefacts\Debug"

Copy-Item -Recurse -Force "$BuildDir\VST3\YourPlugin.vst3" `
    "C:\Program Files\Common Files\VST3\"

Write-Host "Plugin installed! Restart DAW to load."
```

### DAW Workflow

1. **Build plugin:** `cmake --build build`
2. **Run install script:** `./install_plugin.sh`
3. **Start dev server:** `cd plugin/ui/public && http-server`
4. **Open DAW** (e.g., Ableton, Reaper, Logic)
5. **Load plugin** on a track
6. **Edit UI** in code editor
7. **Refresh WebView** in plugin (Ctrl+Shift+R)
8. **See changes** without closing DAW!

---

## Debugging in Specific DAWs

### Ableton Live

- **Enable plugin scanning:** Preferences → Plug-ins → Rescan
- **Open WebView DevTools:** Right-click plugin UI → Inspect
- **Console location:** Same window as plugin

### Reaper

- **Fast plugin loading:** Preferences → Plug-ins → VST → "Clear cache/re-scan"
- **DevTools:** Right-click plugin → Inspect (opens separate window)
- **Console errors:** Check REAPER console (Ctrl+Alt+C on Windows)

### Logic Pro

- **AU validation:** Logic validates plugins strictly - may fail if WebView2 missing
- **DevTools:** Right-click plugin → Inspect Element
- **Console:** Safari Web Inspector opens in separate window

---

## Best Practices

### 1. Use Git Ignore for Node Modules

```bash
# .gitignore
node_modules/
plugin/ui/public/js/juce/  # Auto-copied from JUCE
```

### 2. Separate Dev and Prod Code

```javascript
// index.js
const isDev = window.location.protocol === "http:";

if (isDev) {
    console.log("Development mode - verbose logging enabled");
}
```

### 3. Version Your Dev Server

```bash
# package.json
{
  "scripts": {
    "dev": "http-server plugin/ui/public -p 8080"
  }
}

# Run with: npm run dev
```

### 4. Use Source Maps for JavaScript

If using a build tool (webpack, vite, etc.):

```javascript
// vite.config.js
export default {
    build: {
        sourcemap: true  // Enable source maps for debugging
    }
}
```

### 5. Log WebView Version

```cpp
// In constructor
webView.evaluateJavascript(R"(
    console.log("User Agent:", navigator.userAgent);
    console.log("WebView Backend:", window.__JUCE__ ? "JUCE" : "Unknown");
)");
```

---

## Next Steps

Continue to **07-distribution.md** to learn how to package your plugin for release.
