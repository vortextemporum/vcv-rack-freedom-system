# checkpoint-system

**Purpose:** Enable risk-free exploration and clean resume points through strategic workflow boundaries and handoff documentation.

---

## Core Concept

TÂCHES v2.0 is designed for **Claude Code's checkpointing feature (Esc+Esc)**, which allows users to rewind conversations and try different paths. The checkpoint system makes every decision boundary a safe exploration point.

## What It Enables

**Risk-Free Exploration:**
```
User: 3 (Save as template)
→ Template saved
User: "Hmm, don't want that actually"
User: Esc + Esc → Rewind conversation
User: [Restore options presented]
User: Restore code only → Template gone
User: 2 (Different option) → New path
```

**Benefit:** Users can safely try any option without fear. Makes exploration **fun** instead of risky.

## Checkpoint Types

### 1. Stage Boundaries (Hard Checkpoints)

**Occur after completing each workflow stage:**
- Stage 0: Research complete
- Stage 1: Planning complete
- Stage 2: Foundation compiles
- Stage 3: Shell loads in DAW
- Stage 4: DSP implemented and tested
- Stage 5: GUI implemented and tested
- Stage 6: Validation passed

**What happens:**
1. Auto-commit with standardized message
2. Update `.continue-here.md` handoff file
3. Present decision menu
4. Wait for user response

**Why hard checkpoints:**
- Natural completion points
- Clean state for resume
- Git commit provides rollback
- Context preserved in handoff file

### 2. Phase Boundaries (Soft Checkpoints)

**Occur within complex stages (complexity ≥3):**
- Stage 4.1: Core DSP algorithm
- Stage 4.2: Parameter modulation
- Stage 4.3: Advanced features
- Stage 5.1: Layout structure
- Stage 5.2: Visual polish

**What happens:**
1. Auto-commit with phase message
2. Update handoff file with phase progress
3. Present decision menu (continue or pause)
4. Proceed if user continues

**Why soft checkpoints:**
- Break down complex work
- Isolate concerns
- Enable mid-stage pause
- Easier debugging

### 3. Decision Checkpoints

**Occur before making significant choices:**
- Before starting mockup ("How would you like to start?")
- Before applying template ("Which template?")
- Before implementing improvement ("Confirm scope?")
- On build failure ("What would you like to do?")

**What happens:**
1. Present decision menu
2. Wait for user choice
3. Execute chosen path
4. Document decision in context

**Why decision checkpoints:**
- Enable exploration
- User stays in control
- Safe to try options
- Can rewind and choose differently

## Handoff File System

### Purpose

`.continue-here.md` files preserve context across:
- Long implementations (pause and resume later)
- Fresh chat sessions (resume in new conversation)
- Exploration paths (rewind and try different options)
- Error recovery (pause, fix manually, resume)

### Location

`plugins/[PluginName]/.continue-here.md`

Created after Stage 0, updated after every checkpoint.

### Format

```yaml
---
plugin: PluginName
stage: 4
phase: 2
status: in_progress
last_updated: 2025-11-10 14:23:45
complexity_score: 4.2
phased_implementation: true
---

# Resume Point

## Current State: Stage 4.2 - Parameter Modulation

Currently implementing parameter modulation system for ModDelay plugin.

## Completed So Far

**Stage 0-3:** ✓ Complete
- Research completed (research.md)
- Plan created (plan.md, 3 DSP phases)
- Foundation compiles successfully
- Shell loads in DAW

**Stage 4.1:** ✓ Complete (committed: abc1234)
- Core delay algorithm implemented
- Basic feedback loop working
- Dry/wet mix functional

**Stage 4.2:** 🚧 In Progress (started: 2025-11-10 14:00)
- LFO modulation implemented
- Parameter smoothing added
- Next: Test modulation depth behavior

## Next Steps

1. Complete parameter modulation testing
2. Commit Stage 4.2 with message: "feat: ModDelay Stage 4.2 - parameter modulation"
3. Update plan.md with completion timestamp
4. Present decision menu (continue to 4.3 or pause)

## Context to Preserve

**Key Decisions:**
- Using juce::dsp::Oscillator for LFO
- Smoothing with 50ms ramp time
- Modulation depth: 0-100% of delay time

**Files Modified:**
- Source/PluginProcessor.cpp (lines 45-89)
- Source/PluginProcessor.h (added LFO member)

**Current Build Status:**
- Last build: Successful
- Last test: Passed (automated tests)

**Research References:**
- research.md lines 34-56 (modulation approach)
- plan.md Phase 4.2 test criteria

## How to Resume

**In new chat session:**
1. User: `/continue ModDelay`
2. System reads this file
3. System: "Resuming ModDelay at Stage 4.2..."
4. System presents current state + next steps
5. System: "Continue where we left off? (y/n)"

**After manual fixes:**
1. User makes changes to code
2. User: "resume automation"
3. System reads this file
4. System picks up from "Next Steps"
```

### Update Triggers

**Update `.continue-here.md` after:**
- Every stage completion
- Every phase completion (complex plugins)
- Before presenting decision menu
- After user makes choice at checkpoint
- On build failures (document error context)
- On manual pause ("Pause here" option)

## Checkpointing Workflows

### Risk-Free Template Exploration

```
Session 1:
User: /mockup DelayPlugin
→ Mockup complete (CHECKPOINT)
User: 3 (Save as template)
→ Template saved (STATE CHANGED)

User reviews, decides against it:
User: Esc + Esc (rewind to CHECKPOINT)
User: Restore code only
→ State reverted, template not saved

User: 2 (Different option)
→ New path explored
```

### Mid-Stage Pause and Resume

```
Session 1:
User: /implement ModDelay
→ Stages 0-3 complete
→ Stage 4.1 complete (CHECKPOINT)
→ Stage 4.2 in progress...
User: "Pause here" (CHECKPOINT)
→ .continue-here.md updated
→ Session ends

Session 2 (later, new chat):
User: /continue ModDelay
→ System reads .continue-here.md
→ System: "Resuming at Stage 4.2..."
→ Context restored
→ Work continues
```

### Error Recovery with Checkpoint

```
User: /implement SimpleGain
→ Stages 0-2 complete
→ Stage 3: Build fails (CHECKPOINT)
System: "What would you like to do?"
User: 4 (I'll fix manually)
→ .continue-here.md updated with error context
→ User fixes code manually

User: "resume automation"
→ System reads .continue-here.md
→ System: "Continuing from Stage 3..."
→ Retries build
→ Success, continues
```

### Exploring Different Design Paths

```
User: /mockup ReverbPlugin
System: "How would you like to start?" (CHECKPOINT)
User: 2 (Apply template)
→ Template applied
→ Mockup generated

User reviews, wants to try different approach:
User: Esc + Esc (rewind to CHECKPOINT)
User: Restore code only
→ Template application reverted

System: "How would you like to start?" (back at CHECKPOINT)
User: 1 (Brainstorm together)
→ Different path explored
```

## Resume Commands

### `/continue [PluginName]`

**What it does:**
1. Reads `plugins/[PluginName]/.continue-here.md`
2. Parses current stage/phase and status
3. Summarizes completed work
4. Presents next steps
5. Asks: "Continue where we left off?"

**Example:**
```
User: /continue ModDelay

System reads .continue-here.md:
─────────────────────────────
Resuming ModDelay plugin

Current State: Stage 4.2 - Parameter Modulation
Status: In progress
Last updated: 2025-11-10 14:23

Completed:
✓ Stages 0-3 (Foundation, Shell)
✓ Stage 4.1 (Core DSP)
✓ 50% of Stage 4.2 (LFO + smoothing done)

Next Steps:
1. Test modulation depth behavior
2. Commit Stage 4.2
3. Continue to Stage 4.3 or pause

─────────────────────────────

Continue where we left off? (y/n): _
```

### "resume automation"

**Used after manual fixes during workflow:**
```
System: "Build failed. What would you like to do?"
User: 4 (I'll fix manually)
→ Workflow pauses, .continue-here.md updated

User makes fixes...

User: "resume automation"
→ System reads .continue-here.md
→ System picks up from documented "Next Steps"
→ Continues workflow
```

## Checkpoint State Preservation

### What Gets Preserved

**In git commits:**
- All code changes
- File additions/deletions
- Commit message with stage/phase

**In `.continue-here.md`:**
- Current stage and phase
- Completed work summary
- Next steps to execute
- Key decisions made
- Modified files list
- Build/test status
- Research references

**In plan.md (for complex plugins):**
- Phase completion timestamps
- Test criteria pass/fail
- Notes on implementation choices

### What Doesn't Need Preservation

**Transient state:**
- Claude's working context (regenerated on resume)
- Temporary build files
- Claude's memory of conversation

**Why:** `.continue-here.md` provides enough context for Claude to reconstruct understanding and continue effectively.

## Integration with Other Systems

### Works With Interactive Decisions

```
CHECKPOINT → Decision Menu → User Choice → Execute → CHECKPOINT
```

Every decision menu is a potential checkpoint for exploration.

### Works With Validation

```
Stage Complete → Validation → CHECKPOINT → Decision Menu
```

Validation ensures quality before creating checkpoint.

### Works With Git Commits

```
Stage Complete → Auto-commit → Update Handoff → CHECKPOINT
```

Git commit provides rollback, handoff provides context.

## Best Practices

### Create Checkpoints Frequently

**Good cadence:**
- Every stage boundary
- Every phase boundary (complex work)
- Before major decisions
- After significant completions

**Not too frequent:**
- Don't checkpoint mid-implementation
- Don't checkpoint during active coding
- Wait for natural boundaries

### Document Context Thoroughly

**In `.continue-here.md`, always include:**
- What's done
- What's in progress
- What's next
- Key decisions made
- Files touched

**Why:** Next session (or exploration path) needs this context.

### Make Checkpoints Explorable

**Present meaningful options:**
- Not just "continue or stop"
- Include discovery features
- Enable trying different paths
- Make exploration safe

### Clean Up Failed Paths

**When exploring:**
```
Try Option A → Doesn't work well → Esc+Esc → Rewind
Try Option B → Better → Continue
```

Only the successful path gets committed.

## Success Criteria

Checkpoint system is working when:
1. Users can safely try any option (no fear of breaking)
2. Work can pause and resume cleanly across sessions
3. Error recovery doesn't lose context
4. Exploration is encouraged, not risky
5. `.continue-here.md` provides enough context to resume effectively
6. Git commits align with checkpoints
7. System feels playful and safe to experiment with
