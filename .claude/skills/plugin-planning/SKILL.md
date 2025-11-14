---
name: plugin-planning
description: Stage 0 (Research & Planning) for JUCE plugin development. Invoke via /plan command after creative-brief.md exists or as first step of /implement workflow.
allowed-tools:
  - Read # For contracts and references
  - Write # For architecture.md, plan.md
  - Edit # For state updates (PLUGINS.md, .continue-here.md)
  - Bash # For git commits, precondition checks
  - WebSearch # For professional plugin research
  - Grep # For searching existing implementations
  - Glob # For finding reference files
preconditions:
  - creative-brief.md must exist in plugins/[Name]/.ideas/
  - Plugin must NOT already be past Stage 0
---

# plugin-planning Skill

**Purpose:** Handle Stage 0 (Research & Planning - consolidated) through subagent delegation. This skill creates the foundation contracts (architecture.md, plan.md) that guide implementation in a single consolidated pass.

**Invoked by:** `/plan` command or as first step of `/implement` workflow

---

## Entry Point

**Check preconditions first:** See [references/preconditions.md](references/preconditions.md) for detailed validation logic.

Quick validation:
1. creative-brief.md must exist at `plugins/[Name]/.ideas/`
2. Plugin status must be ≤ Stage 0 (not already in implementation)
3. Detect existing contracts (architecture.md, plan.md) for resume logic

If all preconditions pass → proceed to Stage 0

---

## Stage 0: Research & Planning

**Goal:** Create DSP architecture specification (architecture.md) AND implementation plan (plan.md)
**Duration:** 5-35 minutes (complexity-dependent)
**Implementation:** Delegated to research-planning-agent subagent via Task tool

<preconditions>
  <parameter_specification required="true">
    Stage 0 requires parameter definitions to research DSP architecture.

    Accept EITHER:
    - parameter-spec.md (full specification from finalized mockup)
    - parameter-spec-draft.md (minimal specification from ideation)

    <check>
      IF parameter-spec.md exists:
        Use full specification (preferred)
        LOG: "Using full parameter specification"
      ELSE IF parameter-spec-draft.md exists:
        Use draft specification (sufficient for Stage 0)
        LOG: "Using draft parameters. Full spec needed before Stage 1."
      ELSE:
        BLOCK with error: "No parameter specification found. Either:
        1. Run quick parameter capture (/dream → option 1)
        2. Create UI mockup first (/dream → option 2)
        3. Manually create parameter-spec.md or parameter-spec-draft.md"
    </check>
  </parameter_specification>
</preconditions>

**Dispatch pattern:**

1. Read contracts to pass to subagent:
   - plugins/[Name]/.ideas/creative-brief.md (REQUIRED)
   - plugins/[Name]/.ideas/parameter-spec.md OR parameter-spec-draft.md (REQUIRED - see preconditions above)
   - plugins/[Name]/.ideas/mockups/*.yaml (if exists, for design sync)

2. Construct prompt with contracts prepended:
   ```
   You are research-planning-agent. Execute Stage 0 (Research & Planning) for [PluginName].

   Creative brief:
   [content of creative-brief.md]

   Parameters:
   [content of parameter-spec.md or parameter-spec-draft.md]

   [If mockup exists:]
   UI mockup:
   [content of mockup file]

   Execute the full Stage 0 protocol:

   PART 1 - Research:
   1. Complexity detection (Tier 1-6)
   2. Feature identification (meta-research)
   3. Per-feature deep research (algorithmic understanding, professional research, JUCE API mapping, validation)
   4. Integration analysis
   5. Create architecture.md from template

   PART 2 - Planning:
   1. Calculate complexity score from parameters and architecture
   2. Determine implementation strategy (single-pass or phased)
   3. Create phase breakdown if complex (score ≥ 3.0)
   4. Generate plan.md from template

   Update state files and commit both architecture.md and plan.md.
   Return JSON report with both file locations and status.
   ```

3. Invoke subagent:
   ```
   Task(subagent_type="research-planning-agent", description="[prompt with contracts]", model="sonnet")
   ```

4. AFTER subagent returns, execute checkpoint protocol:
   - Read subagent's JSON report
   - Verify architecture.md created and contains all required sections
   - Verify plan.md created with complexity score and strategy
   - Present decision menu (see [references/state-updates.md](references/state-updates.md))
   - WAIT for user response

**Subagent executes:** Full protocol from research-planning-agent.md
- Complexity detection (Tier 1-6) with extended thinking
- Per-feature deep research (algorithmic understanding, professional research, JUCE API mapping, validation)
- Integration analysis (dependencies, interactions, processing order, threads)
- architecture.md generation from assets/architecture-template.md
- Complexity score calculation from parameters and architecture
- Implementation strategy determination
- plan.md generation from assets/plan-template.md
- State updates (.continue-here.md, PLUGINS.md, git commit)

**Outputs:**
- `plugins/[Name]/.ideas/architecture.md` (see assets/architecture-template.md)
- `plugins/[Name]/.ideas/plan.md` (see assets/plan-template.md)

**Decision menu after subagent completes:**

```
✓ Stage 0 complete: Research & Planning finished
   Architecture documented, complexity assessed ([X.X])
   Strategy: [Single-pass | Phased implementation]

What's next?
1. Start implementation - Begin Stage 1 (Foundation + Shell) (recommended)
2. Review contracts (architecture.md, plan.md)
3. Improve creative brief based on research
4. Run deeper JUCE investigation (deep-research skill) ← Discover troubleshooting
5. Pause here
6. Other

Choose (1-6): _
```

<delegation_rules>
**Handle menu choices:**

<delegation_rule skill="plugin-workflow" trigger="option_1">
- Condition: User chooses option 1 (Start implementation)
- Action: Must invoke plugin-workflow skill via Skill tool
- Prompt: "Begin implementation for [PluginName] starting at Stage 1"
</delegation_rule>

- **Option 1: Start implementation**

  Invoke plugin-workflow skill via Skill tool:
  ```
  Skill tool:
  - skill: "plugin-workflow"
  - prompt: "Begin implementation for [PluginName] starting at Stage 1"
  ```

- **Option 2: Review contracts**

  Read and display files:
  - `plugins/[PluginName]/.ideas/architecture.md`
  - `plugins/[PluginName]/.ideas/plan.md`
  - `plugins/[PluginName]/.ideas/parameter-spec.md`
  - `plugins/[PluginName]/.ideas/creative-brief.md`

  After displaying, return to decision menu.

- **Option 3: Improve creative brief**

  Invoke plugin-ideation skill in improvement mode:
  ```
  Skill tool:
  - skill: "plugin-ideation"
  - prompt: "Improve creative brief for [PluginName] based on Stage 0 research findings"
  ```

<delegation_rule skill="deep-research" trigger="option_4">
- Condition: User chooses option 4 (Run deeper investigation)
- Action: Must invoke deep-research skill via Skill tool
</delegation_rule>

- **Option 4: Run deeper JUCE investigation**

  Invoke deep-research skill via Skill tool:
  ```
  Skill tool:
  - skill: "deep-research"
  - prompt: "Investigate JUCE implementation patterns for [PluginName] - focus on [topic from architecture.md]"
  ```

- **Option 5: Pause here**

  Confirm state saved:
  - Verify `.continue-here.md` shows stage: 0, status: complete
  - Verify `PLUGINS.md` shows 🚧 Stage 0
  - Display: "Stage 0 complete. Resume anytime with /continue [PluginName]"
  - Exit skill

- **Option 6: Other**

  Collect custom input and handle appropriately:
  - If request relates to implementation: Route to plugin-workflow
  - If request relates to design: Route to ui-mockup
  - If request relates to research: Route to deep-research
  - Otherwise: Handle directly or exit
</delegation_rules>

**Note:** research-planning-agent runs in fresh context (5-35 min session doesn't pollute orchestrator)

**VALIDATION GATE: Before proceeding to Stage 1:**

1. Verify: architecture.md exists and is not empty
2. Verify: architecture.md contains all required sections (Core Components, Processing Chain, Parameter Mapping)
3. Verify: plan.md exists with complexity score and implementation strategy
4. If any verification fails:
   - Display error: "Stage 0 incomplete - architecture.md or plan.md missing/invalid"
   - Return to Stage 0
5. Only proceed when all verifications pass

---

## Handoff to Implementation

<handoff_protocol id="planning-to-implementation">
<state_requirement>
CRITICAL: Handoff file is already updated to Stage 0 complete by research-planning-agent. Verify state before proceeding to Stage 1.
</state_requirement>

**When user chooses to proceed to Stage 1:**

<verification_step>
Before handoff, verify:
- plugins/[PluginName]/.continue-here.md exists with stage: 0, status: complete
- PLUGINS.md shows 🚧 Stage 0
- Both architecture.md and plan.md exist
</verification_step>

Display to user:

```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✓ Stage 0 Complete: Research & Planning
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Plugin: [PluginName]
Complexity: [X.X] ([Simple/Complex])
Strategy: [Single-pass | Phased implementation]

Contracts created:
✓ creative-brief.md
✓ parameter-spec.md
✓ architecture.md
✓ plan.md

Ready for implementation. Proceeding to Stage 1 (Foundation + Shell)...
```
</handoff_protocol>

---

## Reference Files

Detailed protocol is in:
- Research-planning-agent.md - Complete Stage 0 research and planning protocol

Templates are in:
- `assets/architecture-template.md` - DSP architecture contract template
- `assets/plan-template.md` - Implementation plan template

Legacy references (for understanding prior workflow):
- `references/stage-0-research.md` - Original research protocol
- `references/stage-1-planning.md` - Original planning protocol
