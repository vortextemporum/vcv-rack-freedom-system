# Parameter Binding - Control Audio Parameters from WebView

## Overview

JUCE WebView uses the **Relay Pattern** to synchronize audio parameters with web UI controls. This guide covers all three parameter types: **float (slider)**, **bool (checkbox)**, and **choice (select/combobox)**.

## Architecture: Traditional vs WebView

### Traditional JUCE Pattern

```
AudioParameterFloat
       ↕ (synced by)
SliderParameterAttachment
       ↕
Slider (C++ GUI Component)
```

### WebView Pattern

```
AudioParameterFloat
       ↕ (synced by)
WebSliderParameterAttachment
       ↕
WebSliderRelay  ← Exposes SliderState to JavaScript
       ↕
SliderState (JavaScript object)
       ↕ (you implement binding)
HTML <input type="range">
```

**Key Insight:** The **Relay** class bridges C++ and JavaScript. It exposes a JavaScript state object that you bind to HTML elements.

---

## Parameter Type 1: Float (Slider)

### Step 1: Define Parameter ID

**plugin/include/PluginName/ParameterIDs.h**

```cpp
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace id {
    const juce::ParameterID GAIN{"GAIN", 1};  // ID, version
}
```

### Step 2: Create AudioParameterFloat

**plugin/source/PluginProcessor.cpp**

```cpp
#include "PluginName/ParameterIDs.h"

namespace {
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            id::GAIN.getParamID(),       // ID
            "Gain",                       // Name
            juce::NormalisableRange<float>{
                0.0f,   // min
                1.0f,   // max
                0.01f,  // step
                0.9f    // skew (for perceptual scaling)
            },
            1.0f  // default value
        ));

        return layout;
    }
}

// In PluginProcessor constructor
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()...),
      state(*this, nullptr, "State", createParameterLayout())
{
}
```

### Step 3: Use Parameter in Audio Processing

```cpp
// PluginProcessor.h - Add member
std::atomic<float>* gainParameter{nullptr};

// PluginProcessor.cpp - Constructor
gainParameter = state.getRawParameterValue(id::GAIN.getParamID());

// processBlock()
void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    buffer.applyGain(gainParameter->load());
}
```

### Step 4: Create C++ Slider (Optional, for hybrid UI)

**plugin/include/PluginName/PluginEditor.h**

```cpp
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
private:
    juce::Slider gainSlider{"gain"};
    juce::SliderParameterAttachment gainSliderAttachment;

    // WebView components
    juce::WebSliderRelay webGainRelay;
    juce::WebSliderParameterAttachment webGainSliderAttachment;
    juce::WebBrowserComponent webView;
};
```

**plugin/source/PluginEditor.cpp**

```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      // C++ slider attachment
      gainSliderAttachment{
          *processorRef.getState().getParameter(id::GAIN.getParamID()),
          gainSlider,
          nullptr  // undo manager
      },
      // WebView relay - MUST initialize before webView
      webGainRelay{id::GAIN.getParamID()},
      // WebView component
      webView(juce::WebBrowserComponent::Options{}
          // ... other options ...
          .withOptionsFrom(webGainRelay)),  // CRITICAL: Pass relay options
      // WebView slider attachment
      webGainSliderAttachment{
          *processorRef.getState().getParameter(id::GAIN.getParamID()),
          webGainRelay,  // NOT gainSlider!
          nullptr
      }
{
    // C++ slider setup
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearBar);
    addAndMakeVisible(gainSlider);

    // WebView setup
    addAndMakeVisible(webView);
    webView.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
}

void AudioPluginAudioProcessorEditor::resized() {
    auto bounds = getBounds();
    // Left half: C++ GUI
    gainSlider.setBounds(bounds.removeFromLeft(getWidth() / 2).removeFromTop(50).reduced(5));
    // Right half: WebView
    webView.setBounds(bounds);
}
```

### Step 5: Create HTML Slider

**plugin/ui/public/index.html**

```html
<input type="range" id="gainSlider" min="0" max="1" step="0.01">
<label for="gainSlider">Gain</label>
```

### Step 6: Bind JavaScript to Parameter

**plugin/ui/public/js/index.js**

```javascript
import * as Juce from "./juce/index.js";

// Get HTML slider
const slider = document.getElementById("gainSlider");

// Get parameter state from JUCE backend
const sliderState = Juce.getSliderState("GAIN");

// Set slider properties from parameter
slider.min = sliderState.properties.start;
slider.max = sliderState.properties.end;
slider.step = 1 / sliderState.properties.numSteps;

// CRITICAL: Bind slider → parameter
slider.oninput = function() {
    sliderState.setNormalisedValue(this.value);
};

// CRITICAL: Bind parameter → slider
sliderState.valueChangedEvent.addListener(() => {
    slider.value = sliderState.getNormalisedValue();
});
```

**How it works:**
1. User moves HTML slider → `oninput` fires → `setNormalisedValue()` updates parameter
2. Parameter changes (automation, preset load) → `valueChangedEvent` fires → slider updates

---

## Parameter Type 2: Bool (Checkbox/Toggle)

### Step 1: Define Parameter

**ParameterIDs.h**

```cpp
namespace id {
    const juce::ParameterID BYPASS{"BYPASS", 1};
}
```

### Step 2: Create AudioParameterBool

**PluginProcessor.cpp**

```cpp
layout.add(std::make_unique<juce::AudioParameterBool>(
    id::BYPASS.getParamID(),
    "Bypass",           // Name
    false,              // Default: not bypassed
    juce::AudioParameterBoolAttributes().withLabel("Bypass")
));
```

### Step 3: Use in Audio Processing

```cpp
// PluginProcessor.h
juce::AudioParameterBool& bypassParameter;

// PluginProcessor.cpp - Constructor
bypassParameter(*dynamic_cast<juce::AudioParameterBool*>(
    state.getParameter(id::BYPASS.getParamID())))
{
}

// processBlock()
if (bypassParameter.get()) {
    return;  // Bypass processing
}
```

### Step 4: Create Relay and Attachment

**PluginEditor.h**

```cpp
// C++ component (optional)
juce::ToggleButton bypassButton{"Bypass"};
juce::ButtonParameterAttachment bypassButtonAttachment;

// WebView components
juce::WebToggleButtonRelay webBypassRelay;
juce::WebToggleButtonParameterAttachment webBypassToggleAttachment;
```

**PluginEditor.cpp**

```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      bypassButtonAttachment{
          *processorRef.getState().getParameter(id::BYPASS.getParamID()),
          bypassButton,
          nullptr
      },
      webBypassRelay{id::BYPASS.getParamID()},
      webView(juce::WebBrowserComponent::Options{}
          // ... other options ...
          .withOptionsFrom(webBypassRelay)),
      webBypassToggleAttachment{
          *processorRef.getState().getParameter(id::BYPASS.getParamID()),
          webBypassRelay,
          nullptr
      }
{
    addAndMakeVisible(bypassButton);
    // ... rest of setup
}
```

### Step 5: Create HTML Checkbox

**index.html**

```html
<label for="bypassCheckbox">Bypass</label>
<input type="checkbox" id="bypassCheckbox">
```

### Step 6: Bind JavaScript

**index.js**

```javascript
const bypassCheckbox = document.getElementById("bypassCheckbox");
const bypassToggleState = Juce.getToggleState("BYPASS");

// Checkbox → Parameter
bypassCheckbox.oninput = function() {
    bypassToggleState.setValue(this.checked);
};

// Parameter → Checkbox
bypassToggleState.valueChangedEvent.addListener(() => {
    bypassCheckbox.checked = bypassToggleState.getValue();
});
```

---

## Parameter Type 3: Choice (Select/ComboBox)

### Step 1: Define Parameter

**ParameterIDs.h**

```cpp
namespace id {
    const juce::ParameterID DISTORTION_TYPE{"DISTORTION_TYPE", 1};
}
```

### Step 2: Create AudioParameterChoice

**PluginProcessor.cpp**

```cpp
layout.add(std::make_unique<juce::AudioParameterChoice>(
    id::DISTORTION_TYPE.getParamID(),
    "Distortion Type",
    juce::StringArray{"None", "TanH", "Sigmoid"},  // Choices
    0  // Default index (None)
));
```

### Step 3: Use in Audio Processing

```cpp
// PluginProcessor.h
juce::AudioParameterChoice* distortionTypeParameter{nullptr};

// PluginProcessor.cpp - Constructor
distortionTypeParameter = dynamic_cast<juce::AudioParameterChoice*>(
    state.getParameter(id::DISTORTION_TYPE.getParamID()));

// processBlock()
const int distortionType = distortionTypeParameter->getIndex();
if (distortionType == 1) {
    // Apply TanH distortion
} else if (distortionType == 2) {
    // Apply Sigmoid distortion
}
```

### Step 4: Create Relay and Attachment

**PluginEditor.h**

```cpp
// C++ component (optional)
juce::Label distortionTypeLabel{"label", "Distortion"};
juce::ComboBox distortionTypeComboBox{"combo"};
juce::ComboBoxParameterAttachment distortionTypeComboBoxAttachment;

// WebView components
juce::WebComboBoxRelay webDistortionTypeRelay;
juce::WebComboBoxParameterAttachment webDistortionTypeComboBoxAttachment;
```

**PluginEditor.cpp**

```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      distortionTypeComboBoxAttachment{
          *processorRef.getState().getParameter(id::DISTORTION_TYPE.getParamID()),
          distortionTypeComboBox,
          nullptr
      },
      webDistortionTypeRelay{id::DISTORTION_TYPE.getParamID()},
      webView(juce::WebBrowserComponent::Options{}
          // ... other options ...
          .withOptionsFrom(webDistortionTypeRelay)),
      webDistortionTypeComboBoxAttachment{
          *processorRef.getState().getParameter(id::DISTORTION_TYPE.getParamID()),
          webDistortionTypeRelay,
          nullptr
      }
{
    // C++ combo box setup
    const auto& param = processorRef.getDistortionTypeParameter();
    distortionTypeComboBox.addItemList(param.choices, 1);  // offset = 1
    distortionTypeComboBox.setSelectedItemIndex(param.getIndex(), juce::dontSendNotification);
    addAndMakeVisible(distortionTypeLabel);
    addAndMakeVisible(distortionTypeComboBox);

    // ... rest of setup
}
```

### Step 5: Create HTML Select

**index.html**

```html
<label for="distortionTypeComboBox">Distortion Type</label>
<select id="distortionTypeComboBox"></select>
```

### Step 6: Bind JavaScript

**index.js**

```javascript
const distortionTypeComboBox = document.getElementById("distortionTypeComboBox");
const distortionTypeComboBoxState = Juce.getComboBoxState("DISTORTION_TYPE");

// Populate options when choices change (or at startup)
distortionTypeComboBoxState.propertiesChangedEvent.addListener(() => {
    distortionTypeComboBox.innerHTML = "";  // Clear existing
    distortionTypeComboBoxState.properties.choices.forEach(choice => {
        distortionTypeComboBox.innerHTML += `<option value="${choice}">${choice}</option>`;
    });
});

// ComboBox → Parameter
distortionTypeComboBox.oninput = function() {
    distortionTypeComboBoxState.setChoiceIndex(this.selectedIndex);
};

// Parameter → ComboBox
distortionTypeComboBoxState.valueChangedEvent.addListener(() => {
    distortionTypeComboBox.selectedIndex = distortionTypeComboBoxState.getChoiceIndex();
});
```

---

## Complete Multi-Parameter Example

### PluginEditor.h

```cpp
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
private:
    AudioPluginAudioProcessor& processorRef;

    // C++ GUI components
    juce::Slider gainSlider{"gain"};
    juce::SliderParameterAttachment gainSliderAttachment;

    juce::ToggleButton bypassButton{"Bypass"};
    juce::ButtonParameterAttachment bypassButtonAttachment;

    juce::Label distortionTypeLabel{"label", "Distortion"};
    juce::ComboBox distortionTypeComboBox{"combo"};
    juce::ComboBoxParameterAttachment distortionTypeComboBoxAttachment;

    // WebView relays (MUST initialize before webView)
    juce::WebSliderRelay webGainRelay;
    juce::WebToggleButtonRelay webBypassRelay;
    juce::WebComboBoxRelay webDistortionTypeRelay;

    // WebView component
    juce::WebBrowserComponent webView;

    // WebView attachments (MUST initialize after webView)
    juce::WebSliderParameterAttachment webGainSliderAttachment;
    juce::WebToggleButtonParameterAttachment webBypassToggleAttachment;
    juce::WebComboBoxParameterAttachment webDistortionTypeComboBoxAttachment;
};
```

### PluginEditor.cpp Constructor

```cpp
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processorRef(p),
      // C++ attachments
      gainSliderAttachment{
          *processorRef.getState().getParameter(id::GAIN.getParamID()),
          gainSlider, nullptr},
      bypassButtonAttachment{
          *processorRef.getState().getParameter(id::BYPASS.getParamID()),
          bypassButton, nullptr},
      distortionTypeComboBoxAttachment{
          *processorRef.getState().getParameter(id::DISTORTION_TYPE.getParamID()),
          distortionTypeComboBox, nullptr},
      // WebView relays (initialize before webView!)
      webGainRelay{id::GAIN.getParamID()},
      webBypassRelay{id::BYPASS.getParamID()},
      webDistortionTypeRelay{id::DISTORTION_TYPE.getParamID()},
      // WebView component
      webView(juce::WebBrowserComponent::Options{}
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withNativeIntegrationEnabled()
          .withResourceProvider([this](const auto& url) { return getResource(url); })
          // CRITICAL: Pass all relay options
          .withOptionsFrom(webGainRelay)
          .withOptionsFrom(webBypassRelay)
          .withOptionsFrom(webDistortionTypeRelay)),
      // WebView attachments (initialize after webView!)
      webGainSliderAttachment{
          *processorRef.getState().getParameter(id::GAIN.getParamID()),
          webGainRelay, nullptr},
      webBypassToggleAttachment{
          *processorRef.getState().getParameter(id::BYPASS.getParamID()),
          webBypassRelay, nullptr},
      webDistortionTypeComboBoxAttachment{
          *processorRef.getState().getParameter(id::DISTORTION_TYPE.getParamID()),
          webDistortionTypeRelay, nullptr}
{
    // Setup C++ components...
    // Setup WebView...
}
```

### JavaScript (index.js)

```javascript
import * as Juce from "./juce/index.js";

document.addEventListener("DOMContentLoaded", () => {
    // Gain slider
    const gainSlider = document.getElementById("gainSlider");
    const gainSliderState = Juce.getSliderState("GAIN");
    gainSlider.step = 1 / gainSliderState.properties.numSteps;
    gainSlider.oninput = () => gainSliderState.setNormalisedValue(gainSlider.value);
    gainSliderState.valueChangedEvent.addListener(() => {
        gainSlider.value = gainSliderState.getNormalisedValue();
    });

    // Bypass checkbox
    const bypassCheckbox = document.getElementById("bypassCheckbox");
    const bypassToggleState = Juce.getToggleState("BYPASS");
    bypassCheckbox.oninput = () => bypassToggleState.setValue(bypassCheckbox.checked);
    bypassToggleState.valueChangedEvent.addListener(() => {
        bypassCheckbox.checked = bypassToggleState.getValue();
    });

    // Distortion type combo box
    const distortionComboBox = document.getElementById("distortionTypeComboBox");
    const distortionComboBoxState = Juce.getComboBoxState("DISTORTION_TYPE");

    distortionComboBoxState.propertiesChangedEvent.addListener(() => {
        distortionComboBox.innerHTML = "";
        distortionComboBoxState.properties.choices.forEach(choice => {
            distortionComboBox.innerHTML += `<option>${choice}</option>`;
        });
    });

    distortionComboBox.oninput = () => {
        distortionComboBoxState.setChoiceIndex(distortionComboBox.selectedIndex);
    };

    distortionComboBoxState.valueChangedEvent.addListener(() => {
        distortionComboBox.selectedIndex = distortionComboBoxState.getChoiceIndex();
    });
});
```

---

## Common Issues and Fixes

### Issue: "Cannot read property 'properties' of undefined"

**Cause:** Parameter ID mismatch between C++ and JavaScript.

**Fix:** Ensure parameter IDs match exactly:
```cpp
webGainRelay{id::GAIN.getParamID()}  // C++
```
```javascript
Juce.getSliderState("GAIN")  // JS - must match!
```

### Issue: Slider doesn't update when preset loads

**Cause:** Forgot to add `valueChangedEvent` listener.

**Fix:** Always bind both directions:
```javascript
// User moves slider → update parameter
slider.oninput = () => sliderState.setNormalisedValue(slider.value);

// Parameter changes → update slider (THIS IS CRITICAL!)
sliderState.valueChangedEvent.addListener(() => {
    slider.value = sliderState.getNormalisedValue();
});
```

### Issue: ComboBox shows nothing

**Cause:** Forgot to populate options from `propertiesChangedEvent`.

**Fix:**
```javascript
comboBoxState.propertiesChangedEvent.addListener(() => {
    // Populate dropdown from choices
});
```

### Issue: "Cannot call withOptionsFrom on undefined"

**Cause:** Relay not initialized before WebView component.

**Fix:** Check initialization order in constructor:
```cpp
webGainRelay{id::GAIN.getParamID()},  // MUST be before webView
webView(Options{}.withOptionsFrom(webGainRelay)),
```

---

## Best Practices

### Initialization Order (Critical!)

```cpp
// In constructor initializer list:
// 1. C++ parameter attachments
gainSliderAttachment{...},
// 2. WebView relays
webGainRelay{id::GAIN.getParamID()},
// 3. WebView component (references relays)
webView(Options{}.withOptionsFrom(webGainRelay)),
// 4. WebView parameter attachments (references webView and relays)
webGainSliderAttachment{..., webGainRelay, ...}
```

### Parameter ID Management

**Use a dedicated header for all parameter IDs:**
```cpp
// ParameterIDs.h
namespace id {
    const juce::ParameterID GAIN{"GAIN", 1};
    const juce::ParameterID BYPASS{"BYPASS", 1};
    // ... all parameters in one place
}
```

### Normalized vs Scaled Values

- **Sliders:** Use `getNormalisedValue()` / `setNormalisedValue()` for 0-1 range
- **Sliders (scaled):** Use `getScaledValue()` / `setScaledValue()` for actual range
- **Toggles:** Use `getValue()` / `setValue()` (boolean)
- **ComboBoxes:** Use `getChoiceIndex()` / `setChoiceIndex()` (integer)

### Thread Safety

All parameter changes are **automatically thread-safe** - JUCE handles this internally.

---

## Next Steps

Continue to **05-audio-visualization.md** to learn how to display real-time audio data in your WebView UI.
