# Genesis - Implementation Plan

**Date:** 2026-01-13
**Complexity Score:** 5.0 (Complex - capped from 9.8)
**Strategy:** Phase-based implementation

---

## Complexity Factors

- **Parameters:** 9 parameters (9/5 = 1.8 points) = 1.8
- **Algorithms:** 8 DSP components = 8
  - Waveform Generator (6 waveforms + PolyBLEP)
  - Bit Depth Reduction
  - Sample Rate Reduction
  - 2-Operator FM Synthesis
  - Hard Sync
  - V/Oct Pitch Tracking
  - Polyphonic Voice Management
  - Output Level Control
- **Features:** 0 points
  - No feedback loops, FFT, multiband, modulation systems, or MIDI
- **Total:** 1.8 + 8 + 0 = 9.8 (capped at 5.0)

**Complexity tier:** 4 (Synthesizer with polyphonic oscillators and modulation)
**Research depth:** MODERATE

---

## Stages

- Stage 0: Research ✓
- Stage 1: Foundation ← Next
- Stage 2: Shell (GenesisMono)
- Stage 3: DSP (GenesisMono) - 4 phases
- Stage 4: Testing & Validation (GenesisMono)
- Stage 2: Shell (GenesisPoly)
- Stage 3: DSP (GenesisPoly) - 2 phases (reuse GenesisMono code)
- Stage 4: Testing & Validation (GenesisPoly)

---

## Complex Implementation (Score = 5.0)

### Stage 2: Shell (GenesisMono)

**Goal:** Create build system and module registration for monophonic variant

**Components:**
- Makefile setup with RACK_DIR
- plugin.json manifest (GenesisMono module entry)
- plugin.hpp/cpp registration
- SVG panel (8 HP width)
- Module/ModuleWidget skeleton (no DSP yet)

**Test Criteria:**
- [ ] `make` builds without errors
- [ ] GenesisMono appears in VCV Rack browser
- [ ] Module loads without crash
- [ ] Panel renders correctly (8 HP, all components visible)

---

### Stage 3: DSP (GenesisMono) Phases

#### Phase 3.1: Basic Oscillator (Sine/Triangle)

**Goal:** Implement core oscillator with V/Oct tracking and simple waveforms

**Components:**
- V/Oct pitch calculation (FREQ_PARAM + FINE_PARAM + VOCT_INPUT)
- Phase accumulation
- Sine waveform generation
- Triangle waveform generation
- WAVE_PARAM selection (modes 0-1 only)
- Output level control (LEVEL_PARAM)
- Activity light (ACTIVE_LIGHT)

**Test Criteria:**
- [ ] Module loads in VCV Rack without crash
- [ ] FREQ_PARAM changes pitch (±4 octaves from C4)
- [ ] FINE_PARAM tunes pitch (±50 cents)
- [ ] VOCT_INPUT tracks 1V/Oct (test with keyboard module)
- [ ] Sine wave outputs clean tone (no aliasing audible)
- [ ] Triangle wave outputs clean tone
- [ ] LEVEL_PARAM scales output (0% = silent, 100% = ±5V)
- [ ] Activity light illuminates when audio present

---

#### Phase 3.2: Anti-Aliasing (Saw/Square/Pulse/Noise)

**Goal:** Add remaining waveforms with PolyBLEP anti-aliasing

**Components:**
- Saw waveform with PolyBLEP correction
- Square waveform with PolyBLEP correction
- Pulse waveform with PolyBLEP correction
- PULSE_WIDTH_PARAM for pulse width (0.1-0.9)
- Noise waveform (white noise generator)
- WAVE_PARAM selection (modes 0-5 complete)

**Test Criteria:**
- [ ] Saw wave outputs without audible aliasing (test at high pitch)
- [ ] Square wave outputs without audible aliasing
- [ ] Pulse wave changes timbre with PULSE_WIDTH_PARAM
- [ ] Noise outputs white noise (random values)
- [ ] WAVE_PARAM switches between all 6 waveforms
- [ ] Spectrum analyzer shows minimal aliasing artifacts (<-60dB)
- [ ] No clicks when switching waveforms

---

#### Phase 3.3: FM Synthesis & Hard Sync

**Goal:** Add 2-operator FM synthesis and hard sync

**Components:**
- FM modulator oscillator (ratio-based frequency)
- FM_RATIO_PARAM (0.5:1 to 8:1 modulator:carrier ratio)
- FM_AMOUNT_PARAM (0.0-1.0 modulation index)
- FM_INPUT CV modulation
- Phase modulation (carrier phase offset by modulator output)
- Hard sync (SYNC_INPUT with Schmitt trigger)
- Phase reset on sync rising edge

**Test Criteria:**
- [ ] FM_AMOUNT_PARAM = 0: Clean carrier tone (no FM)
- [ ] FM_AMOUNT_PARAM > 0: FM sidebands audible
- [ ] FM_RATIO_PARAM changes FM timbre (integer ratios = harmonic)
- [ ] FM_INPUT CV modulates FM amount
- [ ] SYNC_INPUT resets phase on rising edge (test with clock)
- [ ] Hard sync creates aggressive timbres (test at high sync rates)
- [ ] FM + sync interaction produces complex tones

---

#### Phase 3.4: Bit Crushing & Sample Rate Reduction

**Goal:** Add lo-fi degradation effects

**Components:**
- Bit depth reduction (BIT_DEPTH_PARAM 1-16 bits)
- BITS_INPUT CV modulation
- Sample rate reduction (SAMPLE_RATE_PARAM 0.0-1.0)
- Sample-and-hold counter for downsampling
- Quantization algorithm

**Test Criteria:**
- [ ] BIT_DEPTH_PARAM = 16: No audible degradation
- [ ] BIT_DEPTH_PARAM = 1: Extreme square wave reduction
- [ ] BITS_INPUT CV modulates bit depth (0-10V → 0-16 bits)
- [ ] SAMPLE_RATE_PARAM = 1.0: No aliasing (full rate)
- [ ] SAMPLE_RATE_PARAM = 0.0: Extreme aliasing (~1kHz rate)
- [ ] Bit crushing + sample rate reduction = authentic lo-fi sound
- [ ] No crashes or NaN outputs with extreme settings

---

### Stage 4: Testing & Validation (GenesisMono)

**Goal:** Verify GenesisMono against creative brief and professional references

**Test Criteria:**
- [ ] All parameters work correctly
- [ ] No audible artifacts (except intentional bit crushing/aliasing)
- [ ] CPU usage acceptable (<25% single core)
- [ ] Sounds similar to Sega Genesis YM2612 references
- [ ] Patch: FM bass (square wave, FM ratio 2:1, bit crush 8-bit)
- [ ] Patch: Bell tone (sine wave, FM ratio 3:1, no bit crush)
- [ ] Patch: Chiptune lead (saw wave, hard sync, bit crush 4-bit)

---

### Stage 2: Shell (GenesisPoly)

**Goal:** Create polyphonic variant (reuse GenesisMono DSP)

**Components:**
- GenesisPoly module registration (separate model)
- SVG panel (10 HP width, label "POLY")
- Module/ModuleWidget with same parameters as GenesisMono
- Shared DSP code via common functions

**Test Criteria:**
- [ ] GenesisPoly appears in VCV Rack browser (separate from GenesisMono)
- [ ] Module loads without crash
- [ ] Panel renders correctly (10 HP, "POLY" label visible)

---

### Stage 3: DSP (GenesisPoly) Phases

#### Phase 3.1: Polyphonic Voice Management

**Goal:** Convert monophonic DSP to 16-voice polyphonic

**Components:**
- State arrays: `phase[16]`, `fmPhase[16]`, `syncTrigger[16]`, `sampleHold[16]`, `holdCounter[16]`
- Channel count detection: `channels = std::max(1, inputs[VOCT_INPUT].getChannels())`
- Per-channel processing loop: `for (int c = 0; c < channels; c++)`
- `getPolyVoltage(c)` for CV inputs
- `setChannels(channels)` for output

**Test Criteria:**
- [ ] Monophonic input (1 channel): Single voice output
- [ ] Polyphonic input (4 channels): 4 independent voices
- [ ] Polyphonic input (16 channels): 16 independent voices
- [ ] Each voice tracks pitch independently
- [ ] Hard sync works per-voice (independent triggers)
- [ ] No cross-talk between voices
- [ ] Output channel count matches input

---

#### Phase 3.2: Polyphonic Testing & Optimization

**Goal:** Verify polyphonic performance and optimize if needed

**Components:**
- CPU profiling with 16 voices active
- SIMD optimization (if CPU usage >50%)
- Edge case testing (channel count changes, rapid patch switching)

**Test Criteria:**
- [ ] CPU usage acceptable with 16 voices (<80% single core)
- [ ] No audio glitches when adding/removing voices
- [ ] Polyphonic FM sounds correct (chords with FM)
- [ ] Polyphonic bit crushing sounds correct
- [ ] No crashes with rapid parameter changes

---

### Stage 4: Testing & Validation (GenesisPoly)

**Goal:** Verify GenesisPoly matches creative brief

**Test Criteria:**
- [ ] Polyphonic chords sound correct (test with MIDI-CV)
- [ ] CPU usage acceptable (16 voices active)
- [ ] Patch: FM pad (sine wave, FM ratio 1.5:1, 8 voices)
- [ ] Patch: Bit-crushed arp (saw wave, 4-bit, 4 voices)
- [ ] GenesisMono and GenesisPoly sound identical (monophonic comparison)

---

## Implementation Flow

1. **Stage 1: Foundation** - VCV Rack plugin structure
2. **Stage 2: Shell (GenesisMono)** - Monophonic module skeleton
3. **Stage 3: DSP (GenesisMono)** - 4 phases
   - Phase 3.1: Basic oscillator (sine/triangle)
   - Phase 3.2: Anti-aliasing (saw/square/pulse/noise)
   - Phase 3.3: FM synthesis & hard sync
   - Phase 3.4: Bit crushing & sample rate reduction
4. **Stage 4: Testing (GenesisMono)** - Validation against YM2612 references
5. **Stage 2: Shell (GenesisPoly)** - Polyphonic module skeleton
6. **Stage 3: DSP (GenesisPoly)** - 2 phases
   - Phase 3.1: Polyphonic voice management
   - Phase 3.2: Polyphonic testing & optimization
7. **Stage 4: Testing (GenesisPoly)** - Validation with polyphonic patches

---

## Implementation Notes

### Thread Safety
- VCV Rack handles parameter access atomically (no explicit locks needed)
- No shared state between voices (per-channel arrays)
- Single-threaded audio processing per module (VCV Rack handles parallelism)

### Performance
- **Per-voice CPU usage (estimated):**
  - Waveform generation: ~5-8% (depending on waveform type)
  - FM synthesis: ~10% (2 oscillators + phase modulation)
  - Bit crushing: ~2%
  - Sample rate reduction: ~1%
  - Total: ~18-21% per voice
- **Polyphonic scaling:** 16 voices = ~300-350% single core
  - VCV Rack handles multi-threading across modules
  - Target: <80% single core with 16 voices active
- **Optimization opportunities:**
  - SIMD vectorization (4-voice batches using VCV Rack's `simd::float_4`)
  - Fast approximations (`exp2f` instead of `std::pow`)
  - Lookup tables for waveforms (memory vs. CPU tradeoff)

### Latency
- Zero processing latency (no buffering)
- Real-time compatible (suitable for live performance)
- No latency compensation needed

### Denormal Protection
- VCV Rack enables flush-to-zero mode (handles denormals automatically)
- Phase wrapping prevents denormals: `if (phase >= 1.f) phase -= 1.f`

### Known Challenges

**PolyBLEP Anti-Aliasing:**
- Challenge: Detecting discontinuities in waveforms
- Solution: Reference Bogaudio VCO implementation (open-source)
- Test: Use Bogaudio Analyzer-XL to verify aliasing <-60dB

**FM Synthesis Phase Wrapping:**
- Challenge: Modulated phase can exceed [0, 1] range
- Solution: Use `fmodf(phase, 1.f)` to wrap to [0, 1]
- Test: Verify no NaN outputs with extreme FM amounts

**Polyphonic State Management:**
- Challenge: Managing 16 independent voice states
- Solution: Use C-style arrays (`float phase[16]`)
- Test: Verify no cross-talk between voices

**Bit Crushing + Sample Rate Interaction:**
- Challenge: Combined effect may cause unexpected artifacts
- Solution: Apply bit crushing before sample rate reduction (order matters)
- Test: Verify staircase + aliasing artifacts are intentional

### VCV Rack-Specific Patterns

**Polyphony:**
- Use `getPolyVoltage(c)` for CV inputs (handles mono-to-poly)
- Use `setChannels(channels)` AFTER processing loop (required by VCV Rack)
- Use `std::max(1, inputs[ID].getChannels())` to determine channel count

**Voltage Standards:**
- Audio output: ±5V (10Vpp)
- CV input: ±10V (1V/Oct pitch standard)
- Gates: 0V (off) / +10V (on)

**Schmitt Trigger:**
- Use `dsp::SchmittTrigger` with thresholds 0.1V (low) / 1.0V (high)
- Prevents false triggers on noisy gate signals

**PolyBLEP Algorithm:**
```cpp
float polyBlep(float t, float dt) {
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.f;
    } else if (t > 1.f - dt) {
        t = (t - 1.f) / dt;
        return t * t + t + t + 1.f;
    }
    return 0.f;
}
```

---

## References

**Contract files:**
- Creative brief: `plugins/Genesis/.ideas/creative-brief.md`
- Parameter spec: `plugins/Genesis/.ideas/parameter-spec.md`
- DSP architecture: `plugins/Genesis/.ideas/architecture.md`
- Critical patterns: `troubleshooting/patterns/vcv-rack-critical-patterns.md`

**Similar VCV Rack modules for reference:**
- **Bogaudio VCO** - PolyBLEP anti-aliasing reference (open-source)
- **Venom VCO Lab** - FM + hard sync integration reference
- **Fundamental VCO-1/2** - Gold standard VCV Rack oscillators (MinBLEP reference)
- **21kHz modules** - Hard sync implementation

**Professional references:**
- YM2612 datasheets (FM synthesis ratios and phase modulation)
- Sega Genesis game soundtracks (Streets of Rage, Sonic)
- TAL-Bitcrusher (bit crushing parameter ranges)
- D16 Decimort 2 (sample rate reduction reference)

**Technical resources:**
- VCV Rack Plugin Development Tutorial: https://vcvrack.com/manual/PluginDevelopmentTutorial
- VCV Rack DSP Guide: https://vcvrack.com/manual/DSP
- PolyBLEP paper: "Antialiasing Oscillators in Subtractive Synthesis" (Välimäki & Huovilainen)
