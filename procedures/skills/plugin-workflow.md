# plugin-workflow

**Purpose:** Complete 7-stage workflow for building professional JUCE audio plugins from research to release.

**Version:** 1.0.0

---

## What It Does

Orchestrates the entire plugin development lifecycle with automated checkpoints, git commits, and context handoff. Takes you from concept to working, validated plugin ready for installation.

## When To Use

**Trigger phrases:**
- "Make me a [plugin]"
- "Create [plugin]"
- "Build [plugin]"

**Preconditions:**
- Plugin directory must not exist OR exist with status 💡 (ideated) in PLUGINS.md
- Plugin must NOT be in development (status 🚧)

## The 7 Stages

### Stage 0: Research
**Goal:** Understand what we're building before writing code

**Actions:**
- Define plugin type and technical approach
- Research professional examples (FabFilter, Waves, etc.)
- Check DSP feasibility with Context7 (juce::dsp modules)
- Research parameter ranges and industry standards
- Check design sync if creative brief and mockup exist

**Output:** `research.md`

**Duration:** 5-10 minutes

### Stage 1: Planning
**Goal:** Analyze complexity and create implementation plan

**Actions:**
- Calculate complexity score (1-5) based on parameters, algorithms, features
- Simple plugins (≤2): Single-pass implementation
- Complex plugins (≥3): Phase-based implementation with test criteria

**Output:** `plan.md` with phased breakdown if complex

**Commit:** `feat: [Plugin] Stage 1 - planning complete`

**Duration:** 2-5 minutes

### Stage 2: Foundation
**Goal:** Create plugin structure that compiles

**Actions:**
- Generate CMakeLists.txt with JUCE configuration
- Create empty PluginProcessor and PluginEditor
- Configure build system
- Verify it compiles (no install yet)

**Output:** CMakeLists.txt, Source/PluginProcessor.{h,cpp}, Source/PluginEditor.{h,cpp}

**Commit:** `feat: [Plugin] Stage 2 - foundation`

**Duration:** 5 minutes

### Stage 3: Shell
**Goal:** Plugin loads in DAW, does nothing yet

**Actions:**
- Basic AudioProcessor setup (channels, processBlock stub)
- Empty editor with placeholder text
- Build and verify it loads in DAW

**Output:** Functioning but empty plugin

**Commit:** `feat: [Plugin] Stage 3 - shell`

**Duration:** 5 minutes

### Stage 4: DSP
**Goal:** Audio processing works, parameters functional

**Actions:**
- Create APVTS (AudioProcessorValueTreeState) with all parameters
- Implement processBlock with DSP algorithms
- For complex plugins: Execute phases (4.1, 4.2, etc.) with incremental commits
- Each phase has specific test criteria and its own commit

**Output:** Working audio processing with parameters

**Testing:** Automated stability tests run after completion

**Commit:**
- Simple: `feat: [Plugin] Stage 4 - DSP`
- Complex: `feat: [Plugin] Stage 4.1 - [phase description]` (per phase)

**Duration:** 15-45 minutes (depending on complexity)

### Stage 5: GUI
**Goal:** Professional UI with working controls

**Actions:**
- Create layout using FlexBox or Grid (never manual setBounds)
- Add rotary sliders, buttons, labels, etc.
- Attach all controls to APVTS parameters
- For complex plugins: Execute phases (5.1, 5.2, etc.) with incremental commits
- Apply visual styling

**Output:** Professional interface with working controls

**Testing:** Automated stability tests run after completion

**Commit:**
- Simple: `feat: [Plugin] Stage 5 - GUI`
- Complex: `feat: [Plugin] Stage 5.1 - [phase description]` (per phase)

**Duration:** 20-60 minutes (depending on complexity)

### Stage 6: Validation & Presets
**Goal:** Ready to install and use

**Actions:**
- Create factory presets (Presets/ directory)
- Run pluginval validation
- Test in DAW (manual or automated)
- Create CHANGELOG.md
- Update PLUGINS.md with status ✅ Working

**Output:** Production-ready plugin

**Commit:** `feat: [Plugin] Stage 6 - validation complete`

**Duration:** 10-20 minutes

---

## Stage Boundary Protocol

At every stage boundary:

1. **Show what was completed**
2. **Run automated tests** (Stages 4 and 5 only)
   - Auto-invokes plugin-testing skill
   - If tests fail: STOP, show results, wait for fixes
   - If tests pass: Continue
3. **Auto-commit** with standardized message
   - Format: `feat: [Plugin] Stage N - [description]`
   - For phased stages: Each phase gets its own commit
4. **Create/update handoff file** (.continue-here.md)
5. **Ask to continue** or pause

---

## Complexity Adaptation

**Simple plugins (score ≤2):**
- Single-pass implementation
- One commit per stage
- Faster workflow

**Complex plugins (score ≥3):**
- Phase-based implementation
- Multiple commits per DSP/GUI stage
- Each phase has test criteria
- plan.md tracks completion with checkmarks

**Complexity score factors:**
- Parameter count (min(count / 5, 2) points)
- Algorithm count (1 point each)
- Special features (1 point each):
  - Feedback loops
  - FFT/frequency domain processing
  - Multiband processing
  - Modulation systems
  - External MIDI control
- Capped at 5 points

---

## Context Handoff System

**Location:** `plugins/[PluginName]/.continue-here.md`

**Created:** After Stage 0 commit

**Updated:** After every stage/phase commit

**Format:**
```yaml
---
plugin: PluginName
stage: N
status: in_progress
last_updated: YYYY-MM-DD HH:MM
---

# Resume Point

## Current Stage: Stage N - [Name]

## Completed So Far
- [Task 1]
- [Task 2]

## Next Steps
1. [Next task]
2. [Following task]

## Context
- Key decisions: [...]
- Files to review: [list]
```

**Purpose:**
- Resume development in fresh chat sessions
- Preserve context across long implementations
- Provides development history

---

## Git Workflow

**Simple plugins:**
```
✓ Stage 5 complete: professional interface
✓ Committed: feat: Shake Stage 5 - GUI (abc1234)

Continue to Stage 6?
```

**Complex plugins:**
```
✓ Phase 4.2 complete: parameter modulation
✓ Committed: feat: ModDelay Stage 4.2 - parameter modulation (def5678)
✓ Updated plan.md with completion timestamp

Continue to Phase 4.3?
```

---

## Available Commands

**During workflow:**
- `"resume automation"` - Continue after manual fixes
- `"skip to stage [N]"` - Jump ahead (if prerequisites met)
- `"restart stage [N]"` - Redo current stage

**Inspection:**
- `"show [Plugin] status"` - Current stage and progress
- `"show handoff"` - Display .continue-here.md

---

## Integration

**Calls these skills:**
- `juce-foundation` - All stages (API verification, constraints)
- `build-automation` - Stages 2-6 (building and installing)
- `plugin-testing` - Stages 4, 5, 6 (automated tests)
- `design-sync` - Stage 0 (if mockup/brief mismatch detected)
- `plugin-lifecycle` - After Stage 6 (installation)

**Called by:**
- User commands ("make me a [plugin]")
- `/implement` slash command
- `plugin-ideation` skill (after creative brief)
- `context-resume` skill (when resuming)

**Creates:**
- `.continue-here.md` (handoff file)
- `research.md` (Stage 0)
- `plan.md` (Stage 1)
- `CHANGELOG.md` (Stage 6)
- `Presets/` directory (Stage 6)

---

## Example Flow

**Simple plugin (single-pass):**
```
Stage 0: Research → 5 min → research.md created
Stage 1: Planning → 2 min → plan.md (score: 1.4, single-pass)
Stage 2: Foundation → 5 min → Compiles successfully
Stage 3: Shell → 5 min → Loads in DAW
Stage 4: DSP → 20 min → Audio processing works → Tests pass
Stage 5: GUI → 30 min → UI complete → Tests pass
Stage 6: Validation → 15 min → Ready to install

Total: ~80 minutes
```

**Complex plugin (phased):**
```
Stage 0: Research → 10 min
Stage 1: Planning → 5 min → plan.md (score: 4.2, 3 DSP phases, 2 GUI phases)
Stage 2: Foundation → 5 min
Stage 3: Shell → 5 min
Stage 4.1: Core processing → 15 min → Commit
Stage 4.2: Modulation → 20 min → Commit
Stage 4.3: Advanced features → 15 min → Commit → Tests pass
Stage 5.1: Layout → 20 min → Commit
Stage 5.2: Visual feedback → 25 min → Commit → Tests pass
Stage 6: Validation → 20 min

Total: ~140 minutes
```

---

## Best Practices

**Let the workflow guide you:**
- Follow stages sequentially
- Don't skip checkpoints
- Trust the automated tests

**Use handoff files:**
- Pause at stage boundaries if needed
- Resume with `/continue [Plugin]`
- Context preserved across sessions

**Respect complexity scores:**
- Don't fight the phase breakdown
- Each phase isolates a concern
- Incremental commits = easier debugging

**When errors occur:**
- Build failures trigger failure protocol
- Choose "investigate" for deep-research
- Fix root cause, not symptoms

---

## Common Issues

**"Can I skip Stage 0/1?"**
No. Research and planning prevent costly mistakes later.

**"My plugin is stuck at Stage X"**
Use `/continue [Plugin]` to resume from checkpoint.

**"Build fails at Stage 2"**
Check CMakeLists.txt format, JUCE path, missing modules.

**"Tests fail at Stage 4/5"**
Investigate failures before continuing. Use deep-research skill.

**"I want to change something from Stage 3"**
Complete current stage, then use `/improve [Plugin]` for changes.

---

## Output Quality

After Stage 6 completion:
- ✅ Plugin compiles in Release mode
- ✅ Passes pluginval validation
- ✅ All parameters work correctly
- ✅ State saves/restores properly
- ✅ UI is professional and functional
- ✅ Factory presets included
- ✅ CHANGELOG.md documents version
- ✅ Ready for installation and real-world use
