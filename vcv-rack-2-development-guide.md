# VCV Rack 2 Plugin Development Guide for macOS

**Last Updated: January 2026**  
**Current VCV Rack Version: 2.6.x**

---

## Table of Contents

1. [Prerequisites & System Requirements](#1-prerequisites--system-requirements)
2. [Development Environment Setup](#2-development-environment-setup)
3. [Creating Your First Plugin](#3-creating-your-first-plugin)
4. [Panel Design (SVG)](#4-panel-design-svg)
5. [Module Development](#5-module-development)
6. [Voltage Standards](#6-voltage-standards)
7. [Polyphony Support](#7-polyphony-support)
8. [DSP Best Practices](#8-dsp-best-practices)
9. [Licensing Requirements](#9-licensing-requirements)
10. [Building & Distribution](#10-building--distribution)
11. [Submitting to VCV Library](#11-submitting-to-vcv-library)
12. [Common Pitfalls & Must-Nots](#12-common-pitfalls--must-nots)
13. [Quick Reference](#13-quick-reference)

---

## 1. Prerequisites & System Requirements

### Hardware Requirements
- **CPU**: Intel/AMD x64 with SSE4.2 (2011+) or Apple Silicon (M1/M2/M3)
- **Graphics**: Dedicated Nvidia/AMD GPU recommended (integrated graphics may cause high CPU usage)
- **RAM**: 8GB minimum recommended
- **Disk Space**: ~15GB for full toolchain build

### Software Requirements
- **macOS**: 10.15 (Catalina) or later
- **Xcode Command Line Tools**: Required for compiler
- **Homebrew**: Package manager for dependencies
- **Git**: Version control
- **Python 3**: For helper scripts
- **Vector Graphics Editor**: Inkscape (recommended), Illustrator, or Affinity Designer

### Knowledge Prerequisites
- C++11 proficiency
- Command line navigation (cd, ls, make)
- Basic understanding of modular synthesis concepts
- DSP knowledge (for audio processing modules)

---

## 2. Development Environment Setup

### Step 1: Install Xcode Command Line Tools
```bash
xcode-select --install
```

### Step 2: Install Homebrew
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### Step 3: Install Build Dependencies
```bash
brew install git wget cmake autoconf automake libtool jq python zstd pkg-config
```

### Step 4: Download the Rack SDK

Download the appropriate SDK for your architecture:
- **Mac x64 (Intel)**: Download from https://vcvrack.com/downloads/
- **Mac ARM64 (Apple Silicon)**: Download from https://vcvrack.com/downloads/

Extract to a convenient location:
```bash
cd ~
unzip Rack-SDK-2.x.x-mac-arm64.zip  # or mac-x64 for Intel
```

### Step 5: Set Environment Variable
Add to your shell configuration (`~/.zshrc` or `~/.bashrc`):
```bash
export RACK_DIR=~/Rack-SDK
```

Reload your shell:
```bash
source ~/.zshrc
```

### Step 6: Install VCV Rack (for testing)
```bash
brew install --cask vcv-rack
```

Or download directly from https://vcvrack.com/Rack

### Apple Silicon Development Notes

If developing on Apple Silicon but targeting x64:
```bash
# Switch to x86_64 shell
arch -x86_64 zsh

# Install x86 Homebrew (separate installation)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# x86 Homebrew installs to /usr/local/bin/brew
# ARM Homebrew installs to /opt/homebrew/bin/brew
```

---

## 3. Creating Your First Plugin

### Step 1: Create Plugin from Template

Navigate to your development directory and run:
```bash
cd ~/Development  # or your preferred location
$RACK_DIR/helper.py createplugin MyPlugin
```

Answer the prompts:
```
Plugin name [MyPlugin]: My Plugin
Version [1.0.0]: 2.0.0
License [proprietary]: GPL-3.0-or-later
Brand [My Plugin]: MyBrand
Author []: Your Name
Author email (optional) []: your@email.com
Plugin website URL (optional) []: https://yoursite.com
Source code URL (optional) []: https://github.com/yourusername/MyPlugin
```

### Step 2: Test the Build System
```bash
cd MyPlugin
make
```

If successful, this creates an empty plugin with no modules.

### Step 3: Create a Module
First, create your SVG panel (see Section 4), then:
```bash
$RACK_DIR/helper.py createmodule MyModule
```

The helper script will:
- Parse your SVG for components
- Generate `src/MyModule.cpp`
- Update `plugin.json` manifest
- Provide instructions for `plugin.hpp` and `plugin.cpp`

### Step 4: Register the Module

Edit `src/plugin.hpp`:
```cpp
#pragma once
#include <rack.hpp>

using namespace rack;

extern Plugin* pluginInstance;

// Declare your module
extern Model* modelMyModule;
```

Edit `src/plugin.cpp`:
```cpp
#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p) {
    pluginInstance = p;
    
    // Register your module
    p->addModel(modelMyModule);
}
```

---

## 4. Panel Design (SVG)

### Eurorack Dimensions
- **1 HP** = 5.08mm = 15 pixels (at standard 75 DPI)
- **Panel height** = 128.5mm = 380 pixels
- Standard widths: 3HP, 4HP, 6HP, 8HP, 10HP, 12HP, etc.

### SVG Requirements

#### Supported Features
- Path objects with solid fill and stroke
- Group objects (`<g>`)
- Simple two-color linear gradients (limited support)
- Basic shapes converted to paths

#### NOT Supported (MUST AVOID)
- ❌ Text (must convert to paths: Path → Object to Path in Inkscape)
- ❌ Fonts/text objects
- ❌ Complex gradients
- ❌ CSS stylesheets
- ❌ Clipping masks
- ❌ Filters and effects
- ❌ Images/bitmaps

### Component Layer Setup

1. Create a layer named exactly `components` in your SVG
2. Add placeholder circles for each component:

| Component Type | Circle Color | Object ID Format |
|---------------|--------------|------------------|
| Param (knob/slider) | `#FF0000` (red) | `param_NAME` |
| Input port | `#00FF00` (green) | `input_NAME` |
| Output port | `#0000FF` (blue) | `output_NAME` |
| Light | `#FF00FF` (magenta) | `light_NAME` |
| Custom widget | `#FFFF00` (yellow) | `widget_NAME` |

3. **Hide the components layer** before saving (click eye icon in Layers panel)
4. Save as Plain SVG to `res/MyModule.svg`

### Example Panel Structure
```
Document
├── background (layer)
│   └── panel artwork
└── components (layer, hidden)
    ├── circle (red, id="param_PITCH")
    ├── circle (green, id="input_VOCT")
    ├── circle (blue, id="output_SINE")
    └── circle (magenta, id="light_BLINK")
```

---

## 5. Module Development

### Basic Module Structure

```cpp
#include "plugin.hpp"

struct MyModule : Module {
    enum ParamId {
        PITCH_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        PITCH_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        SINE_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        BLINK_LIGHT,
        LIGHTS_LEN
    };

    // DSP state variables
    float phase = 0.f;
    float blinkPhase = 0.f;

    MyModule() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        
        // Configure parameters with name, min, max, default
        configParam(PITCH_PARAM, -3.f, 3.f, 0.f, "Pitch", " V");
        
        // Configure inputs/outputs
        configInput(PITCH_INPUT, "1V/octave pitch");
        configOutput(SINE_OUTPUT, "Sine wave");
    }

    void process(const ProcessArgs& args) override {
        // Get pitch from knob and CV input
        float pitch = params[PITCH_PARAM].getValue();
        pitch += inputs[PITCH_INPUT].getVoltage();
        
        // Calculate frequency (C4 = 261.6256 Hz at 0V)
        float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);
        
        // Accumulate phase
        phase += freq * args.sampleTime;
        if (phase >= 1.f)
            phase -= 1.f;
        
        // Generate sine wave
        float sine = std::sin(2.f * M_PI * phase);
        
        // Output at ±5V (standard audio level)
        outputs[SINE_OUTPUT].setVoltage(5.f * sine);
        
        // Blink light at 1Hz
        blinkPhase += args.sampleTime;
        if (blinkPhase >= 1.f)
            blinkPhase -= 1.f;
        lights[BLINK_LIGHT].setBrightness(blinkPhase < 0.5f ? 1.f : 0.f);
    }
};
```

### ModuleWidget Structure

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

        // Add params (positions from helper.py or manual)
        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 46.063)), module, MyModule::PITCH_PARAM));

        // Add inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 77.478)), module, MyModule::PITCH_INPUT));

        // Add outputs
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 108.713)), module, MyModule::SINE_OUTPUT));

        // Add lights
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15.24, 25.81)), module, MyModule::BLINK_LIGHT));
    }
};

Model* modelMyModule = createModel<MyModule, MyModuleWidget>("MyModule");
```

### Saving/Loading Custom Data (JSON)

```cpp
json_t* dataToJson() override {
    json_t* rootJ = json_object();
    json_object_set_new(rootJ, "mode", json_integer(mode));
    json_object_set_new(rootJ, "setting", json_real(setting));
    return rootJ;
}

void dataFromJson(json_t* rootJ) override {
    json_t* modeJ = json_object_get(rootJ, "mode");
    if (modeJ)
        mode = json_integer_value(modeJ);
    
    json_t* settingJ = json_object_get(rootJ, "setting");
    if (settingJ)
        setting = json_real_value(settingJ);
}
```

---

## 6. Voltage Standards

### Signal Levels

| Signal Type | Voltage Range | Notes |
|-------------|---------------|-------|
| **Audio** | ±5V | 10Vpp standard |
| **Unipolar CV** | 0V to +10V | LFOs, envelopes |
| **Bipolar CV** | ±5V | Modulation sources |
| **1V/octave** | Any voltage | 1V = 1 octave, 0V = C4 (261.6256 Hz) |
| **Gates** | 0V (off) / +10V (on) | Binary on/off |
| **Triggers** | +10V pulse, 1ms duration | Use `dsp::PulseGenerator` |

### Trigger Detection
Use Schmitt trigger to prevent false triggers:
```cpp
dsp::SchmittTrigger schmittTrigger;

void process(const ProcessArgs& args) override {
    if (schmittTrigger.process(inputs[TRIG_INPUT].getVoltage(), 0.1f, 1.f)) {
        // Trigger detected!
    }
}
```

### Trigger Generation
```cpp
dsp::PulseGenerator pulseGenerator;

void process(const ProcessArgs& args) override {
    // Somewhere in your logic:
    pulseGenerator.trigger(1e-3f);  // 1ms pulse
    
    // In output section:
    outputs[TRIG_OUTPUT].setVoltage(pulseGenerator.process(args.sampleTime) ? 10.f : 0.f);
}
```

### Clock/Reset Timing Issue

**IMPORTANT**: Each cable introduces 1-sample delay. Modules with CLOCK and RESET inputs should ignore CLOCK triggers for 1ms after RESET:

```cpp
dsp::Timer resetTimer;

void process(const ProcessArgs& args) override {
    if (resetTrigger.process(inputs[RESET_INPUT].getVoltage())) {
        // Handle reset
        resetTimer.reset();
    }
    
    resetTimer.process(args.sampleTime);
    
    // Only process clock if >1ms since reset
    if (resetTimer.time > 1e-3f) {
        if (clockTrigger.process(inputs[CLOCK_INPUT].getVoltage())) {
            // Handle clock
        }
    }
}
```

### Pitch CV Reference
```cpp
// C4 frequency constant
float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);  // pitch in volts

// dsp::FREQ_C4 = 261.6256f Hz
```

---

## 7. Polyphony Support

### Maximum Channels
VCV Rack supports up to **16 polyphonic channels** per cable.

### Basic Polyphonic Module

```cpp
void process(const ProcessArgs& args) override {
    // Get channel count from "primary" input
    int channels = std::max(1, inputs[PITCH_INPUT].getChannels());
    
    for (int c = 0; c < channels; c++) {
        // Use getPolyVoltage for proper mono-to-poly handling
        float pitch = params[PITCH_PARAM].getValue();
        pitch += inputs[PITCH_INPUT].getPolyVoltage(c);
        
        // Process each channel...
        float output = processChannel(c, pitch, args.sampleTime);
        
        outputs[AUDIO_OUTPUT].setVoltage(output, c);
    }
    
    // IMPORTANT: Set the output channel count!
    outputs[AUDIO_OUTPUT].setChannels(channels);
}
```

### Polyphony Rules

1. **Primary Input**: Use the channel count of your main input (V/OCT for VCOs, audio for filters, gate for envelopes)

2. **Secondary Inputs**: 
   - If mono (`M=1`): Apply to all channels
   - If channels ≥ N: Use respective channel
   - Use `getPolyVoltage(c)` which handles this automatically

3. **Monophonic Modules Handling Poly Input**:
   - **Audio inputs**: Sum all channels with `getVoltageSum()`
   - **CV inputs**: Use first channel with `getVoltage()`

4. **Add "Polyphonic" tag** to your manifest if supporting polyphony

### State Arrays for Polyphony
```cpp
// Instead of:
float phase = 0.f;

// Use:
float phase[16] = {};

// Or for complex state:
struct Engine {
    float phase = 0.f;
    float lastPitch = 0.f;
    // ... more state
};
Engine engines[16];
```

---

## 8. DSP Best Practices

### SIMD Optimization

VCV Rack uses SSE4.2 (4-element vectors). Use Rack's SIMD library:

```cpp
#include <rack.hpp>
using namespace rack;

// Instead of processing 1 channel:
for (int c = 0; c < channels; c++) {
    // slow scalar code
}

// Process 4 channels at once:
for (int c = 0; c < channels; c += 4) {
    simd::float_4 pitch = simd::float_4::load(&pitchArray[c]);
    simd::float_4 freq = dsp::FREQ_C4 * simd::pow(2.f, pitch);
    // ... SIMD operations
}
```

### Handling NaN and Infinity

**MUST CHECK** for invalid values in potentially unstable algorithms:
```cpp
float out = myUnstableFilter.process(in);
outputs[AUDIO_OUTPUT].setVoltage(std::isfinite(out) ? out : 0.f);
```

### Available DSP Headers
- `dsp/common.hpp` - Basic math
- `dsp/filter.hpp` - Filters (RC, IIR, etc.)
- `dsp/fft.hpp` - Fast Fourier Transform
- `dsp/minblep.hpp` - Band-limited waveforms
- `dsp/digital.hpp` - Triggers, gates, timers
- `dsp/window.hpp` - Window functions
- `dsp/approx.hpp` - Fast approximations
- `simd/functions.hpp` - SIMD math

### Font and Image Loading (DAW Plugin Compatible)

**CRITICAL**: Don't store Font/Image in constructor. Load each frame:

```cpp
// WRONG - breaks in Rack Pro DAW plugin:
struct MyWidget : Widget {
    std::shared_ptr<Font> font;
    MyWidget() {
        font = APP->window->loadFont(fontPath);  // DON'T DO THIS
    }
};

// CORRECT:
struct MyWidget : Widget {
    std::string fontPath;
    
    MyWidget() {
        fontPath = asset::plugin(pluginInstance, "res/MyFont.ttf");
    }
    
    void draw(const DrawArgs& args) override {
        std::shared_ptr<Font> font = APP->window->loadFont(fontPath);
        if (font) {
            nvgFontFaceId(args.vg, font->handle);
            // ... draw text
        }
    }
};
```

### Bypass Routes
```cpp
MyModule() {
    // ... config ...
    
    // Configure bypass: when bypassed, route input directly to output
    configBypass(AUDIO_INPUT, AUDIO_OUTPUT);
}
```

---

## 9. Licensing Requirements

### License Options

| Plugin Type | License Required | Component Library |
|-------------|------------------|-------------------|
| **Open-source free** | GPLv3+ or any open-source | Allowed with attribution |
| **Closed-source free** | Any (Non-Commercial Exception) | Allowed with attribution |
| **Commercial** | VCV Commercial License | Requires license from VCV |

### Non-Commercial Plugin License Exception
Free plugins (regardless of source availability) may use any license under the VCV Rack Non-Commercial Plugin License Exception. This means:
- ✅ MIT, BSD, CC0 licensed open-source plugins
- ✅ Closed-source freeware
- ✅ Donationware (donation not required)

### Commercial Plugin Requirements
- Contact support@vcvrack.com **early** in development
- Royalty licensing required
- Component Library graphics require separate permission
- Recommended to sell through VCV Library

### Component Library Graphics
The Grayscale Component Library (`componentLibrary.hpp` widgets) is licensed **CC BY-NC 4.0**:
- ✅ Free for non-commercial plugins
- ❌ Requires VCV commercial license for paid plugins
- Must give appropriate credit

### VCV Logo/Name Usage
- ❌ Cannot use "VCV" name for unofficial products
- ✅ May use "for VCV Rack" in promotion
- ❌ Cannot use VCV logo in your plugin

### plugin.json License Field
```json
{
  "license": "GPL-3.0-or-later"
}
```

Use SPDX license identifiers: https://spdx.org/licenses/

---

## 10. Building & Distribution

### Build Commands

```bash
# Build plugin
make

# Build with parallel jobs (faster)
make -j8

# Clean build artifacts
make clean

# Create distributable package
make dist

# Install to local Rack
make install
```

### Output Files
- `plugin.dylib` - The compiled plugin (macOS)
- `dist/MyPlugin-2.x.x-mac-arm64.vcvplugin` - Distribution package

### Development Mode
Launch Rack in dev mode to test your plugin:
```bash
/Applications/VCV\ Rack\ 2\ Free.app/Contents/MacOS/Rack -d
```

This:
- Uses current directory for system/user folders
- Logs to terminal (stderr)
- Disables Library menu (prevents overwriting)

### Cross-Platform Building

For universal distribution, use the **VCV Rack Plugin Toolchain**:

```bash
git clone https://github.com/VCVRack/rack-plugin-toolchain.git
cd rack-plugin-toolchain

# Build with Docker (recommended)
docker build -t rack-plugin-toolchain .

# Build your plugin for all platforms
docker run --rm -v /path/to/MyPlugin:/src rack-plugin-toolchain make -C /src dist
```

---

## 11. Submitting to VCV Library

### Preparation Checklist

- [ ] Version in `plugin.json` starts with `2.` (e.g., `2.0.0`)
- [ ] All text in SVGs converted to paths
- [ ] LICENSE.txt file in root folder
- [ ] README.md or documentation
- [ ] Manifest fields complete and accurate
- [ ] Plugin builds without warnings
- [ ] Tested in VCV Rack

### Submission Process

1. **Create GitHub repository** for your plugin source code

2. **Create issue** in https://github.com/VCVRack/library/issues
   - Title: Your plugin slug (not name)
   - Body: Link to source repository

3. **Wait for maintainer** to:
   - Review your submission
   - Build your plugin
   - Add to Library

4. **For updates**: 
   - Increment version in `plugin.json`
   - Push commit
   - Comment in your issue with commit hash

### Commercial Plugins
Email contact@vcvrack.com directly instead of using GitHub issues.

---

## 12. Common Pitfalls & Must-Nots

### ❌ MUST NOT DO

1. **Don't use text in SVGs** - Always convert to paths
2. **Don't store fonts/images in constructors** - Load each frame
3. **Don't output NaN/Inf** - Always check with `std::isfinite()`
4. **Don't forget to set output channels** - `output.setChannels(n)`
5. **Don't clone hardware without permission** - Get explicit permission
6. **Don't use "VCV" in plugin name** - Trademark violation
7. **Don't ignore clock/reset timing** - Handle 1-sample cable delay
8. **Don't use CSS in SVGs** - Not supported
9. **Don't use complex gradients** - Only simple 2-color linear
10. **Don't block the DSP thread** - No file I/O in `process()`

### ⚠️ COMMON MISTAKES

1. **Forgetting to export model** in `plugin.hpp` and `plugin.cpp`
2. **Wrong voltage standards** - Audio ±5V, CV 0-10V or ±5V
3. **Not handling mono-to-poly** - Use `getPolyVoltage()`
4. **Parameter ranges wrong** - Check min/max/default
5. **Missing "Polyphonic" tag** for poly modules
6. **Version mismatch** - Plugin major version must match Rack
7. **Spaces in path** - Breaks Makefile build system
8. **Forgetting `extern` declaration** - Module won't load

### ✅ MUST DO

1. **Convert all text to paths** in SVG panels
2. **Check output for finite values** in unstable algorithms
3. **Set output channel count** for polyphonic outputs
4. **Use Schmitt trigger** for trigger inputs (0.1V low, 1V high)
5. **Generate 1ms, 10V triggers** for trigger outputs
6. **Handle clock/reset timing** with 1ms ignore window
7. **Add appropriate tags** to manifest
8. **Test with `make clean && make`** before distribution
9. **Version with `2.x.x`** for Rack 2 compatibility

---

## 13. Quick Reference

### Common Component Library Widgets

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
// where T = RedLight, GreenLight, BlueLight, YellowLight, WhiteLight
// or GreenRedLight, RedGreenBlueLight

// Screws
ScrewSilver, ScrewBlack
```

### Useful Macros & Constants

```cpp
RACK_GRID_WIDTH   // 15 (1 HP in pixels)
RACK_GRID_HEIGHT  // 380 (panel height in pixels)
mm2px(Vec(x, y))  // Convert mm to pixels
dsp::FREQ_C4      // 261.6256f Hz
M_PI              // 3.14159...
```

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

### Makefile Variables

```makefile
# In your plugin's Makefile
SLUG = MyPlugin
VERSION = 2.0.0

# Add compiler flags
FLAGS += -DSOME_DEFINE

# Add source files
SOURCES += src/extra.cpp

# Link external libraries
LDFLAGS += -lmylibrary
```

---

## Resources

- **Manual**: https://vcvrack.com/manual/
- **API Reference**: https://vcvrack.com/docs-v2/
- **Community Forum**: https://community.vcvrack.com/
- **GitHub**: https://github.com/VCVRack/Rack
- **Plugin Toolchain**: https://github.com/VCVRack/rack-plugin-toolchain
- **Library Database**: https://github.com/VCVRack/library

---

*This guide is based on official VCV Rack documentation and community best practices as of January 2026. Always refer to the official documentation for the most current information.*
