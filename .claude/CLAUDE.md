# PLUGIN FREEDOM SYSTEM - Plugin Development System

## System Components
- **Scripts**: `scripts/` - Build and installation automation
  - build-and-install.sh - Centralized build automation (7-phase pipeline: validate, build, install, verify)
  - verify-backup.sh - Backup integrity verification (Phase 7)
- **Skills**: `.claude/skills/` - Each skill follows Anthropic's pattern with `SKILL.md`, `references/`, and `assets/` subdirectories
  - plugin-workflow, plugin-ideation, plugin-improve (enhanced with regression testing), ui-mockup, context-resume, plugin-testing, plugin-lifecycle, build-automation, troubleshooting-docs, deep-research, design-sync
- **Subagents**: `.claude/agents/` - foundation-agent, shell-agent, dsp-agent, gui-agent, validator, troubleshooter
- **Commands**: `.claude/commands/` - /dream, /implement, /improve, /continue, /test, /install-plugin, /uninstall, /show-standalone, /troubleshoot-juce, /doc-fix, /research, /sync-design
- **Hooks**: `.claude/hooks/` - Validation gates (PostToolUse, SubagentStop, UserPromptSubmit, Stop, PreCompact, SessionStart)
- **Knowledge Base**: `troubleshooting/` - Dual-indexed (by-plugin + by-symptom) problem solutions

## Contracts (Single Source of Truth)
- `plugins/[Name]/.ideas/` - creative-brief.md (vision), parameter-spec.md (parameters), architecture.md (DSP design), plan.md (implementation strategy)
- **State**: PLUGINS.md (all plugins), .continue-here.md (active workflow)
- **Templates**: Contract templates stored in skill assets (`.claude/skills/*/assets/`)

## Key Principles
1. **Contracts are immutable during implementation** - All stages reference the same specs (zero drift)
2. **Dispatcher pattern** - Each subagent runs in fresh context (no accumulation)
3. **Discovery through play** - Features found via slash command autocomplete and decision menus
4. **Instructed routing** - Commands expand to prompts, Claude invokes skills
5. **Required Reading injection** - Critical patterns (`juce8-critical-patterns.md`) are mandatory reading for all subagents to prevent repeat mistakes

## Checkpoint Protocol (System-Wide)

At every significant completion point (stage complete, phase complete, files generated, contract created):

1. Auto-commit changes (if in workflow)
2. Update state files (.continue-here.md, PLUGINS.md)
3. ALWAYS present numbered decision menu:

✓ [Completion statement]

What's next?
1. [Primary action] (recommended)
2. [Secondary action]
3. [Discovery option] ← User discovers [feature]
4. [Alternative path]
5. Other

Choose (1-5): _

4. WAIT for user response - NEVER auto-proceed
5. Execute chosen action

This applies to:
- All workflow stages (0-6)
- All subagent completions
- Contract creation (creative-brief, mockups, parameter-spec)
- Any point where user needs to decide next action

Do NOT use AskUserQuestion tool for decision menus - use inline numbered lists as shown above.

## Subagent Invocation Protocol

Stages 2-5 use the dispatcher pattern:
- Stage 2 → Invoke foundation-agent via Task tool
- Stage 3 → Invoke shell-agent via Task tool
- Stage 4 → Invoke dsp-agent via Task tool
- Stage 5 → Invoke gui-agent via Task tool

The plugin-workflow skill orchestrates, it does not implement.

After subagent completes:
1. Read subagent's return message
2. Commit changes
3. Update .continue-here.md
4. Update PLUGINS.md
5. Present numbered decision menu
6. Wait for user response

This ensures consistent checkpoint behavior and clean separation of concerns.

## Workflow Entry Points
- New plugin: `/dream` → `/implement`
- Resume work: `/continue [PluginName]`
- Modify existing: `/improve [PluginName]`
- Test plugin: `/test [PluginName]`

## Implementation Status
- ✓ Phase 0: Foundation & Contracts (complete)
- ✓ Phase 1: Discovery System (complete)
- ✓ Phase 2: Workflow Engine (complete)
- ✓ Phase 3: Implementation Subagents (complete)
- ✓ Phase 4: Build & Troubleshooting System (complete)
- ✓ Phase 5: Validation System (complete - hybrid validation operational)
- ✓ Phase 6: WebView UI System (complete)
- ✓ Phase 7: Polish & Enhancement (complete - feedback loop operational)

## Phase 7 Components (Polish & Enhancement)

### Skills
- **plugin-lifecycle** (`.claude/skills/plugin-lifecycle/`) - Installation/uninstallation management
- **design-sync** (`.claude/skills/design-sync/`) - Mockup ↔ brief validation, drift detection
- **deep-research** (`.claude/skills/deep-research/`) - Multi-level problem investigation (3-level graduated protocol)
- **troubleshooting-docs** (`.claude/skills/troubleshooting-docs/`) - Knowledge base capture with dual-indexing
- **plugin-improve** (`.claude/skills/plugin-improve/`) - Version management with regression testing (enhanced)

### Commands

**Lifecycle:**
- `/dream` - Ideate new plugin concept
- `/implement [Name]` - Build plugin through 7-stage workflow
- `/continue [Name]` - Resume paused workflow
- `/improve [Name]` - Fix bugs or add features (with regression testing)

**Deployment:**
- `/install-plugin [Name]` - Install to system folders
- `/uninstall [Name]` - Remove binaries (keep source)
- `/reset-to-ideation [Name]` - Remove implementation, keep idea/mockups
- `/destroy [Name]` - Completely remove everything (with backup)

**Quality:**
- `/test [Name]` - Run validation suite
- `/sync-design [Name]` - Validate mockup ↔ brief consistency
- `/research [topic]` - Deep investigation (3-level protocol)
- `/doc-fix` - Document solved problems (with option to promote to Required Reading)
- `/add-critical-pattern` - Directly add current problem to Required Reading (fast path)

### Knowledge Base
- `troubleshooting/build-failures/` - Build and compilation errors
- `troubleshooting/runtime-issues/` - Crashes, exceptions, performance issues
- `troubleshooting/gui-issues/` - UI layout and rendering problems
- `troubleshooting/api-usage/` - JUCE API misuse and migration issues
- `troubleshooting/dsp-issues/` - Audio processing problems
- `troubleshooting/parameter-issues/` - APVTS and state management
- `troubleshooting/validation-problems/` - pluginval failures
- `troubleshooting/patterns/` - Common patterns and solutions
- `troubleshooting/patterns/juce8-critical-patterns.md` - **REQUIRED READING** for all subagents (Stages 2-5)

### Scripts
- `scripts/build-and-install.sh` - Build automation (supports --uninstall)
- `scripts/verify-backup.sh` - Backup integrity verification

## Feedback Loop

The complete improvement cycle:
```
Build → Test → Find Issue → Research → Improve → Document → Validate → Deploy
    ↑                                                                      ↓
    └──────────────────────────────────────────────────────────────────────┘
```

- **deep-research** finds solutions (3-level graduated protocol: Quick → Moderate → Deep)
- **plugin-improve** applies changes (with regression testing and backup verification)
- **troubleshooting-docs** captures knowledge (dual-indexed for fast lookup)
- **design-sync** prevents drift (validates contracts before implementation)
- **plugin-lifecycle** manages deployment (install/uninstall with cache clearing)
