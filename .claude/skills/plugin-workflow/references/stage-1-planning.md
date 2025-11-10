# Stage 1: Planning

**Context:** This file is part of the plugin-workflow skill.
**Invoked by:** Main workflow dispatcher after Stage 0 completion
**Purpose:** Analyze complexity and create implementation plan with phase breakdown if needed

---

**Goal:** Analyze complexity and create implementation plan

**Duration:** 2-5 minutes

**Model Configuration:**
- Model: Sonnet (deterministic planning)
- Extended thinking: DISABLED

## Preconditions

1. Stage 0 must be complete (architecture.md exists)
2. PLUGINS.md shows `🚧 Stage 0` or similar

## Contract Prerequisites (CRITICAL - BLOCKING)

Check for required contract files:

```bash
test -f "plugins/${PLUGIN_NAME}/.ideas/parameter-spec.md" && echo "✓ parameter-spec.md" || echo "✗ parameter-spec.md MISSING"
test -f "plugins/${PLUGIN_NAME}/.ideas/architecture.md" && echo "✓ architecture.md" || echo "✗ architecture.md MISSING"
```

**If parameter-spec.md OR architecture.md is missing:**

STOP IMMEDIATELY and BLOCK with this exact message:

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ BLOCKED: Cannot proceed to Stage 1
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Missing implementation contracts:

Required contracts:
✓ creative-brief.md - exists
[✓/✗] parameter-spec.md - [exists/MISSING (required)]
[✓/✗] architecture.md - [exists/MISSING (required)]

WHY BLOCKED:
Stage 1 planning requires complete specifications to prevent implementation
drift. These contracts are the single source of truth.

HOW TO UNBLOCK:
1. parameter-spec.md: Complete ui-mockup two-phase workflow
   - Run: /dream [PluginName]
   - Choose: "Create UI mockup"
   - Design UI and finalize (Phase 4.5)
   - Finalization generates parameter-spec.md

2. architecture.md: Create DSP architecture specification
   - Use architecture.md DSP specification (JUCE modules and processing chain)
   - Document DSP components and processing chain
   - Map parameters to DSP components
   - Save to plugins/[PluginName]/.ideas/architecture.md

Once both contracts exist, Stage 1 will proceed.
```

DO NOT PROCEED PAST THIS POINT if contracts are missing.

Exit skill and wait for user to create contracts.

## Actions (contracts confirmed present)

1. Read all contracts:

```bash
# Read parameter specification
cat plugins/[PluginName]/.ideas/parameter-spec.md

# Read DSP architecture specification
cat plugins/[PluginName]/.ideas/architecture.md
```

2. Calculate complexity score:

**Formula:**
```
score = min(param_count / 5, 2.0) + algorithm_count + feature_count
Cap at 5.0
```

**Extract metrics:**

From parameter-spec.md:
- Count parameters (each parameter definition = 1)
- param_score = min(param_count / 5, 2.0)

From architecture.md:
- Count distinct DSP algorithms/components
- algorithm_count = number of juce::dsp classes or custom algorithms

From architecture.md (identify features):
- Feedback loops present? (+1)
- FFT/frequency domain processing? (+1)
- Multiband processing? (+1)
- Modulation systems (LFO, envelope)? (+1)
- External MIDI control? (+1)
- feature_count = sum of above

**Calculate:**
```
total_score = param_score + algorithm_count + feature_count
final_score = min(total_score, 5.0)
```

**Show breakdown:**
```
Complexity Calculation:
- Parameters: [N] parameters ([N/5] points, capped at 2.0) = [X.X]
- Algorithms: [N] DSP components = [N]
- Features: [List] = [N]
Total: [X.X] / 5.0
```

3. Determine implementation strategy:

   - **Simple (score ≤ 2.0):** Single-pass implementation
   - **Complex (score ≥ 3.0):** Phase-based implementation with staged commits

4. For complex plugins (score ≥ 3), create phases:

**Stage 4 (DSP) phases:**

- Phase 4.1: Core processing (essential audio path)
- Phase 4.2: Parameter modulation (APVTS integration)
- Phase 4.3: Advanced features (if applicable)

**Stage 5 (GUI) phases:**

- Phase 5.1: Layout and basic controls
- Phase 5.2: Advanced UI elements
- Phase 5.3: Polish and styling (if applicable)

Each phase needs:

- Description of what gets implemented
- Test criteria to verify completion
- Estimated duration

## Output

Create `plugins/[PluginName]/.ideas/plan.md`

**Format for simple plugins:**

```markdown
# [PluginName] - Implementation Plan

**Date:** [YYYY-MM-DD]
**Complexity Score:** [X.X] (Simple)
**Strategy:** Single-pass implementation

## Stages

- Stage 0: Research ✓
- Stage 1: Planning ← Current
- Stage 2: Foundation
- Stage 3: Shell
- Stage 4: DSP
- Stage 5: GUI
- Stage 6: Validation

## Estimated Duration

Total: ~[X] minutes

- Stage 2: 5 min
- Stage 3: 5 min
- Stage 4: [X] min
- Stage 5: [X] min
- Stage 6: 15 min
```

**Format for complex plugins:**

```markdown
# [PluginName] - Implementation Plan

**Date:** [YYYY-MM-DD]
**Complexity Score:** [X.X] (Complex)
**Strategy:** Phase-based implementation

## Complexity Factors

- Parameters: [N] ([N/5] points)
- Algorithms: [N] ([N] points)
- Features: [List] ([N] points)

## Stage 4: DSP Phases

### Phase 4.1: Core Processing

**Goal:** [Description]
**Test Criteria:**

- [ ] [Criterion 1]
- [ ] [Criterion 2]
      **Duration:** [X] min

### Phase 4.2: Parameter Modulation

**Goal:** [Description]
**Test Criteria:**

- [ ] [Criterion 1]
- [ ] [Criterion 2]
      **Duration:** [X] min

### Phase 4.3: Advanced Features

**Goal:** [Description]
**Test Criteria:**

- [ ] [Criterion 1]
- [ ] [Criterion 2]
      **Duration:** [X] min

## Stage 5: GUI Phases

### Phase 5.1: Layout and Basic Controls

**Goal:** [Description]
**Test Criteria:**

- [ ] [Criterion 1]
- [ ] [Criterion 2]
      **Duration:** [X] min

### Phase 5.2: Advanced UI Elements

**Goal:** [Description]
**Test Criteria:**

- [ ] [Criterion 1]
- [ ] [Criterion 2]
      **Duration:** [X] min

## Estimated Duration

Total: ~[X] hours

- Stage 2: 5 min
- Stage 3: 5 min
- Stage 4: [X] min (phased)
- Stage 5: [X] min (phased)
- Stage 6: 20 min
```

## Update .continue-here.md

Use Edit tool to update YAML frontmatter:

```yaml
---
plugin: [PluginName]
stage: 1
status: complete
last_updated: [YYYY-MM-DD HH:MM:SS]
complexity_score: [X.X]
phased_implementation: [true/false]
---
```

Update markdown sections:
- Current State: "Stage 1 - Planning complete"
- Completed So Far: Add Stage 1 details
- Next Steps: List Stage 2 actions
- Context to Preserve: Add complexity score, strategy

## Update PLUGINS.md

1. Update status:

Use Edit tool to change:
```markdown
**Status:** 🚧 Stage 0 → **Status:** 🚧 Stage 1
```

2. Add timeline entry:

Use Edit tool to append to Lifecycle Timeline:
```markdown
- **[YYYY-MM-DD] (Stage 1):** Planning - Complexity [X.X]
```

3. Update Last Updated field

## Git Commit

```bash
git add plugins/[PluginName]/.ideas/plan.md plugins/[PluginName]/.continue-here.md PLUGINS.md
git commit -m "$(cat <<'EOF'
feat: [PluginName] Stage 1 - planning complete

Complexity: [X.X]
Strategy: [Single-pass | Phased implementation]

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"
```

Display commit hash:
```bash
git log -1 --format='✓ Committed: %h - Stage 1 complete'
```

## Decision Menu

Present inline numbered list (NOT AskUserQuestion):

```
✓ Stage 1 complete: plan created (Complexity [X.X], [single-pass/phased])

What's next?
1. Continue to Stage 2 (recommended)
2. Review plan details
3. Adjust complexity assessment
4. Review contracts (parameter-spec, architecture)
5. Pause here
6. Other

Choose (1-6): _
```

Wait for user response. Handle:
- Number (1-6): Execute corresponding option
- "continue" keyword: Execute option 1
- "pause" keyword: Execute option 5
- "review" keyword: Execute option 2
- "other": Ask "What would you like to do?" then re-present menu

---

**Return to:** Main plugin-workflow orchestration in `SKILL.md`
