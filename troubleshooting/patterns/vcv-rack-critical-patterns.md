# VCV Rack 2 Critical Patterns

**Required Reading for all subagents before implementing VCV Rack modules.**

This document contains non-negotiable patterns that prevent repeat mistakes. Every implementation MUST follow these patterns.

---

## 1. Module Structure Pattern

### CORRECT Module Definition

```cpp
struct MyModule : Module {
    enum ParamId {
        FREQ_PARAM,
        LEVEL_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        VOCT_INPUT,
        GATE_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        AUDIO_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        ACTIVE_LIGHT,
        LIGHTS_LEN
    };

    // DSP state variables (declare here)
    float phase = 0.f;
    dsp::SchmittTrigger gateTrigger;

    MyModule() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        // Configure parameters with name, min, max, default, unit
        configParam(FREQ_PARAM, -3.f, 3.f, 0.f, "Frequency", " Hz", 2.f, dsp::FREQ_C4);
        configParam(LEVEL_PARAM, 0.f, 1.f, 0.5f, "Level", "%", 0.f, 100.f);

        // Configure inputs/outputs
        configInput(VOCT_INPUT, "1V/octave pitch");
        configInput(GATE_INPUT, "Gate");
        configOutput(AUDIO_OUTPUT, "Audio");
    }

    void process(const ProcessArgs& args) override {
        // Implementation here
    }
};
```

### WRONG - Common Mistakes

```cpp
// ❌ WRONG: Forgetting PARAMS_LEN, INPUTS_LEN, etc.
enum ParamId {
    FREQ_PARAM,
    LEVEL_PARAM
    // Missing PARAMS_LEN!
};

// ❌ WRONG: Not calling config() in constructor
MyModule() {
    // Missing: config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
}

// ❌ WRONG: Forgetting to call configParam/configInput/configOutput
```

---

## 2. ModuleWidget Pattern

### CORRECT Widget Definition

```cpp
struct MyModuleWidget : ModuleWidget {
    MyModuleWidget(MyModule* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/MyModule.svg")));

        // Add screws (optional, for aesthetics)
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Add params using mm2px for positioning
        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 46.063)), module, MyModule::FREQ_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(15.24, 77.478)), module, MyModule::LEVEL_PARAM));

        // Add inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 96.859)), module, MyModule::VOCT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 108.713)), module, MyModule::GATE_INPUT));

        // Add outputs
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 118.106)), module, MyModule::AUDIO_OUTPUT));

        // Add lights
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15.24, 25.81)), module, MyModule::ACTIVE_LIGHT));
    }
};

// CRITICAL: Create model at file scope
Model* modelMyModule = createModel<MyModule, MyModuleWidget>("MyModule");
```

### WRONG - Common Widget Mistakes

```cpp
// ❌ WRONG: Forgetting setModule(module)
MyModuleWidget(MyModule* module) {
    // Missing: setModule(module);
    setPanel(...);
}

// ❌ WRONG: Not declaring model at file scope
// The model MUST be declared at the bottom of the .cpp file

// ❌ WRONG: Using pixel coordinates instead of mm2px
addParam(createParamCentered<RoundBigBlackKnob>(Vec(100, 200), ...));  // Wrong!
addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 46.063)), ...));  // Correct!
```

---

## 3. Plugin Registration Pattern

### plugin.hpp (CORRECT)

```cpp
#pragma once
#include <rack.hpp>

using namespace rack;

// Declare plugin instance
extern Plugin* pluginInstance;

// Declare ALL module models
extern Model* modelMyModule;
extern Model* modelMyOtherModule;
```

### plugin.cpp (CORRECT)

```cpp
#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
    pluginInstance = p;

    // Register ALL modules
    p->addModel(modelMyModule);
    p->addModel(modelMyOtherModule);
}
```

### WRONG - Registration Mistakes

```cpp
// ❌ WRONG: Forgetting to declare model in plugin.hpp
// If not declared, linker error!

// ❌ WRONG: Forgetting to add model in plugin.cpp init()
// Module won't appear in VCV Rack!

// ❌ WRONG: Model name mismatch
// plugin.hpp: extern Model* modelMyModule;
// MyModule.cpp: Model* modelMYModule = ...;  // Case mismatch!
```

---

## 4. Voltage Standards (MUST FOLLOW)

### Audio Signals
- Range: ±5V (10Vpp)
- Always use this range for audio outputs

### Control Voltage (CV)
- Unipolar: 0V to +10V (LFOs, envelopes)
- Bipolar: ±5V (modulation)

### Pitch CV (1V/Octave)
- 0V = C4 (261.6256 Hz)
- +1V = C5, -1V = C3
- Use `dsp::FREQ_C4` constant

```cpp
// CORRECT pitch calculation
float pitch = params[FREQ_PARAM].getValue();
pitch += inputs[VOCT_INPUT].getVoltage();
float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);
```

### Gates
- OFF: 0V
- ON: +10V (not 5V, not 1V)

```cpp
// CORRECT gate output
outputs[GATE_OUTPUT].setVoltage(gateOpen ? 10.f : 0.f);
```

### Triggers
- +10V pulse
- 1ms duration (use `dsp::PulseGenerator`)

```cpp
// CORRECT trigger generation
dsp::PulseGenerator pulse;

// In process():
pulse.trigger(1e-3f);  // 1ms pulse
outputs[TRIG_OUTPUT].setVoltage(pulse.process(args.sampleTime) ? 10.f : 0.f);
```

---

## 5. Trigger Detection with Schmitt Trigger

### CORRECT - Use SchmittTrigger

```cpp
dsp::SchmittTrigger trigger;

void process(const ProcessArgs& args) override {
    // Default thresholds: 0.1V low, 1.0V high (Schmitt hysteresis)
    if (trigger.process(inputs[TRIG_INPUT].getVoltage(), 0.1f, 1.f)) {
        // Trigger detected!
        doSomething();
    }
}
```

### WRONG - Don't Use Simple Threshold

```cpp
// ❌ WRONG: Simple threshold causes false triggers
if (inputs[TRIG_INPUT].getVoltage() > 0.5f) {
    doSomething();  // Will trigger multiple times!
}
```

---

## 6. Clock/Reset Timing Pattern

**CRITICAL:** Cables introduce 1-sample delay. Modules with CLOCK and RESET must ignore CLOCK for 1ms after RESET.

### CORRECT Implementation

```cpp
dsp::Timer resetTimer;
dsp::SchmittTrigger clockTrigger;
dsp::SchmittTrigger resetTrigger;

void process(const ProcessArgs& args) override {
    // Handle reset first
    if (resetTrigger.process(inputs[RESET_INPUT].getVoltage())) {
        resetTimer.reset();
        // Perform reset actions
    }

    // Advance reset timer
    resetTimer.process(args.sampleTime);

    // Only process clock if >1ms since reset
    if (resetTimer.time > 1e-3f) {
        if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())) {
            // Handle clock
        }
    }
}
```

---

## 7. Polyphony Pattern

### CORRECT - Full Polyphony Support

```cpp
void process(const ProcessArgs& args) override {
    // Get channel count from "primary" input
    int channels = std::max(1, inputs[VOCT_INPUT].getChannels());

    for (int c = 0; c < channels; c++) {
        // Use getPolyVoltage for proper mono-to-poly handling
        float pitch = params[FREQ_PARAM].getValue();
        pitch += inputs[VOCT_INPUT].getPolyVoltage(c);

        // Process channel...
        float output = processChannel(c, pitch, args.sampleTime);

        outputs[AUDIO_OUTPUT].setVoltage(output, c);
    }

    // CRITICAL: Set output channel count!
    outputs[AUDIO_OUTPUT].setChannels(channels);
}
```

### WRONG - Forgetting Channel Count

```cpp
// ❌ WRONG: Not setting output channels
for (int c = 0; c < channels; c++) {
    outputs[AUDIO_OUTPUT].setVoltage(output, c);
}
// Missing: outputs[AUDIO_OUTPUT].setChannels(channels);
// Results in silent output on channels > 1!
```

### Polyphonic State Arrays

```cpp
// CORRECT: Use arrays for per-channel state
float phase[16] = {};
dsp::SchmittTrigger triggers[16];

// Or for complex state:
struct Engine {
    float phase = 0.f;
    float lastFreq = 0.f;
};
Engine engines[16];
```

---

## 8. Font/Image Loading Pattern (DAW Plugin Compatible)

**CRITICAL:** Don't store Font/Image in constructor. Load each frame for DAW plugin compatibility.

### CORRECT Pattern

```cpp
struct MyWidget : Widget {
    std::string fontPath;

    MyWidget() {
        fontPath = asset::plugin(pluginInstance, "res/MyFont.ttf");
    }

    void draw(const DrawArgs& args) override {
        std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
        if (font) {
            nvgFontFaceId(args.vg, font->handle);
            nvgFontSize(args.vg, 14.f);
            nvgFillColor(args.vg, nvgRGB(255, 255, 255));
            nvgText(args.vg, 10, 20, "Hello", NULL);
        }
    }
};
```

### WRONG - Breaks in Rack Pro

```cpp
// ❌ WRONG: Storing font reference in constructor
struct MyWidget : Widget {
    std::shared_ptr<Font> font;

    MyWidget() {
        font = APP->window->loadFont(fontPath);  // DON'T DO THIS!
    }
};
```

---

## 9. Real-Time Safety in process()

### NEVER Do These in process()

```cpp
void process(const ProcessArgs& args) override {
    // ❌ NEVER: Memory allocation
    std::vector<float> buffer;
    buffer.push_back(1.f);  // Allocates!

    // ❌ NEVER: File I/O
    FILE* f = fopen("data.txt", "r");

    // ❌ NEVER: Locks/Mutexes
    std::lock_guard<std::mutex> lock(mutex);

    // ❌ NEVER: Network calls
    // ❌ NEVER: System calls
    // ❌ NEVER: Exceptions (throw/catch)
    // ❌ NEVER: Logging (except DEBUG in debug builds)
}
```

### ALWAYS Do These

```cpp
void process(const ProcessArgs& args) override {
    // ✅ ALWAYS: Use preallocated buffers
    // (Allocate in constructor or onSampleRateChange)

    // ✅ ALWAYS: Use lock-free operations

    // ✅ ALWAYS: Keep execution bounded
}
```

---

## 10. SVG Panel Requirements

### SVG File Structure

1. Create `components` layer in your SVG
2. Add placeholder circles for widgets:
   - Red `#FF0000`: Params (knobs, sliders)
   - Green `#00FF00`: Input ports
   - Blue `#0000FF`: Output ports
   - Magenta `#FF00FF`: Lights
3. **HIDE the components layer** before saving
4. Save as Plain SVG

### Panel Dimensions

- 1 HP = 5.08mm = 15px (at 75 DPI)
- Panel height = 128.5mm = 380px

### SVG Must-Nots

- ❌ No `<text>` elements (NanoVG doesn't render them)
- ❌ No fonts/text objects
- ❌ No CSS `<style>` blocks (use inline styles only)
- ❌ No complex gradients (simple 2-color linear only)
- ❌ No clipping masks
- ❌ No filters/effects
- ❌ No embedded images/bitmaps

### Adding Text Labels to SVG Panels

Text MUST be converted to paths. Use Inkscape:

**Manual (GUI):**
1. Create text with the Text tool
2. Select the text
3. Menu: Path > Object to Path (Ctrl+Shift+C)
4. Save as Plain SVG

**Automated (CLI):**
```bash
/Applications/Inkscape.app/Contents/MacOS/inkscape \
  --export-text-to-path \
  --export-plain-svg \
  --export-filename="output.svg" \
  "input_with_text.svg"
```

This converts all `<text>` elements to `<path>` elements automatically.

**CRITICAL:** Raw `<text>` elements will NOT render in VCV Rack!

---

## 11. Makefile Pattern

### CORRECT Plugin Makefile

```makefile
# Set RACK_DIR if not set as environment variable
ifndef RACK_DIR
RACK_DIR ?= $(HOME)/Rack-SDK
endif

# Include Rack SDK makefile
include $(RACK_DIR)/plugin.mk
```

### plugin.json (CORRECT)

```json
{
  "slug": "MyPlugin",
  "name": "My Plugin",
  "version": "2.0.0",
  "license": "GPL-3.0-or-later",
  "brand": "MyBrand",
  "author": "Your Name",
  "authorEmail": "you@example.com",
  "pluginUrl": "https://github.com/you/MyPlugin",
  "sourceUrl": "https://github.com/you/MyPlugin",
  "modules": [
    {
      "slug": "MyModule",
      "name": "My Module",
      "description": "A cool module",
      "tags": ["Oscillator", "Polyphonic"]
    }
  ]
}
```

**CRITICAL:** Version MUST start with `2.` for VCV Rack 2 compatibility.

---

## 12. Common Build Errors

### Undefined Reference to modelMyModule

**Cause:** Model not declared in plugin.hpp or not registered in plugin.cpp

**Fix:**
```cpp
// plugin.hpp
extern Model* modelMyModule;  // Add this

// plugin.cpp
p->addModel(modelMyModule);  // Add this
```

### RACK_DIR Not Found

**Cause:** Environment variable not set

**Fix:**
```bash
export RACK_DIR=~/Rack-SDK
# Add to ~/.zshrc for persistence
```

### SVG Not Found

**Cause:** Wrong path or filename

**Fix:**
- Verify file exists at `res/ModuleName.svg`
- Check case sensitivity
- Ensure path in `asset::plugin()` matches exactly

---

## 13. Testing Checklist

Before declaring a module complete:

- [ ] Module loads without crash
- [ ] All parameters work
- [ ] Audio output is correct level (±5V for audio)
- [ ] CV inputs respond correctly
- [ ] Triggers detected reliably
- [ ] Polyphony works (if applicable)
- [ ] Clock/reset timing correct
- [ ] No denormals in audio
- [ ] No memory leaks
- [ ] Module unloads cleanly

---

## Quick Reference

### ProcessArgs Fields

```cpp
void process(const ProcessArgs& args) override {
    args.sampleRate;  // Current sample rate (e.g., 44100, 48000)
    args.sampleTime;  // 1.0 / sampleRate
    args.frame;       // Current frame number (uint64_t)
}
```

### Port Methods

```cpp
// Inputs
inputs[ID].getVoltage();           // First channel voltage
inputs[ID].getVoltage(channel);    // Specific channel
inputs[ID].getPolyVoltage(c);      // Handles mono-to-poly
inputs[ID].getVoltageSum();        // Sum all channels
inputs[ID].getChannels();          // Number of channels
inputs[ID].isConnected();          // Is cable connected?

// Outputs
outputs[ID].setVoltage(v);         // Set first channel
outputs[ID].setVoltage(v, c);      // Set specific channel
outputs[ID].setChannels(n);        // MUST call for polyphonic!
```

### Common Widget Classes

```cpp
// Knobs
RoundBigBlackKnob, RoundLargeBlackKnob, RoundBlackKnob, RoundSmallBlackKnob
Trimpot, RoundBlackSnapKnob

// Ports
PJ301MPort, CL1362Port

// Switches
CKSS, CKSSThree, NKK, BefacoSwitch

// Buttons
VCVButton, LEDButton, TL1105

// Lights
SmallLight<T>, MediumLight<T>, LargeLight<T>
// T = RedLight, GreenLight, BlueLight, YellowLight, WhiteLight
// or GreenRedLight, RedGreenBlueLight

// Screws
ScrewSilver, ScrewBlack
```

### Constants

```cpp
RACK_GRID_WIDTH   // 15 (1 HP in pixels)
RACK_GRID_HEIGHT  // 380 (panel height in pixels)
mm2px(Vec(x, y))  // Convert mm to pixels
dsp::FREQ_C4      // 261.6256f Hz
M_PI              // 3.14159...
```

---

*Last updated: January 2026*
*Based on VCV Rack 2.6.x API*
