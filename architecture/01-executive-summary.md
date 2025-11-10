## Executive Summary

TÂCHES v2.0 is a conversational AI system for professional JUCE audio plugin development through natural dialogue with Claude Code. The v2 architecture addresses v1's "Frankenstein" feeling through:

1. **Subagent architecture** - Fresh context per implementation stage eliminates context accumulation
2. **Contract-driven development** - parameter-spec.md + architecture.md prevent drift between design and implementation
3. **Hybrid validation** - Hooks (deterministic, 2s) + validator subagent (semantic, 60s) provide layered quality assurance
4. **Quality gates** - Hooks enforce contracts (PostToolUse validates code), stage commits (Stop enforces checkpoints), and preserve context (UserPromptSubmit auto-injects, PreCompact preserves contracts)
5. **Clean abstractions** - Minimal coupling, clear interfaces, predictable behavior
6. **Interactive decision system** - Every checkpoint presents contextual options, making capabilities discoverable through use

## Core Philosophy: Discovery Through Play

The system is designed for **progressive disclosure** rather than documentation-heavy onboarding. Users discover features organically through:

1. **Slash command autocomplete** - Type "/" to see all available workflows instantly
2. **Contextual option menus** - Every checkpoint presents numbered choices
3. **Risk-free exploration** - Claude Code checkpointing (Esc+Esc) makes trying options safe

**Key principle:** Workflows start with explicit slash commands (`/implement`, `/dream`, `/improve`), then become "choose your own adventure" at each checkpoint where users shape their journey by selecting numbered options.

### Key Architectural Decisions

| Decision | Rationale |
|----------|-----------|
| **Slash commands for entry** | Discoverable (autocomplete), deterministic (no pattern matching), fast (direct routing) |
| **Dispatcher pattern for workflows** | Main context stays lightweight by spawning subagents for implementation stages |
| **Contract prerequisites** | parameter-spec.md + architecture.md must exist before Stage 1 Planning |
| **Independent validation** | Validator verifies what stages claim they did, catching drift early |
| **Hook-based quality** | Deterministic validation (PostToolUse) vs. probabilistic (SubagentStop prompt) |
| **Two-phase UI design** | Rapid iteration (yaml + test.html) separated from finalization (7-file package) |
| **Unified state management** | State machine tracks plugin lifecycle, single source of truth |
| **Interactive decision menus** | Contextual options at every checkpoint make features discoverable, user feels in control |

### Major Improvements from v1

- **Single-session creation** - Subagents eliminate manual `/continue` interventions
- **Zero drift** - Contracts enforce alignment across all stages
- **Automatic quality** - Hooks catch violations before build
- **Cleaner routing** - Declarative contracts vs. imperative decision trees
- **One-command installation** - `build-and-install.sh` handles build → install → cache clearing in single step

---

## Related Procedures

This summary is implemented through:

- `procedures/skills/plugin-workflow.md` - Complete 7-stage workflow implementation
- `procedures/commands/implement.md` - `/implement` command entry point
- `procedures/commands/dream.md` - `/dream` command entry point
- `procedures/commands/improve.md` - `/improve` command entry point
- `procedures/scripts/build-and-install.md` - One-command installation script
- `procedures/core/interactive-decision-system.md` - Decision menu implementation

---
