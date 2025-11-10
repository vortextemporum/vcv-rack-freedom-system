# /implement

**Purpose:** Build new plugins through the complete 7-stage development workflow.

---

## Syntax

```bash
/implement                # Interactive menu to select plugin
/implement [PluginName]   # Build specific plugin
```

## What It Does

Executes the full plugin creation workflow from research through validation. Implements a working, installable plugin based on a creative brief.

## Prerequisites

**Required:**
- Creative brief must exist: `plugins/[PluginName]/.ideas/creative-brief.md`

**Optional but recommended:**
- UI mockup: `plugins/[PluginName]/.ideas/mockups/v[N]-*`

If no creative brief exists, the system will offer to run `/dream` first.

## The 7 Stages

### Stage 0: Research
**Duration:** 15-30 minutes

**What happens:**
- Technical investigation of DSP algorithms
- JUCE API research
- Reference implementation study
- Feasibility analysis

**Output:** `plugins/[PluginName]/research.md`

**Commit:** `feat: [PluginName] Stage 0 - research`

### Stage 1: Planning
**Duration:** 5-10 minutes

**What happens:**
- Complexity score calculation
- Phase breakdown (for complex plugins)
- Implementation strategy

**Output:** `plugins/[PluginName]/plan.md`

**Commit:** `feat: [PluginName] Stage 1 - planning complete`

### Stage 2: Foundation
**Duration:** 10-15 minutes

**What happens:**
- CMakeLists.txt configuration
- Project structure setup
- Basic JUCE boilerplate
- Parameter definitions in APVTS

**Output:** Build system files, basic structure

**Commit:** `feat: [PluginName] Stage 2 - foundation`

### Stage 3: Shell
**Duration:** 5-10 minutes

**What happens:**
- PluginProcessor skeleton
- Empty processBlock()
- State management setup
- Build verification (compiles but does nothing)

**Output:** Compiling plugin shell

**Commit:** `feat: [PluginName] Stage 3 - shell`

### Stage 4: DSP
**Duration:** 30 minutes - 3 hours (depends on complexity)

**What happens:**
- Audio processing implementation
- Algorithm coding
- Parameter handling
- Real-time safety verification

**Complexity handling:**
- Simple plugins: Single-pass implementation
- Complex plugins: Multiple phases with incremental commits

**Output:** Functional audio processing

**Commits:**
- Simple: `feat: [PluginName] Stage 4 - DSP implementation`
- Complex: `feat: [PluginName] Stage 4.1 - core processing`, etc.

**Automated test:** Stability check runs after completion

### Stage 5: GUI
**Duration:** 20-60 minutes (depends on UI complexity)

**What happens:**
- Copy mockup HTML to `ui/public/index.html`
- Copy JUCE JavaScript module
- Implement PluginEditor with WebView
- Parameter bindings (HTML ↔ C++)
- Build and visual verification

**Complexity handling:**
- Simple UIs: Single-pass implementation
- Complex UIs: Multiple phases (layout → feedback → polish)

**Output:** Working WebView interface

**Commits:**
- Simple: `feat: [PluginName] Stage 5 - GUI implementation`
- Complex: `feat: [PluginName] Stage 5.1 - basic layout`, etc.

**Automated test:** Stability check + parameter binding validation

### Stage 6: Validation
**Duration:** 20-40 minutes

**What happens:**
- Pluginval compliance check
- Preset creation
- Manual testing checklist
- Documentation finalization

**Output:** Production-ready plugin

**Commit:** `feat: [PluginName] Stage 6 - validation complete`

## Example Flow

```bash
/implement SimpleGain

Stage 0: Research...
✓ Researched gain algorithm (simple amplitude scaling)
✓ Committed: feat: SimpleGain Stage 0 - research (abc1234)

Continue to Stage 1? [Y/n]

Stage 1: Planning...
✓ Complexity: 1/5 (simple)
✓ Single-pass implementation approach
✓ Committed: feat: SimpleGain Stage 1 - planning complete (def5678)

Continue to Stage 2? [Y/n]

[... continues through all stages ...]

Stage 6: Validation...
✓ Pluginval: PASS
✓ Presets created: 0%, 50%, 100%
✓ Manual testing checklist provided
✓ Committed: feat: SimpleGain Stage 6 - validation complete (xyz9012)

✅ SimpleGain development complete!

Next: ./scripts/build-and-install.sh SimpleGain
```

## Pause & Resume

The workflow can be paused at any stage:

**Pause:** System asks "Continue to Stage [N]?" before each stage

**Resume:** Use `/continue SimpleGain` to pick up where you left off

The `.continue-here.md` file tracks current state.

## Complexity Intelligence

**Simple plugins (score ≤2):**
- Single-pass through each stage
- Minimal commits (one per stage)
- ~2-4 hours total time

**Complex plugins (score ≥3):**
- Phases within DSP and GUI stages
- Multiple commits per stage
- Test criteria for each phase
- ~5-10 hours total time

The system automatically chooses the approach based on Stage 1 analysis.

## When To Use

**Use `/implement` when:**
- You have a complete creative brief
- You're ready to build (not just exploring)
- You want the full workflow with commits
- You want automated validation

**Don't use `/implement` when:**
- No creative brief exists (run `/dream` first)
- Just prototyping an idea (use `/dream` for mockups)
- Modifying existing plugin (use `/improve`)

## What Gets Built

By the end, you have:
- ✅ Compiling VST3 + AU plugins
- ✅ Working audio processing
- ✅ Functional WebView UI
- ✅ Pluginval compliant
- ✅ Factory presets
- ✅ Complete documentation
- ✅ Git history with all stages

## Installation

After `/implement` completes:

```bash
./scripts/build-and-install.sh [PluginName]
```

Builds in Release mode and installs to system plugin folders.

## Related Commands

- `/dream` - Create creative brief first
- `/continue` - Resume interrupted workflow
- `/test` - Run validation suite
- `/improve` - Make changes after completion

## Tips

**Let it run:** The workflow asks for confirmation between stages but is mostly automated.

**Review commits:** Each stage creates a git commit—review the code as you go.

**Test incrementally:** After Stage 4 and Stage 5, automated tests catch issues early.

**Don't skip validation:** Stage 6 ensures your plugin works correctly in real DAWs.
