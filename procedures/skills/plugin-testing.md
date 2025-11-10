# plugin-testing

**Purpose:** Automated stability testing for JUCE plugins—catches crashes, explosions, broken parameters, and state bugs in 2 minutes.

---

## What It Does

Runs comprehensive automated tests to verify plugin works correctly before manual DAW testing. Catches catastrophic bugs that would crash DAWs or damage audio.

## When Invoked

**Automatically triggered by:**
- `plugin-workflow` after Stage 4 (DSP complete)
- `plugin-workflow` after Stage 5 (GUI complete)
- `plugin-improve` after implementation

**Manually triggered:**
- "Test [PluginName]"
- `/test [PluginName]`
- "Validate [PluginName]"

## Three Testing Modes

### 1. Automated Testing (Recommended)

**What it tests:**
- Parameter response
- State save/load
- Silent input handling
- Feedback loop detection
- CPU performance

**Duration:** ~2 minutes

**Output:** Pass/fail report with details

**Best for:** Quick validation during development

### 2. Build + Pluginval

**What it tests:**
- Clean Release build
- VST3/AU format compliance
- Host compatibility
- Standard compliance (VST3 SDK tests)

**Duration:** ~5-10 minutes

**Output:** Build logs + pluginval report

**Best for:** Pre-release validation

### 3. Manual DAW Testing

**What it provides:**
- Guided testing checklist
- DAW-specific scenarios
- Edge case verification

**Duration:** ~15-30 minutes

**Output:** Human verification checklist

**Best for:** Final quality assurance

## Automated Test Suite

### Test 1: Parameter Response

**What:** Verifies all parameters control audio

**How:**
1. Load plugin
2. Generate white noise input
3. For each parameter:
   - Set to minimum
   - Process audio, measure output
   - Set to maximum
   - Process audio, measure output
   - Verify output changed

**Pass criteria:** All parameters cause measurable audio change

**Failure example:**
```
✗ Parameter 'mix' does not affect output
  Input: White noise (RMS 0.25)
  mix=0%: Output RMS 0.25
  mix=100%: Output RMS 0.25 (expected change)
```

### Test 2: State Save/Load

**What:** Verifies plugin saves and restores state correctly

**How:**
1. Set all parameters to specific values
2. Save state (getStateInformation)
3. Reset all parameters to defaults
4. Load state (setStateInformation)
5. Verify parameters match saved values

**Pass criteria:** All parameters restore correctly

**Failure example:**
```
✗ State load failed for parameter 'delayTime'
  Saved: 750.0 ms
  Loaded: 500.0 ms (default value)
```

### Test 3: Silent Input Handling

**What:** Verifies plugin doesn't explode or produce noise from silence

**How:**
1. Feed completely silent buffer (all zeros)
2. Process for 10 seconds
3. Verify output is silent or reasonable

**Pass criteria:** Output RMS < 0.001 (near silence)

**Failure examples:**
```
✗ Plugin produces noise from silent input
  Input: Complete silence (0.0)
  Output: RMS 0.15 (loud noise - NaN or uninitialized buffer?)

✗ Plugin produces DC offset
  Input: Complete silence
  Output: DC offset +0.5 (will damage speakers)
```

### Test 4: Feedback Loop Detection

**What:** Ensures plugin doesn't infinitely amplify

**How:**
1. Feed small impulse (0.01 amplitude)
2. Set feedback parameters to maximum
3. Process for 5 seconds
4. Verify output doesn't explode

**Pass criteria:** Output stays below 2.0 (some headroom, but contained)

**Failure example:**
```
✗ Feedback loop detected
  Input: Impulse 0.01
  After 2 seconds: Output > 100.0 (infinity!)
  Feedback parameter likely unbounded
```

### Test 5: CPU Performance

**What:** Measures processing efficiency

**How:**
1. Process 1 second of audio
2. Measure time taken
3. Calculate real-time factor

**Pass criteria:** Real-time factor < 0.1 (uses less than 10% of available time)

**Failure example:**
```
⚠ Performance warning
  Processing 1 second took 0.15 seconds
  Real-time factor: 0.15 (acceptable, but inefficient)

  Ideally: < 0.05 for complex plugins, < 0.01 for simple effects
```

## Build + Pluginval Mode

### Clean Build

**Removes old build files:**
```bash
rm -rf build/plugins/[PluginName]
```

**Rebuilds from scratch:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target [PluginName]_VST3 --parallel
cmake --build build --target [PluginName]_AU --parallel
```

**Verifies no build warnings:**
Any compiler warnings = test fails

### Pluginval Validation

**Runs official VST3/AU validator:**

```bash
pluginval \
  --validate ~/Library/Audio/Plug-Ins/VST3/[ProductName].vst3 \
  --strictness-level 10 \
  --timeout-ms 30000 \
  --verbose
```

**Tests:**
- Plugin scans correctly
- Parameters function properly
- State save/load works
- GUI opens without crashes
- Handles edge cases (sample rate changes, etc.)
- Format compliance (VST3/AU standards)

**Pass criteria:** All pluginval tests pass (no errors)

**Failure example:**
```
Pluginval errors:
- Plugin does not return correct tail length
- Parameter automation is not smooth
- State restore changes parameter values
```

## Manual DAW Testing Checklist

**Provides guided checklist:**

```markdown
# Manual Testing Checklist for [PluginName]

## Setup
- [ ] Open Ableton Live or Logic Pro
- [ ] Create audio track with test signal (white noise or tone)
- [ ] Load plugin on track

## Parameter Testing
- [ ] Move each parameter, verify audio changes
- [ ] Verify parameter values display correctly
- [ ] Test extreme settings (min/max)
- [ ] Verify no clicking/popping on parameter changes

## Automation
- [ ] Draw automation for main parameters
- [ ] Play back automation, verify plugin follows
- [ ] Verify automation displays on GUI
- [ ] Test automation recording (touch mode)

## Preset Recall
- [ ] Load factory preset
- [ ] Verify all parameters change
- [ ] Save custom preset
- [ ] Load custom preset, verify recall

## Project Save/Load
- [ ] Save project with plugin active
- [ ] Close and reopen project
- [ ] Verify plugin state restored correctly
- [ ] Verify no error messages on load

## Edge Cases
- [ ] Change sample rate (44.1 → 48 → 96 kHz)
- [ ] Change buffer size (64 → 512 → 2048)
- [ ] Disable/enable plugin (bypass)
- [ ] Delete and re-add plugin

## Sonic Quality
- [ ] Process test signal, verify output sounds correct
- [ ] Check for unexpected noise or artifacts
- [ ] Verify low CPU usage (check DAW meter)
- [ ] Test with real audio material

## Results
Pass: [ ]  Fail: [ ]

Issues found:
(Describe any problems encountered)
```

## Test Results Format

### All Tests Passed

```
✅ All tests PASSED (5/5)

Parameter response: PASS (5/5 parameters)
State save/load: PASS
Silent input: PASS (output RMS: 0.0)
Feedback test: PASS (max output: 0.85)
CPU performance: PASS (real-time factor: 0.03)

Plugin is stable and ready for DAW testing.
```

### Some Tests Failed

```
⚠️ Tests completed with warnings (4/5 PASS, 1 WARNING)

Parameter response: PASS (5/5 parameters)
State save/load: PASS
Silent input: PASS
Feedback test: PASS
CPU performance: ⚠️ WARNING
  Real-time factor: 0.12 (acceptable but could be optimized)

Plugin is functional but has performance issues.
Consider optimization before distribution.
```

### Critical Failures

```
❌ CRITICAL FAILURES (3/5 PASS, 2 FAIL)

Parameter response: ✗ FAIL
  'mix' parameter does not affect output
State save/load: ✗ FAIL
  'delayTime' not restored correctly
Silent input: PASS
Feedback test: PASS
CPU performance: PASS

DO NOT use in DAW until these issues are fixed.
```

## Integration

**Invoked by:**
- `plugin-workflow` (auto-triggers after DSP/GUI)
- `plugin-improve` (auto-triggers after changes)
- Manual testing requests

**Calls:**
- Build system (for pluginval mode)
- Plugin binary (for automated tests)

**Creates:**
- Test reports in `logs/[PluginName]/test_[timestamp].log`
- Pluginval reports (if applicable)

**Reports to:**
- Console output (real-time results)
- Log files (detailed data)

## When Tests Run

**During workflow:**

**After Stage 4 (DSP):**
- Automated tests (parameter, state, silent, feedback)
- Verifies DSP implementation is stable

**After Stage 5 (GUI):**
- Automated tests again (verify UI didn't break audio)
- Optionally: Build + pluginval

**After plugin-improve:**
- Automated tests (regression check)
- Ensures changes didn't break existing functionality

## Best Practices

**Always run automated tests:** 2 minutes catches 90% of bugs

**Fix failures immediately:** Don't proceed to next stage with failing tests

**Run pluginval before release:** Ensures host compatibility

**Manual testing is final step:** Automated tests don't catch everything

**Keep logs:** Failed test logs help debugging

## Common Failures & Solutions

### Parameter Not Affecting Output

**Cause:** Parameter not used in processBlock()

**Fix:** Use parameter value in DSP code

### State Load Fails

**Cause:** Missing parameters in state save/load

**Fix:** Ensure all parameters in APVTS are saved

### Feedback Loop

**Cause:** Feedback parameter allows values > 1.0

**Fix:** Clamp feedback to [0.0, 0.99] range

### Poor Performance

**Cause:** Expensive calculations in processBlock()

**Fix:** Optimize DSP, use JUCE's optimized classes

## Success Criteria

**Plugin testing succeeds when:**
- ✅ All automated tests pass
- ✅ Pluginval reports no errors (if run)
- ✅ Manual checklist completed without issues

**Ready for:**
- Installation to system folders
- Use in DAWs
- Distribution (after pluginval)
