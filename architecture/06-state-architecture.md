## State Architecture

### State Taxonomy

The system manages 4 types of state:

#### 1. Plugin Lifecycle State (PLUGINS.md Registry)

**Location:** `PLUGINS.md` (project root)

**Purpose:** Single source of truth for all plugins - used by routing logic to determine workflow selection

**Architecture:** Human-readable catalog that tracks every plugin from ideation to installation

**States:**
- `💡 Ideated` - Creative brief exists, no implementation
  - Filesystem: `.ideas/creative-brief.md` exists, no `Source/`
  - Next action: Start plugin-workflow Stage 0 or finalize mockup

- `🚧 Stage N.M` - In development (with stage/phase number)
  - Filesystem: `Source/` exists, `.continue-here.md` exists, not validated
  - Next action: `/continue [PluginName]` to resume workflow
  - **CRITICAL:** If 🚧, ONLY plugin-workflow can modify (not plugin-improve)

- `✅ Working` - Completed Stage 6, not installed
  - Filesystem: Build successful, validation passed, NOT in system folders
  - Next action: plugin-lifecycle to install OR plugin-improve to enhance

- `📦 Installed` - Deployed to system folders
  - Filesystem: Everything from ✅ + copied to `~/Library/Audio/Plug-Ins/`
  - Shows: Installation paths and timestamps
  - Next action: plugin-improve for versioned changes

- `🐛 Has Issues` - Known problems (secondary status, combines with others)
  - Shows issue description in "Known Issues" section

- `🔬 Testing` - Manual validation in progress (rare, most skip this)
  - Shows test checklist progress

- `🗑️ Archived` - Deprecated, directory deleted, entry kept for history

**Entry Structure (Full):**
```markdown
### [PluginName]
**Status:** 📦 **Installed**
**Installed:** YYYY-MM-DD
**Version:** X.Y.Z
**Formats:** VST3, AU
**JUCE Version:** X.Y.Z
**Created:** YYYY-MM-DD

**Locations:**
- VST3: `~/Library/Audio/Plug-Ins/VST3/[Name].vst3`
- AU: `~/Library/Audio/Plug-Ins/Components/[Name].component`

**Type:** [Audio Effect | MIDI Instrument | Synth]
- [Brief description]

**Parameters ([N] total):**
- [Param1]: [range], default [value]
- [Param2]: [range], default [value]

**DSP:** [Architecture description]

**GUI:** [Design description]

**Lifecycle Timeline:**
- **YYYY-MM-DD (Stage 0):** Research completed
- **YYYY-MM-DD (Stage 1):** Planning - Complexity N
- ...
- **YYYY-MM-DD (Stage 6):** Validation passed

**Validation:**
- ✓ VST3: All pluginval tests passed
- ✓ AU: All auval tests passed

**Known Issues:**
- None

**Last Updated:** YYYY-MM-DD
**Build Log:** `logs/[PluginName]/build_*.log`
**Validation:** `logs/[PluginName]/pluginval_*.log`
```

**Routing Importance:**

CLAUDE.md checks PLUGINS.md to route correctly:
```
Request: "Fix ReverbPlugin"
→ Read PLUGINS.md: status = 📦 Installed
→ Route to: plugin-improve ✓ (not plugin-workflow)

Request: "Continue Dreamcatcher"
→ Read PLUGINS.md: status = 🚧 Stage 4.4/6
→ Route to: context-resume → plugin-workflow ✓
```

**Who Updates:**
- plugin-workflow: Creates 🚧, updates timeline, changes to ✅
- plugin-lifecycle: Changes ✅ → 📦, adds locations
- plugin-improve: Updates version, timeline, timestamp
- plugin-ideation: Creates 💡 entry

**State Machine:**

```
┌─────────┐
│ Initial │
└────┬────┘
     │ /dream
     ▼
┌─────────┐
│💡 Ideated│
└────┬────┘
     │ /implement
     ▼
┌─────────────┐
│🚧 Stage 0   │
└──────┬──────┘
       │ Stage 0 complete
       ▼
┌─────────────┐
│🚧 Stage 1   │
└──────┬──────┘
       │ ...
       ▼
┌─────────────┐
│🚧 Stage 6   │
└──────┬──────┘
       │ Stage 6 complete
       ▼
┌─────────┐
│✅ Working│
└────┬────┘
     │ /install-plugin
     ▼
┌──────────┐
│📦Installed│
└────┬─────┘
     │ /improve
     ▼
┌─────────────┐
│🚧 Improving │
└──────┬──────┘
       │ Improvement complete
       ▼
┌──────────┐
│📦Installed│ (new version)
└──────────┘
```

**Ownership:** Skills update this file (via Claude following skill instructions)

**Persistence:** Git-tracked, survives sessions

#### 2. Session State (.continue-here.md Handoff)

**Location:** `plugins/[Name]/.continue-here.md`

**Purpose:** Enable session continuity by documenting current progress and next actions

**Architecture:** Structured handoff document for resuming interrupted workflows

**Contains:**
- Current stage/phase with precise progress indicator
- Last action taken (what was just completed)
- What exists now (complete inventory of working components)
- Suggested next steps (prioritized action list)
- Quick reference commands

**Created by:**
- All workflows when pausing (plugin-workflow, plugin-improve)
- After each stage/phase completion
- When build/validation fails (to record error state)
- When user manually pauses ("I'll come back to this")

**Deleted when:**
- Plugin reaches ✅ Working status (Stage 6 complete)
- Plugin reaches 📦 Installed status
- User explicitly archives plugin

**Handoff Contract:**

context-resume skill reads this file to:
1. Identify exact continuation point
2. Load relevant contracts (parameter-spec, architecture, plan)
3. Present context summary to user
4. Resume workflow at precise stage/phase

**Full Structure:**
```markdown
# Continue: [PluginName]

**Last updated:** YYYY-MM-DD HH:MM
**Current state:** Stage N.M complete
**Last action:** [Specific action completed]

---

## What We Have Now

### Completed Stages
- Stage 0: Research → architecture.md
- Stage 1: Planning → plan.md (Complexity N)
- Stage 2: Foundation → Build system verified
- Stage 3: Shell → [N] parameters implemented
- Stage 4.1: Core DSP → [Component] functional
- Stage 4.2: Modulation → [LFO/envelope] implemented

### Current State
- Build: ✓ Compiles successfully
- Test: ✓ Loads in DAW
- Sound: [Description of current sonic behavior]

### Files Modified Recently
- `Source/PluginProcessor.cpp` (Stage 4.2)
- `Source/PluginProcessor.h` (Stage 4.2)

---

## Suggested Next Steps

1. ⭐ **Continue to Stage 4.3** - [Next phase description]
   - Implement [specific components]
   - Expected completion: [estimate]

2. **Test current implementation**
   - Load in DAW and verify [specific aspects]
   - Check [parameters] respond correctly

3. **Review code quality**
   - Check for allocations in processBlock()
   - Verify denormal protection

4. **Read research notes**
   - Review `research.md` for [specific algorithm details]

---

## Known Issues

- [Issue 1]: [Description and potential fix]
- [Issue 2]: [Description and potential fix]

---

## Quick Commands

Resume automation: `/continue [PluginName]`
Test in DAW: `/test [PluginName]`
Show standalone: `/show-standalone [PluginName]`
Manual build: `cmake --build plugins/[PluginName]/build --config Release`

---

## Context for Next Session

**Contracts:**
- parameter-spec.md: [N] parameters defined
- architecture.md: [Key DSP components]
- plan.md: Stage 4 has [M] phases total

**Last build:** `logs/[PluginName]/build_[timestamp].log`
**Last commit:** [commit hash] - "Stage 4.2 complete - [description]"
```

**Usage Pattern:**

```
Stage 4.2 completes
  ↓
plugin-workflow updates .continue-here.md
  ↓
Git commit: "Stage 4.2 complete - Modulation implemented"
  ↓
User stops session
  ↓
[New session starts]
  ↓
User: "Continue ReverbPlugin"
  ↓
context-resume reads .continue-here.md
  ↓
Loads: parameter-spec, architecture, plan, recent code
  ↓
Presents summary: "Last left off at Stage 4.2, next is 4.3"
  ↓
User confirms
  ↓
plugin-workflow resumes at Stage 4.3
```

**Relationship to PLUGINS.md:**

.continue-here.md is **detailed**, PLUGINS.md is **summary**:
- PLUGINS.md: "🚧 Stage 4.4/6 - DSP nearing completion"
- .continue-here.md: Complete inventory, specific next actions, commands

**Error State Handoffs:**

When build/validation fails:
```markdown
## Current State
- Build: ✗ Failed (see logs/[Plugin]/build_[timestamp].log)
- Error: [Extracted error message]

## Suggested Next Steps
1. ⭐ **Investigate error** - `/troubleshoot-juce`
2. **Show error context** - Review PluginProcessor.cpp:78
3. **Fix manually** - Make changes, then "rebuild and continue"
```

**Ownership:** Active workflow (plugin-workflow or plugin-improve)

**Persistence:** Session-local (deleted on completion)

#### 3. Version State (Permanent)

**Location:** `plugins/[Name]/CHANGELOG.md`

**Format:** Keep a Changelog + Semantic Versioning

**Example:**
```markdown
# Changelog

## [1.2.0] - 2025-11-09

### Added
- Stereo width parameter
- New "Wide" preset

### Changed
- Increased reverb tail length to 10s (was 5s)
```

**Ownership:** plugin-improve skill

**Persistence:** Git-tracked, permanent history

#### 4. Logs and Backups (Build Artifacts)

**Architecture:** Comprehensive logging and version backup system

**Directory Structure:**
```
logs/
├── system/                  # System-wide logs
│   ├── cmake_config.log     # CMake configuration output
│   └── juce_version.txt     # JUCE version detection
└── [PluginName]/            # Per-plugin logs
    ├── build_YYYYMMDD_HHMMSS.log      # Timestamped build output
    ├── pluginval_YYYYMMDD_HHMMSS.log  # Validation results
    └── install_YYYYMMDD.log           # Installation paths/timestamps

backups/
└── [PluginName]/
    └── v[X.Y.Z]/            # Complete snapshot before improvements
        ├── Source/          # Full source code
        ├── CMakeLists.txt   # Build configuration
        ├── research.md      # Technical research
        ├── plan.md          # Implementation plan
        └── CHANGELOG.md     # Version history
```

**Logging System:**

**Build logs** (`build_*.log`):
- Created by: build-automation, plugin-workflow, plugin-improve
- Contains: Full CMake output, compiler warnings/errors, linking info, timing
- Use for: Syntax errors, missing headers, linker errors, optimization checks
- Retention: Indefinite (manually delete old logs)

**Validation logs** (`pluginval_*.log`):
- Created by: plugin-workflow Stage 6, plugin-testing, build-automation
- Contains: Pluginval strictness level, test results, pass/fail per test
- Use for: Plugin crashes, state save/restore issues, thread safety violations
- Format:
  ```
  ✓ Plugin loaded successfully
  ✓ State can be saved and restored
  ✗ No denormals detected  ← First failure
  RESULT: FAIL
  ```

**Install logs** (`install_*.log`):
- Created by: plugin-lifecycle, build-and-install.sh
- Contains: Copy operations, timestamps, DAW cache clearing, paths
- Use for: Plugin not appearing in DAW, stale version issues
- Shows: VST3/AU installation paths and verification timestamps

**Backup System:**

**When created:** plugin-improve ONLY (automatic before EVERY improvement)

**Backup structure:** Complete snapshot of plugin at specific version
- Includes: All Source/, CMakeLists.txt, docs, CHANGELOG.md
- Excludes: build/, logs/, .ideas/ (remains in main plugin directory)

**Recovery process:**
```bash
# Manual recovery from v1.0.1
cp -r backups/ReverbPlugin/v1.0.1/* plugins/ReverbPlugin/
./scripts/build-and-install.sh ReverbPlugin
# Update PLUGINS.md with revert note
```

**Retention:** Indefinite (each backup ~100-500 KB), recommended keep last 3 versions

**Error reading examples:**

Build error:
```
fatal error: 'juce_dsp/juce_dsp.h' file not found
→ Fix: Add juce::juce_dsp to target_link_libraries() in CMakeLists.txt
```

Validation error:
```
✗ Thread safety
  Memory allocation detected in processBlock()
  Location: PluginProcessor.cpp:78
→ Fix: Move allocation to prepareToPlay()
```

**Ownership:** build-automation skill, plugin-improve

**Persistence:** Not git-tracked, local only

### Ownership Model

| State Type | Owner | Readers | Writers |
|------------|-------|---------|---------|
| Plugin lifecycle | Skills (via Claude) | All workflows | Skills only |
| Session state | Active workflow | context-resume | Current workflow |
| Version state | plugin-improve | All | plugin-improve only |
| Build artifacts | build-automation | troubleshooter, verifier | build-automation |

### State Persistence Strategy

**Git-tracked (survives sessions):**
- `PLUGINS.md`
- `plugins/[Name]/CHANGELOG.md`
- `plugins/[Name]/.ideas/*`  (creative-brief, parameter-spec, architecture, mockups)
- `plugins/[Name]/plan.md`
- `plugins/[Name]/Source/*`

**Session-local (deleted on completion):**
- `plugins/[Name]/.continue-here.md`
- `logs/*`

**Backup-only (not in git):**
- `backups/[Name]/v[X.Y.Z]/` (complete snapshots before improvements)

### Consistency Guarantees

**Atomic state transitions:**
- State updates commit with code changes (single git commit)
- PLUGINS.md status updated only after stage completion verified
- Version bumps commit with CHANGELOG updates

**Validation points:**
- Validator verifies state matches reality (e.g., "Stage 3 complete" means parameters actually exist)
- Hooks prevent invalid state transitions (e.g., can't skip stages)

**Recovery:**
- Handoff files enable resume from any stage boundary
- Git history provides complete audit trail
- Backups enable rollback to any version

---

## Related Procedures

This state architecture is implemented through:

- `procedures/skills/context-resume.md` - .continue-here.md parsing and workflow resumption
- `procedures/skills/plugin-improve.md` - CHANGELOG.md version management and backup creation
- `procedures/skills/plugin-workflow.md` - PLUGINS.md state machine updates

---
