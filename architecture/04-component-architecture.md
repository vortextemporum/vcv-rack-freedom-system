## Component Architecture

### Component Relationships

```
┌─────────────────────────────────────────────────────────┐
│                     USER REQUEST                         │
└──────────────────────┬──────────────────────────────────┘
                       │
         ┌─────────────▼─────────────┐
         │   NAVIGATION INDEX        │
         │      (CLAUDE.md)           │
         │                            │
         │  - System topology map     │
         │  - Component locations     │
         │  - Navigation guidance     │
         │  - Key principles          │
         └─────────┬──────────────────┘
                   │
    ┌──────────────┼──────────────┐
    │              │              │
    ▼              ▼              ▼
┌────────┐   ┌──────────┐   ┌────────────┐
│WORKFLOW│   │ WORKFLOW │   │  WORKFLOW  │
│ideation│   │ workflow │   │  improve   │
└────────┘   └────┬─────┘   └──────┬─────┘
                  │                 │
        ┌─────────┴─────────┬───────┴────┐
        │                   │            │
        ▼                   ▼            ▼
  ┌──────────┐        ┌─────────┐  ┌─────────┐
  │foundation│        │   dsp   │  │   gui   │
  │ subagent │        │subagent │  │subagent │
  └──────────┘        └─────────┘  └─────────┘
        │                   │            │
        └───────────┬───────┴────────────┘
                    │
              ┌─────▼──────┐
              │ VALIDATOR  │
              │  SUBAGENT  │
              └─────┬──────┘
                    │
         ┌──────────┴──────────────────────┐
         │   HOOKS (6 of 9 used)        │
         │  - PostToolUse (code quality)│
         │  - UserPromptSubmit (context)│
         │  - Stop (stage enforcement)  │
         │  - SubagentStop (validation) │
         │  - PreCompact (preservation) │
         │  - SessionStart (setup)      │
         └──────────────────────────────┘
```

### How Claude Finds Components

**CLAUDE.md provides navigation:**
- Lists component locations and purposes
- Claude reads CLAUDE.md at session start
- When user requests action, Claude consults CLAUDE.md for WHERE to find details
- Claude then reads specific skill/command/hook files as needed (progressive disclosure)

**Example flow:**
1. User types `/implement DelayPlugin`
2. Slash command expands to: "Begin plugin-workflow for DelayPlugin..."
3. Claude thinks: "I need plugin-workflow skill details"
4. Claude checks CLAUDE.md: "Skills are in `.claude/skills/`"
5. Claude invokes Skill tool → `.claude/skills/plugin-workflow/SKILL.md` loads
6. Claude follows detailed skill instructions

**CLAUDE.md doesn't register or route - it tells Claude where to look.**

### Command → Skill Mapping

**Slash commands contain instructions that tell Claude which skill to invoke.**

This is **instructed routing** (Claude follows text instructions), not **automatic routing** (programmatic dispatch).

**Example pattern:**

```markdown
# .claude/commands/implement.md
---
name: implement
description: Create plugin from concept to installed
---

When user runs /implement [PluginName], invoke the plugin-workflow skill to begin the 7-stage implementation process.

First check preconditions:
- PLUGINS.md exists and plugin status is valid
- creative-brief.md exists in plugins/[PluginName]/.ideas/

If preconditions pass, invoke plugin-workflow skill starting at Stage 0 (Research).
```

**Key concept:** The slash command **expands to a prompt** that instructs Claude. Claude reads the instruction and invokes the appropriate skill using the Skill tool.

**Flow:**
1. User types `/implement DelayPlugin`
2. Command expands to prompt: "When user runs /implement DelayPlugin, invoke the plugin-workflow skill..."
3. Claude reads the instruction
4. Claude invokes plugin-workflow skill via Skill tool
5. Skill content loads (progressive disclosure)

**Command → Skill relationships:**
- `/implement` → Instructs Claude to invoke plugin-workflow skill
- `/dream` → Instructs Claude to invoke plugin-ideation skill
- `/improve` → Instructs Claude to invoke plugin-improve skill
- `/continue` → Instructs Claude to load handoff context and resume

**No frontmatter routing fields exist.** Commands are lean entry portals that point Claude to detailed skills.

**Implementation Details:** Complete command specifications are in `procedures/commands/` (implement.md, dream.md, improve.md, continue.md). Skills are fully documented in `procedures/skills/`.

### Interface Specifications

#### Skill Interface

```yaml
---
name: plugin-workflow
description: 7-stage JUCE plugin development workflow (dispatcher pattern)
model: sonnet
allowed-tools:
  - Task      # To spawn subagents
  - Bash      # For git commits
  - Read      # For contracts
  - Edit      # For state updates
preconditions:
  - Plugin directory must not exist OR status is 💡 (ideated)
  - Plugin must NOT be in development (status 🚧)
---
```

**Skill must:**
- Check preconditions before executing
- Dispatch implementation to subagents (Stages 2-5)
- Commit results at stage boundaries
- Update handoff files
- Report progress to user

#### Subagent Interface

```yaml
---
name: dsp-agent
type: agent
description: Implement audio processing from architecture.md (dsp-agent subagent)
model: sonnet
allowed-tools:
  - Read
  - Edit
  - Write
  - Bash
  - mcp__context7__resolve-library-id
  - mcp__context7__get-library-docs
---
```

**Subagent must:**
- Receive complete specifications via prompt
- Execute autonomously (no user interaction)
- Return structured JSON report with `"agent": "agent-name"` field
- Self-validate before reporting completion

#### Validator Interface

```yaml
---
name: validator
type: agent
description: Independent verification of stage completion claims (validator subagent)
model: sonnet  # Quality > speed at gates (see rationale below)
allowed-tools:
  - Read
  - Bash  # For checking files exist
  - Grep  # For verifying code patterns
---
```

**Model Choice: Sonnet (not Haiku)**

**Trade-off Analysis:**
| Aspect | Haiku | Sonnet |
|--------|-------|--------|
| Speed | ~15-30s | ~30-60s |
| Detection | 70-80% | 90-95% |

**Why Sonnet for semantic validation:**
- **Validator focuses on semantic quality** - Hooks already caught pattern errors
- **Reasoning depth required** - Must understand semantic contract violations:
  - DSP architecture doesn't match parameter-spec.md intent
  - Parameter ranges don't align with creative vision
  - Code quality issues (allocations in processBlock)
  - Subtle design decisions in complex plugins
- **30s overhead acceptable** - Imperceptible in 40-80 minute workflow
- **Max Plan benefit** - No cost difference between models, optimize for quality over speed
- **No wasted time** - Only runs when deterministic checks pass

**Validator must:**
- Assume patterns are correct (hooks validated that)
- Compare implementation to creative intent from brief
- Evaluate semantic correctness and design decisions
- Check JUCE best practices (beyond compilation)
- Return structured report with nuanced recommendations
- Be advisory, not blocking (user decides next action)
- Support user override with logged justification (for false positives)

#### Validator Implementation Details

**Stage 3: Parameter Validation** (spec → APVTS verification)

Uses regex pattern matching to verify `parameter-spec.md` matches `PluginProcessor.cpp`:

1. Parse parameter-spec.md for IDs, types, ranges
2. Extract APVTS declarations via regex: `AudioParameter(Float|Bool|Choice)\s*\(\s*ParameterID\s*\{\s*"(\w+)"`
3. Check ID sets match (spec ⊆ code)
4. Validate type mapping (float → AudioParameterFloat, bool → AudioParameterBool)
5. Warn on range mismatches (not errors—may be intentional normalization)

**Stage 4: Component Validation** (plan → DSP code verification)

Pattern matching to verify `plan.md` components exist in code:

1. Parse plan.md for DSP component list (e.g., "juce::dsp::StateVariableTPTFilter")
2. Check required #include present: `#include <juce_dsp/juce_dsp.h>`
3. Verify member variable declaration: `juce::dsp::StateVariableTPTFilter<float> filter;`
4. Warn if prepare() not called (check `prepareToPlay()` body)

**Stage 5: Binding Validation** (HTML ↔ C++ parameter binding verification)

String extraction + set comparison:

1. Extract HTML parameter IDs: `getSliderState\(['"](\w+)['"]\)`
2. Extract C++ relays: `WebSliderRelay\s+(\w+)Relay\s*\{\s*"(\w+)"\s*\}`
3. Check ID sets match (HTML IDs == relay IDs)
4. Verify attachments exist for each relay
5. Verify `.withOptionsFrom(relay)` registration in browser setup

**False Positive Overrides:**

File: `plugins/[PluginName]/.validator-overrides.yaml`

```yaml
overrides:
  - stage: 4
    check_type: "missing_dsp_component"
    pattern: "CustomReverbImpl"
    reason: "Using custom reverb, not juce::dsp::Reverb"
    date: "2025-11-08"
    expires: null  # Optional expiration date
```

Validator loads overrides before running checks and suppresses matching errors with logged reason.

#### Hybrid Validation Flow

**The system uses a two-layer validation approach combining hooks and the validator subagent:**

**Complete Stage 3 Example:**

```
shell-agent subagent completes Stage 3
  ↓
┌─────────────────────────────────────────┐
│ Layer 1: SubagentStop Hook (Automatic) │
│ Deterministic validation - 2 seconds    │
└─────────────────────────────────────────┘
  Checks:
    ✓ All parameter IDs from parameter-spec.md present?
    ✓ Types match (Float→AudioParameterFloat)?
    ✓ Code compiles?
  ↓
  Result: FAIL
  → Block immediately with specific error
  → shell-agent subagent must fix before continuing
  ↓
  Result: PASS
  → Continue to Layer 2
  ↓
┌─────────────────────────────────────────┐
│ Layer 2: validator Subagent (Explicit) │
│ Semantic validation - 60 seconds        │
└─────────────────────────────────────────┘
  plugin-workflow: "Use validator to review Stage 3"
  ↓
  validator reads:
    - creative-brief.md (creative intent)
    - parameter-spec.md (contract)
    - PluginProcessor.cpp (implementation)
  ↓
  validator evaluates:
    - Do parameter ranges match creative vision?
    - Is code quality acceptable?
    - Are JUCE best practices followed?
    - Any subtle issues hooks can't detect?
  ↓
  Returns JSON report:
    {
      "status": "PASS",
      "issues": [
        {
          "severity": "suggestion",
          "description": "Consider adding smoothing to cutoff parameter",
          "location": "PluginProcessor.cpp:142"
        }
      ],
      "recommendation": "continue"
    }
  ↓
  Workflow presents findings to user:
    "✓ Stage 3 validated with 1 suggestion:
     'Consider adding smoothing to cutoff parameter'

     What's next?
     1. Continue to Stage 4 (recommended)
     2. Address suggestion first
     3. Review findings in detail"
  ↓
  User decides next action
```

**Why This Hybrid Approach Works:**

**Performance - Fast path for simple failures**
- Hook catches obvious errors in 2s (vs 60s for validator)
- Validator only runs when patterns are correct
- No wasted LLM invocations on trivial failures

**Quality - Layered defense**
- Hooks = compiler (syntax/pattern errors)
- Validator = code reviewer (semantic/design issues)
- Both together = true "zero drift"

**Separation of concerns**
- Hooks: automatic, binary, blocks workflow
- Validator: interactive, nuanced, advisory
- Maps to system philosophy: automation + user control

**Scales with complexity**
- Simple plugins (complexity 1-2): Hooks sufficient, skip validator
- Complex plugins (complexity 4-5): Full validation stack
- User always in control of validation depth

**validator Subagent Definition:**

```yaml
# .claude/agents/validator.md
---
name: validator
description: |
  Independent semantic validation. Use after deterministic hooks pass
  to verify quality, design decisions, and best practices.
  Invoke for complexity 4+ or when thorough review needed.
tools: Read, Grep, Bash
model: sonnet  # Quality over speed for semantic validation
---

You are an independent validator performing semantic review.

## Your Role
You are NOT checking if files exist or patterns match - hooks did that.
You ARE checking:
- Does implementation match creative intent?
- Are design decisions sound?
- Code quality acceptable?
- JUCE best practices followed (beyond compilation)?
- Any subtle issues hooks can't detect?

## Process
1. Read contracts (creative-brief, parameter-spec, architecture)
2. Read implementation files
3. Evaluate semantic correctness and quality
4. Return structured JSON with recommendations
```

**Hook Validation Script Example:**

```bash
#!/bin/bash
# .claude/hooks/validate-stage.sh

INPUT=$(cat)
SUBAGENT=$(echo "$INPUT" | jq -r '.subagent_name')

case "$SUBAGENT" in
  shell-agent)  # Stage 3 subagent
    # Check all parameters from spec present in code
    python3 .claude/hooks/validators/validate-parameters.py || exit 2
    ;;

  dsp-agent)  # Stage 4 subagent
    # Check all components from architecture present
    python3 .claude/hooks/validators/validate-dsp-components.py || exit 2
    ;;

  gui-agent)  # Stage 5 subagent
    # Check all bindings match parameter IDs
    python3 .claude/hooks/validators/validate-gui-bindings.py || exit 2
    ;;

  *)
    exit 0  # Not a workflow subagent, skip validation
    ;;
esac
```

**Key Principle: Validator subagent is prerequisite-aware**
- Never invoke validator subagent if hooks failed
- Validator subagent assumes patterns are correct, focuses on semantics
- This division prevents wasting time on obvious errors

### Lifecycle Management

**Component Lifecycle:**

1. **Discovery** - Scan and register at startup
2. **Validation** - Check frontmatter, verify tool access
3. **Registration** - Load into routing system
4. **Invocation** - Invoke via Skill tool or Task tool
5. **Completion** - Return results, update state
6. **Cleanup** - Archive handoffs, commit state

**Workflow Lifecycle Example:**

```
1. User: "Make me a reverb plugin"

2. Claude:
   - Consults CLAUDE.md for workflow entry point
   - Checks PLUGINS.md → ReverbPlugin doesn't exist ✓
   - Invokes plugin-workflow skill (CLAUDE.md tells it where to look)

3. plugin-workflow (main context):
   - Checks preconditions ✓
   - Invokes plugin-ideation if no brief exists
   - Returns to wait for /implement

4. User: "/implement ReverbPlugin"

5. plugin-workflow:
   - Stage 0: Research in main context
     → Generates architecture.md
     → Validator verifies ✓
     → Commit "Stage 0 complete"

   - Stage 1: Planning in main context
     → Checks prerequisites (creative-brief ✓, parameter-spec ✓, architecture ✓)
     → Generates plan.md
     → Validator verifies ✓
     → Commit "Stage 1 complete"

   - Stage 2: Foundation
     → Spawns foundation-agent subagent via Task tool
     → Subagent returns report JSON
     → Validator verifies build succeeded ✓
     → Commit "Stage 2 complete"

   - Stage 3: Shell
     → Spawns shell-agent subagent via Task tool
     → Subagent returns report JSON
     → Validator verifies parameters match spec ✓
     → Commit "Stage 3 complete"

   - Stage 4: DSP
     → Spawns dsp-agent subagent via Task tool (executes all phases)
     → Subagent returns report JSON
     → Validator verifies components match architecture ✓
     → Commit "Stage 4 complete"

   - Stage 5: GUI
     → Spawns gui-agent subagent via Task tool
     → Subagent returns report JSON
     → Validator verifies bindings match spec ✓
     → Commit "Stage 5 complete"

   - Stage 6: Validation in main context
     → Runs pluginval
     → Creates presets
     → Validator verifies pluginval actually ran ✓
     → Commit "Stage 6 complete"

   - Update PLUGINS.md → ✅ Working
   - Present to user: "ReverbPlugin complete, ready to install"

6. User: "Install it"

7. plugin-lifecycle:
   - Runs build-and-install.sh
   - Updates PLUGINS.md → 📦 Installed
```

---

## Related Procedures

This architecture is implemented through:

- `procedures/skills/plugin-workflow.md` - Complete 7-stage workflow dispatcher implementation
- `procedures/commands/implement.md` - Entry point for workflow invocation
- `procedures/commands/dream.md` - Entry point for ideation workflow
- `procedures/commands/improve.md` - Entry point for improvement workflow
- `procedures/commands/continue.md` - Entry point for context resumption
- `procedures/core/interactive-decision-system.md` - Decision menu implementation patterns
- `procedures/agents/troubleshooter.md` - Multi-level investigation protocol
