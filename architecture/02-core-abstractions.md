## Core Abstractions

The system is built on 5 fundamental concepts:

### 1. Navigation Index (CLAUDE.md)

**Responsibility:** Tell Claude where to find system components and context

**Key Purpose:**
- System topology map (where things are located)
- Component directory (skills, subagents, commands, contracts)
- High-level behavioral hints (when to use what)
- Navigation guidance for Claude (~200 words, not 2000)

**What CLAUDE.md Contains:**
- Skills location and purpose (`.claude/skills/`)
- Subagents location and roles (`.claude/agents/`)
- Slash commands location (`.claude/commands/`)
- Contract locations (`plugins/[Name]/.ideas/`)
- State tracking files (`PLUGINS.md`, `.claude/handoff.md`)
- Key system principles (e.g., "contracts are immutable during implementation")

**What CLAUDE.md Does NOT Contain:**
- Detailed workflow instructions (that's in skills)
- Command behavior definitions (that's in slash commands)
- Validation logic (that's in hooks)
- Duplicated content from other components

**Example Structure:**
```markdown
# TÂCHES v2.0 - Plugin Development System

## System Components
- Skills: `.claude/skills/` - plugin-workflow, plugin-ideation, plugin-improve, ui-mockup
- Subagents: `.claude/agents/` - foundation-agent, shell-agent, dsp-agent, gui-agent, validator
- Commands: `.claude/commands/` - /dream, /implement, /improve, /continue

## Contracts (Single Source of Truth)
- `plugins/[Name]/.ideas/` - creative-brief.md, parameter-spec.md, architecture.md, plan.md
- State: PLUGINS.md (all plugins), .claude/handoff.md (active workflow)

## Key Principle
Contracts are immutable during implementation. All stages reference the same specs.
```

**CLAUDE.md instructs Claude WHERE to find details, not WHAT those details are.**

### 2. Workflows (Skills)

**Responsibility:** High-level coordination and subagent dispatch

**Characteristics:**
- Stay in main conversation context
- Dispatch implementation to subagents
- Manage git commits and state updates
- Present results to user

**Interface Pattern:**
```typescript
interface Workflow {
  checkPreconditions(): boolean
  executePhases(): Phase[]
  commitResults(phase: Phase): void
  updateHandoff(state: State): void
}
```

**Types:**
- **plugin-workflow** - Dispatcher for 7-stage creation
- **plugin-improve** - Versioned enhancement workflow
- **plugin-ideation** - Creative vision capture
- **ui-mockup** - Two-phase UI design

### 3. Subagents

**Responsibility:** Autonomous implementation in fresh contexts

**Characteristics:**
- Spawned via Task tool
- Receive complete specifications
- Return structured reports
- No direct user interaction

**Interface:**
```typescript
interface Subagent {
  name: string
  inputs: SubagentInputs
  execute(): SubagentReport
  validate(): ValidationResult
}

interface SubagentReport {
  agent: string  // Self-identification
  status: "success" | "failure"
  outputs: Record<string, any>
  issues: string[]
  ready_for_next_stage: boolean
}
```

**Types:**
- **foundation-agent subagent** - Build system setup (Stage 2)
- **shell-agent subagent** - Parameter implementation (Stage 3)
- **dsp-agent subagent** - Audio processing (Stage 4)
- **gui-agent subagent** - WebView UI integration (Stage 5)
- **validator subagent** - Quality audit (optional, complexity 4+)
- **troubleshooter subagent** - Research and investigation

### 4. Contracts (Canonical Specifications)

**Responsibility:** Single source of truth for implementation

**Terminology distinction:**
- **"Specification" (spec)** = The file itself (e.g., `parameter-spec.md`, `architecture.md`)
  - Concrete, tangible document
  - "Read the parameter spec"
  - "The spec defines 12 parameters"
- **"Contract"** = The enforcement concept/promise
  - Abstract, binding agreement
  - "Contracts enforce zero drift"
  - "Implementation adheres to the contract"
  - "Validate against contract"

**Example usage:** "Stage 3 implements parameters from **parameter-spec.md** (the specification). The **contract** ensures all subsequent stages reference this same spec, guaranteeing zero drift between design and implementation."

**Types:**

**creative-brief.md** - High-level vision
```markdown
# Plugin Concept
- Type: Effect/Synth/Utility
- Sonic goals
- Use cases
- Inspirations
```

**parameter-spec.md** - Implementation contract (CRITICAL)
```markdown
# Parameter Specification

## PARAMETER_ID
- Type: Float | Choice | Bool
- Range: min to max
- Default: value
- Skew: factor
- UI Control: description
- DSP Usage: how it's used
```

**architecture.md** - DSP implementation contract
```markdown
# DSP Architecture

## Core Components
- Component Name (juce::dsp::Class)
- Purpose
- Parameters affected

## Processing Chain
Input → Component A → Component B → Output

## Parameter Mapping
| Parameter ID | DSP Component | Usage |
```

**plan.md** - Stage-by-stage breakdown
```markdown
# Implementation Plan

## Complexity Assessment
Overall: 3/5 (phased DSP implementation)

## Stage Breakdown
### Stage 3: Shell
- Implement EXACT parameters from parameter-spec.md
### Stage 4: DSP (3 phases)
- Phase 4.1: Core from architecture.md
- Phase 4.2: Modulation from architecture.md
### Stage 5: GUI
- Use v[N]-ui.html from finalized mockup
```

### 5. Hybrid Validation Strategy

**Two-layer validation ensures both correctness and quality:**

**Layer 1: SubagentStop Hooks (Deterministic)**
- Automatic validation after each subagent completes
- Fast (1-3 seconds)
- Pattern matching: file existence, type correctness, compilation
- Binary pass/fail - blocks workflow if fails
- Catches obvious errors immediately

**Layer 2: validator Subagent (Semantic)**
- Custom subagent invoked explicitly by workflow
- Slower (30-60 seconds)
- LLM-based reasoning about code quality and design
- Returns nuanced recommendations
- User decides whether to accept/ignore findings
- Invoked for complexity 4+ plugins or when thorough review needed

**Pattern:**
```typescript
interface Validator {
  stage: string
  expected_outputs: string[]
  actual_outputs: Record<string, any>
  reference_docs: Record<string, string>

  validate(): ValidationResult
}

interface ValidationResult {
  status: "PASS" | "FAIL"
  checks: Check[]
  recommendation?: string
  continue_to_next_stage: boolean
}
```

**Validation Flow:**
```
Stage completes
  ↓
Layer 1: Hook validates patterns (2s)
  → FAIL: Block immediately with error
  → PASS: Continue
  ↓
Layer 2: validator subagent reviews semantics (60s)
  → Returns recommendations
  → User decides next action
```

**When to use validator subagent:**
- Always run hooks (automatic)
- Invoke validator when:
  - Plugin complexity >= 4
  - User requests thorough review
  - First time implementing plugin type
- Skip validator when:
  - Simple plugin (complexity 1-2)
  - Rapid prototyping

**Validation Points:**
- After Stage 0 (Research → architecture.md complete?)
- After Stage 1 (Planning → all contracts referenced?)
- After Stage 2 (Foundation → builds successfully?)
- After Stage 3 (Shell → all parameters from spec?)
- After Stage 4 (DSP → all components from architecture?)
- After Stage 5 (GUI → all bindings from spec?)
- After Stage 6 (Validation → pluginval actually ran?)

---
