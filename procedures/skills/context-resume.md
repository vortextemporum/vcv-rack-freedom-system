# context-resume

**Purpose:** Load plugin development context from handoff files to resume work seamlessly across sessions.

---

## What It Does

Reads `.continue-here.md` files to understand where you left off and what to do next. Eliminates manual "where was I?" overhead when resuming plugin development.

## When Invoked

**User says:**
- "Continue [PluginName]"
- "Resume [PluginName]"
- "Pick up where we left off"

**Slash command:**
```bash
/continue [PluginName]
```

## Handoff File System

### File Locations

**Main plugin development:**
```
plugins/[PluginName]/.continue-here.md
```

**Ideation/mockup in progress:**
```
plugins/[PluginName]/.ideas/.continue-here.md
```

**Multiple iterations:**
```
plugins/[PluginName]/.ideas/mockups/.continue-here.md
```

System checks all locations and loads the most relevant.

## Handoff Document Structure

```markdown
# Continue: [PluginName]

**Last updated:** 2025-11-08 14:30
**Current state:** Stage 4 complete, ready for Stage 5
**Last action:** DSP implementation finished

---

## What We Have Now
[Summary of completed work]

## What Was Just Done
[Details of last session]

## Suggested Next Steps
1. [Primary next action] ⭐ Recommended
2. [Alternative option]
3. [Other possibility]

---

## Quick Reference
Files: ...
Commands: ...
```

## Loading Process

### Step 1: Locate Handoff
Searches for `.continue-here.md` in:
1. Plugin root directory
2. `.ideas/` subdirectory
3. `.ideas/mockups/` subdirectory

### Step 2: Parse Context
Extracts:
- Current state (which stage, what's complete)
- Last action (what happened in previous session)
- Suggested next steps (intelligent recommendations)
- Quick references (files, commands)

### Step 3: Summarize
Presents concise summary:

```
Resuming [PluginName]:

Status: Stage 4 complete, DSP implemented
Last session: Core audio processing finished, tested stable

Next steps:
1. ⭐ Start Stage 5 (GUI implementation)
2. Review Stage 4 code
3. Run additional tests

Ready to proceed with Stage 5?
```

### Step 4: Proceed
Based on user confirmation:
- Invokes next skill (e.g., `plugin-workflow` Stage 5)
- Loads specific files if requested
- Continues automation from pause point

## Context Types

### Plugin Workflow Resume
```
Current: Stage 4.2 complete
Next: Continue to Stage 4.3

Loads: plugin-workflow skill
Action: Resume multi-phase DSP implementation
```

### Ideation Resume
```
Current: Creative brief complete
Next: Create UI mockup or start implementation

Loads: Offers ui-mockup or plugin-workflow
Action: User chooses next direction
```

### Mockup Resume
```
Current: v2 mockup complete
Next: Implement in Stage 5 or iterate to v3

Loads: Offers plugin-workflow Stage 5 or ui-mockup
Action: User decides iteration vs implementation
```

### Improvement Resume
```
Current: Improvement brief written
Next: Implement changes or continue brainstorming

Loads: Offers plugin-improve or plugin-ideation
Action: User decides direction
```

## Intelligent Suggestions

The system analyzes state and recommends actions:

**Stage boundaries:**
- Completed Stage 4 → Suggest Stage 5 (GUI)
- Completed Stage 5 → Suggest Stage 6 (Validation)

**Incomplete work:**
- Failed build in Stage 4 → Suggest investigation before continuing
- Missing mockup at Stage 5 → Suggest creating mockup first

**Context-specific:**
- Complex plugin with phases → Suggest next phase
- Simple plugin → Suggest next stage
- Improvement in progress → Suggest testing after implementation

## Session Continuity

### What Gets Preserved

**Technical state:**
- Which stage/phase
- Completed tasks
- Pending work
- Known issues

**Creative context:**
- Design decisions made
- Assumptions documented
- Alternative approaches considered

**File references:**
- What exists
- What's needed
- Where to find things

### What Doesn't Transfer

- In-memory state (reloads fresh)
- Open file buffers
- Build artifacts (may rebuild)

## Multi-Session Workflow

**Monday morning:**
```
/implement DelayPlugin
→ Stages 0-2 complete
→ Pauses at Stage 3
```

**Tuesday afternoon:**
```
/continue DelayPlugin
→ "Resuming DelayPlugin: Stage 2 complete, ready for Stage 3"
→ Continues from Stage 3
```

**Wednesday evening:**
```
/continue DelayPlugin
→ "Resuming DelayPlugin: Stage 5 in progress (Phase 5.2)"
→ Continues multi-phase GUI implementation
```

No context loss between sessions.

## Error Handling

### No Handoff File

```
No .continue-here.md found for [PluginName].

Plugin status:
- Source files exist: Yes
- Last commit: feat: Stage 3 - shell (2 days ago)

Best guess: Workflow was interrupted at Stage 3.

Options:
1. Start from Stage 4 (next stage)
2. Review existing code first
3. Start fresh from Stage 0
```

### Ambiguous State

```
Multiple handoff files found:
1. plugins/[PluginName]/.continue-here.md (main workflow)
2. plugins/[PluginName]/.ideas/.continue-here.md (ideation)

Which context do you want to resume?
```

### Corrupted Handoff

```
Handoff file exists but appears corrupted.

Last commit: feat: Stage 4 - DSP (1 hour ago)

Safe assumption: Continue to Stage 5?
```

## Integration

**Creates handoff files:**
- All skills that pause/complete (workflow, ideation, mockup, improve)

**Reads handoff files:**
- context-resume skill (this one)

**Updates handoff files:**
- Skills update after completing work
- Adds timestamp on each update
- Preserves history in git commits

## Quick Commands Reference

The handoff file includes command shortcuts:

```markdown
## Quick Reference

Resume this work:
/continue [PluginName]

Next likely commands:
"implement Stage 5"  # Start GUI
"show me the DSP code"  # Review implementation
"test [PluginName]"  # Run validation
```

Copy-paste ready commands for common next steps.

## Best Practices

**Always create handoffs:** Every skill that pauses work must create `.continue-here.md`

**Update on completion:** When work finishes, update handoff with new state

**Include context:** Don't just say "Stage 4 done", explain what was implemented

**Suggest intelligently:** Recommend most likely next action based on state

**Preserve lineage:** Git commits ensure handoff file history is tracked

## Output Example

```
✓ Loaded context for DelayPlugin

Last session: Yesterday at 2:30 PM
State: Stage 4 complete (DSP implementation)

What was done:
- Implemented delay buffer with feedback
- Added parameter smoothing
- Tested with white noise input
- All automated tests passed

Files created:
- plugins/DelayPlugin/Source/PluginProcessor.cpp (DSP logic)
- plugins/DelayPlugin/research.md (algorithm notes)
- plugins/DelayPlugin/plan.md (implementation plan)

Recommended next step:
⭐ Start Stage 5 (GUI implementation)
   - Mockup v1 already exists
   - HTML ready to integrate

Continue to Stage 5? [Y/n]
```

Clean resume with full context.
