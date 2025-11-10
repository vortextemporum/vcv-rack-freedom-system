# Communication Patterns - C++ ↔ JavaScript

## Overview

JUCE WebView provides **four mechanisms** for C++ → JavaScript communication and **two mechanisms** for JavaScript → C++ communication.

## C++ → JavaScript Communication

### Method 1: Evaluate JavaScript

Execute JavaScript code from C++ with optional result callback.

#### C++ Side

```cpp
// In PluginEditor.h
juce::TextButton runJavaScriptButton{"Run JavaScript"};

// In PluginEditor.cpp constructor
runJavaScriptButton.onClick = [this] {
    constexpr auto JAVASCRIPT_TO_RUN = R"(
        console.log("Hello from C++!");
        return 42;
    )";

    webView.evaluateJavascript(
        JAVASCRIPT_TO_RUN,
        [](juce::WebBrowserComponent::EvaluationResult result) {
            if (const auto* resultPtr = result.getResult()) {
                std::cout << "Result: " << resultPtr->toString() << std::endl;
            } else {
                std::cout << "Error: " << result.getError()->message << std::endl;
            }
        });
};
```

**Key Points:**
- Runs in browser context, has access to all JavaScript globals
- Callback is optional but useful for debugging
- Result may not always be available (platform-dependent)
- Use raw string literals `R"(...)"` for multi-line JavaScript

### Method 2: Emit JavaScript Events

Send events from C++ that JavaScript can listen for.

#### C++ Side

```cpp
// In PluginEditor.h
juce::TextButton emitEventButton{"Emit Event"};

// In PluginEditor.cpp
emitEventButton.onClick = [this] {
    static const juce::Identifier eventId{"exampleEvent"};
    const juce::var valueToEmit{42.0};

    webView.emitEventIfBrowserIsVisible(eventId, valueToEmit);
};
```

#### JavaScript Side

```javascript
window.__JUCE__.backend.addEventListener("exampleEvent", (value) => {
    console.log("Received from C++:", value);  // 42
});
```

**Key Points:**
- Event name must match exactly (case-sensitive)
- Can pass any `juce::var` type (number, string, object, array)
- `emitEventIfBrowserIsVisible()` only emits if WebView is shown
- Multiple listeners can subscribe to same event

### Method 3: User Script (Pre-Load JavaScript)

Run JavaScript before page loads.

#### C++ Side

```cpp
// In WebBrowserComponent::Options
.withUserScript(R"(
    console.log("C++ backend: This runs before page loads");
    window.MY_CUSTOM_FLAG = true;
)")
```

**Use Cases:**
- Set global flags before page initialization
- Inject polyfills or shims
- Override console methods for debugging
- Prepare environment for frontend code

**Key Points:**
- Executes **before** any HTML/JS loads
- Runs in global scope
- Useful for one-time setup
- Must be valid JavaScript (no C++ preprocessing)

### Method 4: Initialization Data

Pass compile-time or startup data to JavaScript.

#### C++ Side

```cpp
// Get compile-time values from CMake
target_compile_definitions(YourPlugin PRIVATE
    JUCE_COMPANY_NAME="${COMPANY_NAME}"
    JUCE_PRODUCT_NAME="${PRODUCT_NAME}"
    JUCE_PRODUCT_VERSION="${PROJECT_VERSION}"
)

// In WebBrowserComponent::Options
.withInitialisationData("vendor", JUCE_COMPANY_NAME)
.withInitialisationData("pluginName", JUCE_PRODUCT_NAME)
.withInitialisationData("pluginVersion", JUCE_PRODUCT_VERSION)
```

#### JavaScript Side

```javascript
const data = window.__JUCE__.initialisationData;

console.log(data.vendor);         // "YourCompany"
console.log(data.pluginName);     // "YourPlugin"
console.log(data.pluginVersion);  // "1.0.0"

// Update DOM
document.getElementById("vendor").innerText = data.vendor;
```

**Key Points:**
- Data available immediately on page load
- Best for **static** configuration (not runtime data)
- Keys are strings, values can be any `juce::var` type
- Accessed via `window.__JUCE__.initialisationData`

---

## JavaScript → C++ Communication

### Method 1: Native Functions

Call C++ functions from JavaScript with async/await support.

#### C++ Side

```cpp
// In PluginEditor.h
void nativeFunction(const juce::Array<juce::var>& args,
                   juce::WebBrowserComponent::NativeFunctionCompletion completion);

// In WebBrowserComponent::Options
.withNativeFunction(
    juce::Identifier{"nativeFunction"},
    [this](const juce::Array<juce::var>& args,
           juce::WebBrowserComponent::NativeFunctionCompletion completion) {
        nativeFunction(args, std::move(completion));
    })

// In PluginEditor.cpp
void AudioPluginAudioProcessorEditor::nativeFunction(
    const juce::Array<juce::var>& args,
    juce::WebBrowserComponent::NativeFunctionCompletion completion)
{
    // Process arguments
    juce::String concatenated;
    for (const auto& arg : args) {
        concatenated += arg.toString() + " ";
    }

    // Do work on GUI thread (this is safe)
    labelUpdatedFromCpp.setText("Called with: " + concatenated,
                                juce::dontSendNotification);

    // Call completion to resolve Promise in JavaScript
    completion("Success! All OK");
}
```

#### JavaScript Side

```javascript
const nativeFunction = Juce.getNativeFunction("nativeFunction");

// Call with await
const result = await nativeFunction("one", 2, null);
console.log(result);  // "Success! All OK"

// Or with .then()
nativeFunction("arg1", "arg2").then(result => {
    console.log(result);
});
```

**Key Points:**
- Returns a Promise in JavaScript
- Arguments passed as `juce::Array<juce::var>`
- Completion callback resolves the Promise
- Runs on GUI thread (safe to update UI)
- Function name must match exactly

### Method 2: JavaScript Events

Emit events from JavaScript that C++ can listen for.

#### C++ Side

```cpp
// In WebBrowserComponent::Options
.withEventListener(
    "exampleJavaScriptEvent",
    [this](juce::var objectFromFrontend) {
        labelUpdatedFromJS.setText(
            "Event value: " + objectFromFrontend.getProperty("count", 0).toString(),
            juce::dontSendNotification);
    })
```

#### JavaScript Side

```javascript
let count = 0;
emitEventButton.addEventListener("click", () => {
    count++;
    window.__JUCE__.backend.emitEvent("exampleJavaScriptEvent", {
        count: count,
        timestamp: Date.now()
    });
});
```

**Key Points:**
- Event name must match exactly
- Can pass JavaScript objects (converted to `juce::var`)
- Callback runs on GUI thread
- Multiple C++ handlers can subscribe (chain `.withEventListener()` calls)

---

## Resource Provider Pattern

The **Resource Provider** serves files and dynamic data from C++ to the WebView.

### Basic Setup

```cpp
// In WebBrowserComponent::Options
.withResourceProvider(
    [this](const auto& url) { return getResource(url); })

// In PluginEditor
auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    const auto resourceToRetrieve =
        url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

    // Handle static files (HTML, CSS, JS)
    // See Distribution docs for implementation

    return std::nullopt;  // Not found
}
```

### Serving Dynamic Data

```cpp
auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    // ... parse resourceToRetrieve ...

    // Serve dynamic JSON data
    if (resourceToRetrieve == "outputLevel.json") {
        juce::DynamicObject::Ptr levelData{new juce::DynamicObject{}};
        levelData->setProperty("left", processorRef.outputLevelLeft.load());
        levelData->setProperty("right", processorRef.outputLevelRight.load());

        const auto jsonString = juce::JSON::toString(levelData.get());
        juce::MemoryInputStream stream{jsonString.getCharPointer(),
                                       jsonString.getNumBytesAsUTF8(), false};

        return juce::WebBrowserComponent::Resource{
            streamToVector(stream),
            juce::String{"application/json"}
        };
    }

    // Handle static files...
}
```

### JavaScript Fetching from Resource Provider

```javascript
// Fetch dynamic data from C++ backend
fetch(Juce.getBackendResourceAddress("outputLevel.json"))
    .then(response => response.json())
    .then(data => {
        console.log("Output levels:", data.left, data.right);
    });
```

**Key Points:**
- Resource provider acts as a local web server
- Can serve **static files** (HTML, CSS, JS) or **dynamic data** (JSON)
- `Juce.getBackendResourceAddress()` creates proper URL for fetch
- Useful for real-time audio data visualization

---

## Complete Communication Example

### C++ Setup (PluginEditor.cpp)

```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView(juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withNativeIntegrationEnabled()

          // Pass startup data
          .withInitialisationData("pluginName", JUCE_PRODUCT_NAME)

          // Pre-load script
          .withUserScript("console.log('Backend initialized');")

          // Expose C++ function to JS
          .withNativeFunction(
              juce::Identifier{"savePreset"},
              [this](const juce::Array<juce::var>& args, auto completion) {
                  // Save preset logic...
                  completion("Preset saved!");
              })

          // Listen for JS events
          .withEventListener(
              "presetChanged",
              [this](juce::var data) {
                  auto presetName = data.getProperty("name", "").toString();
                  // Load preset...
              })

          // Serve resources
          .withResourceProvider(
              [this](const auto& url) { return getResource(url); }))
{
    addAndMakeVisible(webView);
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());

    // Emit events to JS periodically
    startTimer(60);  // 60ms
}

void AudioPluginAudioProcessorEditor::timerCallback() {
    // Send real-time data to frontend
    webView.emitEventIfBrowserIsVisible("audioUpdate", juce::var{});
}
```

### JavaScript Setup (index.js)

```javascript
import * as Juce from "./juce/index.js";

// Access initialization data
const pluginName = window.__JUCE__.initialisationData.pluginName;
console.log("Plugin:", pluginName);

// Get C++ function
const savePreset = Juce.getNativeFunction("savePreset");

// Listen for C++ events
window.__JUCE__.backend.addEventListener("audioUpdate", async () => {
    // Fetch real-time data from C++
    const response = await fetch(Juce.getBackendResourceAddress("audioData.json"));
    const data = await response.json();
    updateVisualization(data);
});

// Call C++ function
document.getElementById("saveBtn").addEventListener("click", async () => {
    const result = await savePreset();
    console.log(result);  // "Preset saved!"
});

// Emit event to C++
document.getElementById("presetSelect").addEventListener("change", (e) => {
    window.__JUCE__.backend.emitEvent("presetChanged", {
        name: e.target.value
    });
});
```

---

## Best Practices

### Thread Safety

- **All callbacks run on GUI thread** - Safe to update JUCE components
- **Atomic variables** for shared data between audio and GUI threads:
  ```cpp
  std::atomic<float> outputLevelLeft{0.0f};
  ```

### Performance

- **Don't call `evaluateJavascript()` from audio thread** - Only GUI thread
- **Batch events** - Don't emit 44,100 events per second
- **Use timer** for periodic updates (30-60ms interval)

### Error Handling

- **Always check `result.getResult()` vs `result.getError()`** in evaluate callbacks
- **Validate arguments** in native functions before processing
- **Log resource provider requests** during development:
  ```cpp
  std::cout << "Resource requested: " << url << std::endl;
  ```

### Security

- **Validate all data from JavaScript** - Don't trust frontend input
- **Don't expose file system paths** - Only serve via resource provider
- **Sanitize user input** before passing to native functions

---

## Debugging Tips

### Enable Browser DevTools

**Right-click WebView → Inspect** opens Chrome DevTools (Windows/Linux) or Safari DevTools (macOS).

### Log All Communication

```cpp
// C++ side
webView.evaluateJavascript("console.log('C++ says:', " + value + ");");
```

```javascript
// JS side
console.log("JS received:", value);
```

### Check JUCE Object

In browser console:
```javascript
console.log(window.__JUCE__);           // Should exist
console.log(window.__JUCE__.backend);   // Event system
console.log(window.__JUCE__.initialisationData);  // Startup data
```

### Test Resource Provider

```cpp
auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    std::cout << "Resource requested: " << url << std::endl;  // DEBUG
    // ... rest of implementation
}
```

## Next Steps

Continue to **04-parameter-binding.md** to learn how to control audio parameters from WebView UI.
