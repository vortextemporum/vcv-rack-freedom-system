# Audio Visualization - Display Real-Time Audio Data in WebView

## Overview

This guide shows how to:
1. Measure audio data in C++ (envelope following, FFT, etc.)
2. Expose data via resource provider as JSON
3. Fetch data periodically from JavaScript
4. Visualize using web libraries (Plotly.js, D3.js, Canvas, etc.)

## Pattern: Timed Events + Resource Provider

```
Audio Thread (C++)
  ↓ measure/process
Audio Data (atomic variables)
  ↓ read periodically
GUI Timer (60ms)
  ↓ emit event
JavaScript EventListener
  ↓ fetch resource
Resource Provider (C++)
  ↓ return JSON
JavaScript Visualization Library
  ↓ render
HTML Canvas/SVG
```

## Example: Output Level Meter

### Step 1: Measure Audio Level (Envelope Follower)

**PluginProcessor.h**

```cpp
#include <juce_dsp/juce_dsp.h>

class AudioPluginAudioProcessor : public juce::AudioProcessor
{
private:
    // Envelope follower
    juce::dsp::BallisticsFilter<float> envelopeFollower;
    juce::AudioBuffer<float> envelopeFollowerOutputBuffer;

public:
    // Exposed to editor (atomic for thread safety)
    std::atomic<float> outputLevelLeft{-100.0f};  // dB
    std::atomic<float> outputLevelRight{-100.0f}; // dB
};
```

**PluginProcessor.cpp**

```cpp
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    using namespace juce;

    // Initialize envelope follower
    dsp::ProcessSpec spec{
        .sampleRate = sampleRate,
        .maximumBlockSize = static_cast<uint32>(samplesPerBlock),
        .numChannels = static_cast<uint32>(getTotalNumOutputChannels())
    };

    envelopeFollower.prepare(spec);
    envelopeFollower.setAttackTime(200.0f);   // 200ms
    envelopeFollower.setReleaseTime(200.0f);  // 200ms
    envelopeFollower.setLevelCalculationType(
        dsp::BallisticsFilter<float>::LevelCalculationType::peak);

    // Resize output buffer
    envelopeFollowerOutputBuffer.setSize(
        getTotalNumOutputChannels(),
        samplesPerBlock);
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    using namespace juce;

    // Create audio blocks
    const auto inBlock = dsp::AudioBlock<float>{buffer}
        .getSubsetChannelBlock(0, static_cast<size_t>(getTotalNumOutputChannels()));

    auto outBlock = dsp::AudioBlock<float>{envelopeFollowerOutputBuffer};

    // Process envelope follower
    dsp::ProcessContextNonReplacing<float> context{inBlock, outBlock};
    envelopeFollower.process(context);

    // Read last sample from first channel (left) and convert to dB
    const auto lastSampleIndex = static_cast<int>(outBlock.getNumSamples()) - 1;
    const float leftPeak = outBlock.getSample(0, lastSampleIndex);
    const float rightPeak = getTotalNumOutputChannels() > 1
        ? outBlock.getSample(1, lastSampleIndex)
        : leftPeak;

    outputLevelLeft.store(Decibels::gainToDecibels(leftPeak));
    outputLevelRight.store(Decibels::gainToDecibels(rightPeak));
}
```

**Link juce_dsp in CMakeLists.txt:**

```cmake
target_link_libraries(YourPlugin PRIVATE
    juce::juce_dsp  # Add this
    # ... other libraries
)
```

### Step 2: Emit Periodic Events from Editor

**PluginEditor.h**

```cpp
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer  // Inherit Timer
{
private:
    void timerCallback() override;
};
```

**PluginEditor.cpp**

```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      webView(/* ... options ... */)
{
    // ... setup ...

    startTimer(60);  // 60ms = ~16 FPS
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    // Emit event to JavaScript - no data needed, JS will fetch
    webView.emitEventIfBrowserIsVisible("outputLevel", juce::var{});
}
```

### Step 3: Serve Audio Data via Resource Provider

**PluginEditor.cpp**

```cpp
auto AudioPluginAudioProcessorEditor::getResource(const juce::String& url) const
    -> std::optional<Resource>
{
    const auto resourceToRetrieve =
        url == "/" ? "index.html" : url.fromFirstOccurrenceOf("/", false, false);

    // Serve output level data as JSON
    if (resourceToRetrieve == "outputLevel.json") {
        juce::DynamicObject::Ptr levelData{new juce::DynamicObject{}};
        levelData->setProperty("left", processorRef.outputLevelLeft.load());
        levelData->setProperty("right", processorRef.outputLevelRight.load());

        const auto jsonString = juce::JSON::toString(levelData.get());
        juce::MemoryInputStream stream{
            jsonString.getCharPointer(),
            jsonString.getNumBytesAsUTF8(),
            false};

        return juce::WebBrowserComponent::Resource{
            streamToVector(stream),
            juce::String{"application/json"}
        };
    }

    // ... handle other resources (HTML, JS, etc.) ...

    return std::nullopt;
}
```

### Step 4: Import Visualization Library (Plotly.js)

**index.html**

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <!-- Import Plotly.js from CDN -->
    <script src="https://cdn.plot.ly/plotly-2.33.0.min.js"></script>
    <script type="module" src="js/index.js"></script>
    <title>Audio Visualization</title>
</head>
<body>
    <main>
        <h1>Output Level Meter</h1>
        <!-- Plotly will render into this div -->
        <div id="outputLevelPlot"></div>
    </main>
</body>
</html>
```

### Step 5: Create Visualization in JavaScript

**index.js**

```javascript
import * as Juce from "./juce/index.js";

document.addEventListener("DOMContentLoaded", () => {
    // Create initial plot
    const base = -60;  // Minimum dB level
    Plotly.newPlot("outputLevelPlot", {
        data: [{
            x: ["Left", "Right"],
            y: [0, 0],
            base: [base, base],
            type: "bar",
            marker: { color: ["#4CAF50", "#2196F3"] }
        }],
        layout: {
            width: 300,
            height: 400,
            yaxis: {
                range: [base, 0],  // -60 dB to 0 dB
                title: "Level (dB)"
            },
            xaxis: { title: "Channel" }
        }
    });

    // Listen for output level updates from C++
    window.__JUCE__.backend.addEventListener("outputLevel", () => {
        // Fetch level data from C++ backend
        fetch(Juce.getBackendResourceAddress("outputLevel.json"))
            .then(response => response.json())
            .then(levelData => {
                // Animate plot with new data
                Plotly.animate(
                    "outputLevelPlot",
                    {
                        data: [{
                            y: [levelData.left - base, levelData.right - base]
                        }],
                        traces: [0],
                        layout: {}
                    },
                    {
                        transition: { duration: 20, easing: "cubic-in-out" },
                        frame: { duration: 20 }
                    }
                );
            })
            .catch(error => console.error("Failed to fetch level data:", error));
    });
});
```

**How it works:**
1. C++ timer fires every 60ms → emits `"outputLevel"` event
2. JavaScript receives event → fetches `outputLevel.json` from C++
3. Resource provider returns current levels as JSON
4. JavaScript parses JSON → animates Plotly chart

---

## Alternative: Canvas-Based Visualization

For more control and better performance, use HTML Canvas directly.

### HTML

```html
<canvas id="waveformCanvas" width="800" height="200"></canvas>
```

### JavaScript

```javascript
const canvas = document.getElementById("waveformCanvas");
const ctx = canvas.getContext("2d");

window.__JUCE__.backend.addEventListener("outputLevel", async () => {
    const response = await fetch(Juce.getBackendResourceAddress("outputLevel.json"));
    const data = await response.json();

    // Clear canvas
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Draw left channel bar
    const leftHeight = Math.abs((data.left / -60) * canvas.height);
    ctx.fillStyle = "#4CAF50";
    ctx.fillRect(50, canvas.height - leftHeight, 100, leftHeight);

    // Draw right channel bar
    const rightHeight = Math.abs((data.right / -60) * canvas.height);
    ctx.fillStyle = "#2196F3";
    ctx.fillRect(200, canvas.height - rightHeight, 100, rightHeight);

    // Draw labels
    ctx.fillStyle = "#000";
    ctx.font = "14px sans-serif";
    ctx.fillText(`L: ${data.left.toFixed(1)} dB`, 50, 20);
    ctx.fillText(`R: ${data.right.toFixed(1)} dB`, 200, 20);
});
```

---

## Advanced: FFT Spectrum Analyzer

### C++ Side: Compute FFT

**PluginProcessor.h**

```cpp
#include <juce_dsp/juce_dsp.h>

class AudioPluginAudioProcessor : public juce::AudioProcessor
{
private:
    static constexpr int fftOrder = 11;  // 2^11 = 2048
    static constexpr int fftSize = 1 << fftOrder;

    juce::dsp::FFT forwardFFT{fftOrder};
    juce::dsp::WindowingFunction<float> window{
        fftSize, juce::dsp::WindowingFunction<float>::hann};

    std::array<float, fftSize * 2> fftData{};  // Real + imaginary
    std::array<float, fftSize / 2> magnitudes{};  // FFT bins

    juce::CriticalSection fftLock;

public:
    void getFFTMagnitudes(std::vector<float>& output) {
        juce::ScopedLock lock(fftLock);
        output.assign(magnitudes.begin(), magnitudes.end());
    }
};
```

**PluginProcessor.cpp**

```cpp
void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    // Copy left channel to FFT input
    const auto* channelData = buffer.getReadPointer(0);
    for (int i = 0; i < fftSize; ++i) {
        fftData[i] = i < buffer.getNumSamples() ? channelData[i] : 0.0f;
    }

    // Apply window
    window.multiplyWithWindowingTable(fftData.data(), fftSize);

    // Perform FFT
    forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

    // Calculate magnitudes
    {
        juce::ScopedLock lock(fftLock);
        for (int i = 0; i < fftSize / 2; ++i) {
            magnitudes[i] = fftData[i];
        }
    }
}
```

### Resource Provider: Serve FFT Data

```cpp
if (resourceToRetrieve == "fftData.json") {
    std::vector<float> mags;
    processorRef.getFFTMagnitudes(mags);

    juce::DynamicObject::Ptr fftData{new juce::DynamicObject{}};
    juce::Array<juce::var> magArray;
    for (float mag : mags) {
        magArray.add(juce::Decibels::gainToDecibels(mag));
    }
    fftData->setProperty("magnitudes", magArray);

    const auto jsonString = juce::JSON::toString(fftData.get());
    juce::MemoryInputStream stream{jsonString.getCharPointer(),
                                   jsonString.getNumBytesAsUTF8(), false};

    return juce::WebBrowserComponent::Resource{
        streamToVector(stream),
        juce::String{"application/json"}
    };
}
```

### JavaScript: Visualize Spectrum

```javascript
const canvas = document.getElementById("spectrumCanvas");
const ctx = canvas.getContext("2d");

window.__JUCE__.backend.addEventListener("fftUpdate", async () => {
    const response = await fetch(Juce.getBackendResourceAddress("fftData.json"));
    const data = await response.json();

    ctx.clearRect(0, 0, canvas.width, canvas.height);

    const barWidth = canvas.width / data.magnitudes.length;
    const minDb = -100;
    const maxDb = 0;

    data.magnitudes.forEach((magnitude, i) => {
        const normalized = (magnitude - minDb) / (maxDb - minDb);
        const barHeight = normalized * canvas.height;

        ctx.fillStyle = `hsl(${i / data.magnitudes.length * 360}, 100%, 50%)`;
        ctx.fillRect(
            i * barWidth,
            canvas.height - barHeight,
            barWidth - 1,
            barHeight
        );
    });
});
```

---

## Performance Considerations

### Timer Frequency

**Too fast (< 30ms):** High CPU usage, choppy animation
**Too slow (> 100ms):** Laggy feel, missed updates
**Recommended:** 60ms (16 FPS) for meters, 33ms (30 FPS) for spectrums

```cpp
startTimer(60);  // Good for most visualizations
```

### Data Size

**Problem:** Sending 2048 FFT bins as JSON every 30ms = high bandwidth

**Solution 1:** Downsample FFT data
```cpp
// Send only every 4th bin
for (int i = 0; i < fftSize / 2; i += 4) {
    magArray.add(magnitudes[i]);
}
```

**Solution 2:** Send diff/delta only
```cpp
// Only send bins that changed significantly
for (int i = 0; i < fftSize / 2; ++i) {
    if (std::abs(magnitudes[i] - previousMagnitudes[i]) > threshold) {
        // Include this bin
    }
}
```

### Rendering Performance

**Use Canvas instead of DOM updates** for 60+ FPS animations.

**Bad (slow):**
```javascript
// Creating DOM elements every frame
divs.forEach((div, i) => {
    div.style.height = data[i] + "px";
});
```

**Good (fast):**
```javascript
// Canvas draw calls
ctx.fillRect(x, y, width, height);
```

---

## Example Libraries for Visualization

### Plotly.js
- **Use case:** Quick charts, interactive plots
- **Performance:** Good for < 60 FPS updates
- **Size:** ~3 MB (use CDN)

### D3.js
- **Use case:** Custom visualizations, data binding
- **Performance:** Excellent with Canvas renderer
- **Size:** ~250 KB

### Chart.js
- **Use case:** Standard charts (line, bar, pie)
- **Performance:** Good, optimized for real-time
- **Size:** ~200 KB

### Raw Canvas
- **Use case:** Maximum performance, full control
- **Performance:** Excellent (can hit 60 FPS easily)
- **Size:** 0 bytes (native browser API)

---

## Common Issues and Fixes

### Issue: Visualization lags behind audio

**Cause:** Too slow timer, or JSON parsing overhead

**Fix:**
- Increase timer frequency: `startTimer(30)` instead of `startTimer(60)`
- Reduce data size (downsample, send diff only)

### Issue: High CPU usage

**Cause:** Too frequent updates, heavy rendering

**Fix:**
- Decrease timer frequency
- Use `requestAnimationFrame()` instead of timer for smoother rendering
- Optimize Canvas drawing (batch operations, minimize state changes)

### Issue: JSON parse errors

**Cause:** Malformed JSON from C++, or NaN/Infinity values

**Fix:** Validate data before sending
```cpp
if (!std::isfinite(value)) {
    value = -100.0f;  // Clamp to safe value
}
```

---

## Best Practices

1. **Atomic variables** for shared audio data:
   ```cpp
   std::atomic<float> outputLevel{-100.0f};
   ```

2. **Critical sections** for complex data:
   ```cpp
   juce::CriticalSection fftLock;
   juce::ScopedLock lock(fftLock);
   ```

3. **Validate data** before sending as JSON:
   ```cpp
   if (std::isfinite(level) && level > -100.0f) {
       // Safe to send
   }
   ```

4. **Throttle updates** - don't send more data than you can render

5. **Use appropriate data structures:**
   - Simple values → JSON
   - Large arrays → Consider binary format (advanced)
   - Time series → Ringbuffer in C++, send windowed view

---

## Next Steps

Continue to **06-development-workflow.md** to learn hot reloading and debugging techniques.
