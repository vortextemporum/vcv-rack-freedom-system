# /test

**Purpose:** Validate plugins through automated tests, build validation, or manual DAW testing.

---

## Syntax

```bash
/test                           # Interactive menu
/test [PluginName]              # Select plugin, choose test method
/test [PluginName] automated    # Run automated test suite
/test [PluginName] build        # Build Release + run pluginval
/test [PluginName] manual       # Show DAW testing checklist
```

## What It Does

Provides three validation approaches to verify plugin stability, correctness, and compatibility. Choose based on what you need to test and what's available.

## Three Test Methods

### 1. Automated Stability Tests

**Requirements:** `plugins/[Plugin]/Tests/` directory must exist

**What it tests:**
- Parameter stability (all combinations, edge cases)
- State save/restore (preset corruption)
- Processing stability (buffer sizes, sample rates)
- Thread safety (concurrent access)
- Edge case handling (silence, extremes, denormals)

**Duration:** ~2 minutes

**Output:**
```
✓ All 47 tests passed (1.8s)

Plugin is stable for basic scenarios.
Still recommended: Manual DAW testing for real-world usage.
```

**If tests fail:**
```
❌ 3 tests failed:
  - testParameterStability: Crash at extreme values
  - testStateSaveRestore: Preset data mismatch
  - testProcessingStability: Output explosion at 96kHz

Options:
1. Investigate failures (triggers deep-research)
2. Show me the test code
3. Show full test output
4. I'll fix it manually
```

### 2. Build and Validate

**Requirements:** None (always available)

**What it does:**
1. Build Release mode (VST3 + AU)
2. Run pluginval with strict settings (level 10)
3. Install to system folders
4. Clear DAW caches

**Duration:** ~5-10 minutes

**Output:**
```
✓ Build successful (Release mode)
✓ Pluginval passed all tests
✓ Installed to system folders

Next steps:
- Test manually in DAW
- /test [Plugin] automated if test suite exists
```

**If build fails:**
```
Build error at [stage]:
[Error output with context]

Options:
1. Investigate (triggers deep-research)
2. Show me the code
3. Show full output
4. I'll fix it manually
```

**If pluginval fails:**
```
Pluginval failed 2 tests:
- Parameter persistence: Values not restored
- State restoration: Corrupted preset data

Options:
1. Investigate failures
2. Show full pluginval report
3. Continue anyway (skip validation)
```

### 3. Manual DAW Testing

**Requirements:** None

**What it does:** Displays comprehensive testing checklist

**Checklist includes:**
```
LOAD & INITIALIZE
[ ] Opens in Ableton Live without errors
[ ] Opens in Logic Pro without errors
[ ] UI displays correctly (no visual glitches)
[ ] All parameters visible and responsive

AUDIO PROCESSING
[ ] Processes audio (input → modified output)
[ ] No pops/clicks during playback
[ ] Bypass works correctly (input = output)
[ ] No DC offset or silence issues

PARAMETER TESTING
[ ] All knobs/sliders respond smoothly
[ ] Parameter automation works
[ ] Extreme values don't crash (0%, 100%)
[ ] Values persist after reopening project

STATE MANAGEMENT
[ ] Preset save/recall works
[ ] Project save/reopen preserves state
[ ] No settings lost when closing GUI

PERFORMANCE
[ ] CPU usage reasonable (<10% single instance)
[ ] No audio dropouts at 128 buffer size
[ ] Handles multiple instances well

COMPATIBILITY
[ ] Works in 44.1kHz and 96kHz projects
[ ] Handles mono and stereo tracks
[ ] VST3 and AU formats both work

STRESS TESTING
[ ] Survives rapid parameter changes
[ ] Handles hot-swapping (enable/disable)
[ ] No memory leaks after extended use

Found issues? Use /improve [Plugin] to fix them.
```

## How It Works

### Without Plugin Name

Lists available plugins with test status:

```
Which plugin would you like to test?

1. MicroGlitch v1.2.0 - Has automated tests ✓
2. ClapPlugin808 v1.0.0 - Build validation only
3. SimpleDelay v0.9.0 - Has automated tests ✓

Reply with number:
```

Indicates which plugins have test suites.

### With Plugin, No Mode

Presents test method options:

```
How would you like to test MicroGlitch?

1. Automated stability tests (2 min)
   Run test suite - catches crashes, explosions, broken params

2. Build and validate (5-10 min)
   Compile Release build + run pluginval

3. Manual DAW testing (guidance)
   Show testing checklist for real-world validation
```

Options adapt based on what's available (automated only shown if Tests/ exists).

### With Plugin and Mode

Executes test directly:

```bash
/test MicroGlitch automated
# → Runs automated tests immediately

/test MicroGlitch build
# → Builds Release + runs pluginval

/test MicroGlitch manual
# → Shows checklist
```

## Auto-Invoked During Workflow

**plugin-workflow auto-invokes testing:**
- After Stage 4 (DSP) completion
- After Stage 5 (GUI) completion

If tests fail, workflow stops until fixed.

## Routes To

**Automated tests:** `plugin-testing` skill

**Build validation:** `build-automation` skill

**Manual testing:** Displays checklist directly (no skill)

## After Testing

**Automated tests pass:**
```
Next steps:
- /test [Plugin] manual for DAW checklist
- /improve [Plugin] if you find issues
```

**Build+validate passes:**
```
Next steps:
- Test manually in DAW
- /test [Plugin] automated if test suite exists
```

**Manual testing complete:**
```
Complete the checklist above.
If you find bugs: /improve [Plugin] to fix them.
```

## Examples

**Example 1: Interactive selection**
```
User: /test
Assistant: [Lists 3 plugins]
User: [Selects MicroGlitch]
Assistant: How would you like to test?
          1. Automated (2 min)
          2. Build and validate (5-10 min)
          3. Manual DAW testing
User: [Selects 1]
Assistant: [Runs automated tests]
          ✓ All 47 tests passed
```

**Example 2: Direct automated test**
```
User: /test MicroGlitch automated
Assistant: [Runs test suite immediately]
          ✓ All tests passed (1.8s)
```

**Example 3: Build validation**
```
User: /test ClapPlugin808 build
Assistant: [Builds Release mode]
          [Runs pluginval]
          ✓ Build successful
          ✓ Pluginval passed
          ✓ Installed to system
```

**Example 4: Plugin without tests**
```
User: /test ClapPlugin808 automated
Assistant: ClapPlugin808 doesn't have automated tests yet.

          Available options:
          1. Build and validate
          2. Manual DAW testing
```

**Example 5: Manual checklist**
```
User: /test SimpleDelay manual
Assistant: [Displays full testing checklist]

          Manual DAW Testing Checklist for SimpleDelay:
          [Complete checklist shown]

          Found issues? Use /improve SimpleDelay to fix them.
```

## Natural Language Equivalent

These phrases work the same:
- "Test MicroGlitch"
- "Run tests for MicroGlitch"
- "Validate MicroGlitch"

## Related Commands

- `/improve` - Fix bugs found during testing
- `/continue` - Resume development if tests fail
- `/implement` - Testing happens automatically at stage boundaries

## When To Use

**Use /test when:**
- After implementing changes
- Before releasing to users
- Checking plugin stability
- Debugging issues
- Verifying fix works

**Auto-invoked when:**
- Stage 4 (DSP) completes
- Stage 5 (GUI) completes
- Part of plugin-workflow validation

## Tips

**Start with automated:** Fastest way to catch obvious bugs (if Tests/ exists).

**Build validation is thorough:** Pluginval catches format compliance issues.

**Manual testing is essential:** Real DAW usage reveals issues tests miss.

**Test after every change:** Catch regressions early.

**Use all three methods:** Each catches different types of issues.

## Common Issues

**"Tests fail with crash"**
Investigate with Option 1 (triggers deep-research).

**"Pluginval fails validation"**
Often state management or parameter issues. Check pluginval report.

**"Plugin works in tests but not DAW"**
Manual testing reveals real-world issues. Use checklist thoroughly.

**"No automated tests available"**
Use build validation and manual testing. Consider adding tests later.

## Output Quality

After passing all three test methods:
- ✅ No crashes or explosions
- ✅ Parameters work correctly
- ✅ State saves/restores properly
- ✅ Meets plugin format standards
- ✅ Works in real DAW environments
- ✅ Ready for production use
