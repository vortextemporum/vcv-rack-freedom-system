# juce-foundation

**Purpose:** Core JUCE development rules, API verification, and best practices enforcement for real-time audio safety and professional quality.

---

## What It Does

Provides foundational knowledge and constraints for JUCE plugin development. Acts as the "JUCE expert" that all other skills consult for API correctness and best practices.

## When Invoked

**Automatically consulted by:**
- `plugin-workflow` (all stages)
- `plugin-improve` (when modifying code)
- `deep-research` (for JUCE-specific questions)

**Rarely invoked directly** (other skills use it internally)

## Core Responsibilities

### 1. API Verification

**Before using JUCE APIs:**
- Verifies class/method exists in target JUCE version
- Checks correct namespace (juce::, juce::dsp::, etc.)
- Validates parameter types and signatures
- Confirms availability on macOS

**Example check:**
```cpp
juce::WebSliderRelay relay { "gain" };  // Is this valid?

✓ Verified: juce::WebSliderRelay
  Available: JUCE 8.0.9+
  Requires: JUCE_WEB_BROWSER=1
  Namespace: juce::
```

### 2. Real-Time Audio Safety

**Enforces rules for processBlock():**

**NEVER allowed in audio thread:**
- Memory allocation (`new`, `malloc`)
- File I/O operations
- Network calls
- Lock-based synchronization (std::mutex)
- System calls
- Logging (except DBG in debug builds)

**ALWAYS required:**
- Lock-free parameter access
- Preallocated buffers
- Bounded execution time
- No exceptions thrown

**Safe patterns:**
```cpp
// ✓ SAFE: Atomic parameter access
float gainValue = gainParameter->load();

// ✓ SAFE: Preallocated buffer
delayBuffer.copyFrom(channel, writePos, buffer, channel, 0, numSamples);

// ✓ SAFE: Lock-free queue
fifo.push(parameterChange);
```

**Unsafe patterns:**
```cpp
// ✗ UNSAFE: Memory allocation
auto* tempBuffer = new float[bufferSize];

// ✗ UNSAFE: File I/O
File::getCurrentWorkingDirectory().getChildFile("log.txt").appendText(message);

// ✗ UNSAFE: Mutex lock
std::lock_guard<std::mutex> lock(paramMutex);
```

### 3. Parameter Management Rules

**Parameter naming:**
- Use camelCase for IDs: `delayTime`, `feedbackAmount`
- Max 32 characters
- Must be valid C++ identifier
- No spaces or special characters

**Parameter ranges:**
- Use `juce::NormalisableRange` for non-linear scaling
- Specify step size for discrete values
- Set sensible defaults (not 0 unless appropriate)
- Include units in parameter name: "Delay Time (ms)"

**Parameter creation:**
```cpp
layout.add(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID { "delayTime", 1 },  // ID + version
    "Delay Time",                           // Display name
    juce::NormalisableRange<float>(0.0f, 2000.0f, 1.0f),
    500.0f,                                 // Default
    "ms"                                    // Unit suffix
));
```

### 4. DSP Constraints

**Buffer processing:**
- Process in place when possible (avoid copies)
- Use JUCE's buffer utilities (avoid raw loops)
- Handle all channel configurations
- Support variable buffer sizes

**Sample rate handling:**
- Always call `prepareToPlay()` before processing
- Recalculate coefficients on sample rate change
- Use `getSampleRate()`, never hardcode

**Denormal handling:**
```cpp
// ✓ GOOD: JUCE handles denormals automatically
juce::FloatVectorOperations::clear(buffer, numSamples);

// ✓ GOOD: Force denormals to zero
if (std::abs(value) < 1e-8f)
    value = 0.0f;
```

### 5. UI Thread Safety

**UI can access:**
- Parameter values (via APVTS or atomic reads)
- Non-realtime data structures
- File operations
- Graphics rendering

**UI must NEVER:**
- Directly modify audio state
- Access audio buffers
- Call processBlock()

**Safe UI → Audio communication:**
```cpp
// ✓ SAFE: Parameter change
gainParam->setValueNotifyingHost(newValue);

// ✓ SAFE: Atomic flag
bypassFlag.store(true, std::memory_order_release);

// ✓ SAFE: Lock-free FIFO
messageQueue.push(UIMessage { MessageType::Bypass, true });
```

### 6. WebView-Specific Rules

**For WebView UIs (JUCE 8.0.9+):**

**Required setup:**
```cpp
juce_add_plugin([PluginName]
    NEEDS_WEB_BROWSER TRUE
    NEEDS_WEBVIEW2 TRUE
)

target_compile_definitions([PluginName]
    PUBLIC
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
)
```

**Parameter binding:**
- One `juce::WebSliderRelay` per parameter
- Register with `.withOptionsFrom(relay)`
- One `juce::WebSliderParameterAttachment` per parameter
- Initialize relays BEFORE browser component

**Resource provider:**
- All files in `ui/public/` embedded as zip
- Correct MIME types critical (.js → application/javascript)
- Path mapping: "/" → "public/index.html"

### 7. Platform Considerations

**macOS-specific:**
- AU format requires PRODUCT_NAME
- Plugins install to `~/Library/Audio/Plug-Ins/`
- Use case-insensitive file paths
- Code signing required for distribution

**Build system:**
- CMake 3.15+ required
- JUCE modules as subdirectory or FetchContent
- juce_add_plugin() for VST3/AU/Standalone
- Binary resources via juce_add_binary_data()

## Knowledge Database

### Common JUCE Classes

**Audio Processing:**
- `juce::AudioProcessor` - Base plugin class
- `juce::AudioProcessorEditor` - Base UI class
- `juce::AudioProcessorValueTreeState` (APVTS) - Parameter management
- `juce::dsp::*` - DSP building blocks

**Parameters:**
- `juce::AudioParameterFloat` - Continuous parameter
- `juce::AudioParameterBool` - Toggle parameter
- `juce::AudioParameterChoice` - Discrete selection
- `juce::NormalisableRange` - Non-linear scaling

**DSP:**
- `juce::dsp::ProcessorChain` - Sequential processing
- `juce::dsp::StateVariableTPTFilter` - Filters
- `juce::dsp::Gain` - Gain processing
- `juce::dsp::Reverb` - Reverb effect

**WebView (8.0.9+):**
- `juce::WebBrowserComponent` - HTML renderer
- `juce::WebSliderRelay` - Parameter relay
- `juce::WebSliderParameterAttachment` - Binding

### API Version Requirements

**JUCE 8.0.9+ (current system requirement):**
- WebView support (WebBrowserComponent)
- WebSliderRelay for parameter binding
- Modern C++17 features

**Breaking changes from JUCE 7:**
- ParameterID now requires version number
- WebView APIs new in 8.x

## Consulting Protocol

**When other skills need JUCE guidance:**

1. **API question:** Verify in JUCE docs (Context7)
2. **Pattern question:** Check best practices
3. **Safety question:** Apply real-time rules
4. **Platform question:** Check macOS constraints

**Example consultation:**

**plugin-workflow asks:** "Is this parameter definition correct?"

```cpp
layout.add(std::make_unique<juce::AudioParameterFloat>(
    "gain", "Gain", 0.0f, 2.0f, 1.0f
));
```

**juce-foundation responds:**
```
✗ Incorrect for JUCE 8+

Issue: Missing ParameterID version number

Correct:
layout.add(std::make_unique<juce::AudioParameterFloat>(
    juce::ParameterID { "gain", 1 },  // ← Add version
    "Gain",
    juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
    1.0f
));

Also: Add step size (0.01f) for smoother parameter control
```

## Quality Standards

**Code must:**
- ✅ Compile without warnings
- ✅ Use JUCE idioms (not raw C++)
- ✅ Follow real-time safety rules
- ✅ Handle all sample rates
- ✅ Support mono, stereo, and multichannel
- ✅ Process variable buffer sizes
- ✅ Initialize all members
- ✅ Clean up resources in destructor

**Code should:**
- Use JUCE's buffer utilities
- Prefer juce::dsp classes over manual DSP
- Document non-obvious decisions
- Avoid premature optimization
- Keep processBlock() simple

## Integration

**Used by:**
- All skills that generate/modify JUCE code
- Research skills for API verification
- Build system for configuration

**Provides:**
- API correctness
- Best practices
- Safety enforcement
- Platform knowledge

**Does NOT:**
- Write code directly (other skills do that)
- Make creative decisions (that's ideation)
- Decide architecture (that's planning)

## Reference Mode

This skill operates mostly in "reference mode"—other skills consult it, it doesn't act independently.

**Think of it as:** The JUCE documentation and best practices, distilled and queryable.

## Success Criteria

**juce-foundation succeeds when:**
- Generated code compiles without warnings
- No real-time violations in processBlock()
- API usage matches current JUCE version
- Parameters follow naming conventions
- Platform requirements met
- WebView integration correct (when used)

**It fails when:**
- Code uses deprecated APIs
- Real-time rules violated
- Wrong JUCE version assumptions
- Platform-specific code breaks

## Best Practices

**For skills using juce-foundation:**
1. Consult BEFORE generating JUCE code
2. Verify API exists in target version
3. Ask about safety if unsure
4. Check platform constraints
5. Validate generated code against rules

**For humans:**
- Trust the real-time rules (they prevent crashes)
- Use JUCE idioms (they're optimized)
- Don't fight the framework (JUCE way is usually best)
- Update JUCE regularly (bug fixes and features)
