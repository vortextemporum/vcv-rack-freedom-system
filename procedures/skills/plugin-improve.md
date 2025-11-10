# plugin-improve

**Purpose:** Fix bugs and add features to completed plugins with versioning, backups, and changelog automation.

---

## What It Does

Makes changes to existing plugins while managing version history, creating backups, documenting changes, and handling git workflow automatically.

## When Invoked

**User says (directive):**
- "Fix [plugin]"
- "Add [feature] to [plugin]"
- "Change [specific thing] in [plugin]"
- "Update [plugin] to do [X]"

**Slash command:**
```bash
/improve [PluginName]
/improve [PluginName] "specific change"
```

**Prerequisites:**
- Plugin must exist
- Status must be ✅ (Working) or 📦 (Installed)
- NOT 🚧 (In Development) - use `/continue` instead

## Two Entry Modes

### Exploratory (vague request)
"Make the filters better in MicroGlitch"

**System offers:**
```
This is vague. Would you like to:
1. Brainstorm approaches first (plugin-ideation)
2. Implement with my best assumptions
```

### Directive (specific request)
"Add preset system to MicroGlitch with dropdown menu"

**System proceeds to Phase 0.5 (Investigation)**

## Workflow Phases

### Phase 0: Exploration (Optional)
Only for vague requests where user chooses to brainstorm first.

**Routes to:** `plugin-ideation` skill in improvement mode

**Creates:** `plugins/[PluginName]/.ideas/improvements/[feature-name].md`

**Returns to plugin-improve** when ready to implement

### Phase 0.5: Investigation
**Purpose:** Prevent band-aid fixes by finding root causes

**Three-tier research protocol:**

**Tier 1: Basic Code Inspection (5-10 min)**
- Quick file read
- Obvious issue check
- Known pattern match

**Use for:** Cosmetic changes, simple fixes, typos

**Tier 2: Root Cause Analysis (15-30 min)**
- Logic flow trace
- Integration point check
- Related code inspection

**Use for:** Logic errors, parameter issues, integration bugs

**Tier 3: Deep Research (30-60 min)**
- Invokes `deep-research` skill
- Multi-source investigation
- Architectural analysis

**Use for:** Complex bugs, performance issues, unknown errors

**Presents findings:**
```markdown
## Investigation Findings

### Problem Analysis
[What's actually wrong and why]

### Root Cause
[Technical explanation]

### Affected Files
- plugins/[Name]/Source/PluginProcessor.cpp:123
- plugins/[Name]/Source/PluginEditor.cpp:45

### Recommended Approach
[How to fix properly]

### Alternative Approaches
[Other options with trade-offs]

Proceed with implementation? [Y/n]
```

**Waits for approval before changing code**

### Phase 1: Pre-Implementation Checks

**Loads current state:**
- Reads `CHANGELOG.md` for version history
- Checks `PLUGINS.md` for status
- Reviews recent git commits
- Identifies current version (e.g., v1.2)

**Determines version bump:**
- **Patch (v1.2 → v1.2.1):** Bug fixes, cosmetic changes
- **Minor (v1.2 → v1.3):** New features, enhancements
- **Major (v1.2 → v2.0):** Breaking changes, redesigns

### Phase 2: Backup Creation

**Before ANY changes:**
```bash
cp -r plugins/[PluginName] backups/[PluginName]-v[X.Y]-$(date +%Y%m%d-%H%M%S)
```

**Backup includes:**
- All source files
- Build configuration
- UI files
- Documentation

**Purpose:** Rollback capability if something goes wrong

### Phase 3: Implementation

Makes the requested changes:
- Modifies source files
- Updates build config if needed
- Adjusts UI if required
- Updates parameter definitions

**Follows juce-foundation rules:**
- Real-time safety
- API correctness
- Best practices

### Phase 4: CHANGELOG Update

**Automatically updates:**
```markdown
# Changelog

## [1.3.0] - 2025-11-08

### Added
- Preset system with dropdown menu
- Factory presets: Clean, Warm, Aggressive
- Preset save/load functionality

### Changed
- UI layout: Added preset selector at top
- Increased window height to 500px (was 450px)

### Fixed
- (none this release)

## [1.2.0] - 2025-11-01
[Previous version entries...]
```

Follows [Keep a Changelog](https://keepachangelog.com/) format

### Phase 5: Build & Test

**Runs:**
1. Build with `build-automation` skill
2. Automated tests with `plugin-testing` skill
3. Offers manual testing checklist

**If build fails:** Failure protocol (investigate/show/wait)

**If tests fail:** Reports issues, offers investigation

### Phase 6: Git Workflow

**Stages changes:**
```bash
git add plugins/[PluginName]/
```

**Commits with version:**
```bash
git commit -m "improve: [PluginName] v[X.Y] - [description]"
```

**Tags release:**
```bash
git tag -a v[X.Y] -m "[PluginName] v[X.Y]"
```

**Example:**
```bash
git commit -m "improve: MicroGlitch v1.3 - add preset system"
git tag -a v1.3 -m "MicroGlitch v1.3"
```

### Phase 7: Installation

**Offers to install:**
```
Build successful. Install to system folders?
```

If yes: Runs `./scripts/build-and-install.sh [PluginName]`

## Version Management

### Semantic Versioning

**Format:** vMAJOR.MINOR.PATCH

**Increment rules:**
- **MAJOR:** Breaking changes (preset format change, parameter removal)
- **MINOR:** New features (added parameters, new capabilities)
- **PATCH:** Bug fixes (no new features)

**System auto-determines** based on change description:
- Contains "fix", "bug", "crash" → Patch
- Contains "add", "new", "feature" → Minor
- Contains "breaking", "remove", "redesign" → Major (asks confirmation)

### Backward Compatibility

**System checks:**
- Are parameter IDs changing? (breaking)
- Are parameters being removed? (breaking)
- Is state format changing? (breaking)

**If breaking change detected:**
```
⚠️ Warning: This appears to be a breaking change

Changes:
- Removing parameter "oldGain"
- Renaming "delay" to "delayTime"

This will break existing presets.

Recommended: Major version bump (v1.5 → v2.0)

Proceed? [y/N]
```

## CHANGELOG Format

**Structure:**
```markdown
# Changelog

All notable changes to [PluginName] documented here.

Format: [Keep a Changelog](https://keepachangelog.com/)
Versioning: [Semantic Versioning](https://semver.org/)

## [Unreleased]
(Changes in development)

## [1.3.0] - 2025-11-08

### Added
- New features and capabilities

### Changed
- Modifications to existing features

### Fixed
- Bug fixes

### Removed
- Deprecated features removed

## [1.2.0] - 2025-11-01
...
```

**Auto-populated** based on changes made

## Examples

### Bug Fix (Patch)

**Request:** "Fix the clicking noise in DelayPlugin when changing delay time"

**Investigation:** Tier 2 (Root cause: missing parameter smoothing)

**Changes:**
- Added `juce::SmoothedValue` for delay time parameter
- Smoothing ramp: 50ms

**Version:** v1.2.0 → v1.2.1

**CHANGELOG:**
```markdown
## [1.2.1] - 2025-11-08
### Fixed
- Clicking noise when changing delay time parameter (added smoothing)
```

**Commit:** `improve: DelayPlugin v1.2.1 - fix parameter clicking`

### Feature Addition (Minor)

**Request:** "Add wet/dry mix to ReverbPlugin"

**Investigation:** Tier 1 (straightforward addition)

**Changes:**
- Added "mix" parameter (0-100%)
- Updated processBlock() with mix blend
- Added knob to UI

**Version:** v1.1.0 → v1.2.0

**CHANGELOG:**
```markdown
## [1.2.0] - 2025-11-08
### Added
- Wet/dry mix parameter (0-100%)
- Mix knob in UI (bottom right)

### Changed
- UI height increased to 420px to fit new control
```

**Commit:** `improve: ReverbPlugin v1.2.0 - add wet/dry mix`

### Major Redesign (Major)

**Request:** "Redesign MicroGlitch UI with new layout"

**Investigation:** Tier 1 (UI change, not logic change)

**Changes:**
- Complete UI redesign
- New WebView mockup (v3)
- Updated component organization

**Version:** v1.5.0 → v2.0.0

**CHANGELOG:**
```markdown
## [2.0.0] - 2025-11-08
### Changed
- **BREAKING:** Complete UI redesign with new layout
- Window size: 700×500 (was 600×400)
- Controls reorganized into logical sections

Note: Presets from v1.x are compatible (parameters unchanged)
```

**Commit:** `improve: MicroGlitch v2.0 - UI redesign`

## Integration

**Calls:**
- `plugin-ideation` (if user chooses to brainstorm first)
- `deep-research` (for Tier 3 investigation)
- `build-automation` (for building)
- `plugin-testing` (for validation)

**Creates:**
- Updated source files
- Updated CHANGELOG.md
- Backup in `backups/`
- Git commit with version tag

**Updates:**
- `PLUGINS.md` (version number)
- `.continue-here.md` (if improvement incomplete)

## Safety Features

### Automatic Backups
Every improvement creates timestamped backup before changes

### Investigation First
Never jumps straight to coding—always investigates root cause

### Approval Gates
Presents findings and waits for confirmation before implementing

### Rollback Capability
```bash
# If v1.3 has problems, rollback to v1.2:
cp -r backups/[PluginName]-v1.2-20251107-143000/* plugins/[PluginName]/
git reset --hard v1.2
./scripts/build-and-install.sh [PluginName]
```

### Version Tags
Git tags make it easy to checkout specific versions

## Best Practices

**Be specific in requests:** "Add preset system" is better than "make it better"

**One improvement at a time:** Don't bundle multiple features in one improvement

**Review investigation findings:** Understand the root cause before proceeding

**Test after implementation:** Don't skip the testing phase

**Keep breaking changes to minimum:** Preserve backward compatibility when possible

## Success Output

```
✓ Investigation complete (Tier 2: Root cause analysis)
✓ Backup created: backups/DelayPlugin-v1.2-20251108-153045/
✓ Changes implemented
✓ CHANGELOG updated (v1.2 → v1.3)
✓ Build successful
✓ Tests passed (5/5)
✓ Committed: improve: DelayPlugin v1.3 - add tempo sync (abc1234)
✓ Tagged: v1.3
✓ Installed to system folders

DelayPlugin v1.3 ready to use.
```

Complete version management with full traceability.
