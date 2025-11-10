# /improve

**Purpose:** Fix bugs and add features to completed plugins with automatic versioning and backups.

---

## Syntax

```bash
/improve                              # Interactive menu
/improve [PluginName]                 # Select plugin, ask what to improve
/improve [PluginName] [description]   # Direct implementation
```

## What It Does

Modifies completed plugins with full version control: automatic version bumps, backups, CHANGELOG updates, git commits, git tags, building, and installation.

**Handles:**
- Bug fixes (PATCH version bump)
- New features (MINOR version bump)
- Breaking changes (MAJOR version bump - rare)
- Versioning and backups
- CHANGELOG.md updates
- Git commits and tags
- Build and install

## Preconditions

**Plugin must be:**
- Status ✅ Working OR 📦 Installed in PLUGINS.md
- NOT 🚧 In Development (use `/continue` instead)
- NOT ❌ Deprecated

## How It Works

### Without Plugin Name

Lists completed plugins:

```
Which plugin would you like to modify?

1. MicroGlitch v1.2.0 - Glitch effect (✅ Working)
2. ClapPlugin808 v1.0.0 - 808 clap synth (📦 Installed)
3. SimpleDelay v0.9.0 - Delay effect (✅ Working)

Reply with number:
```

Then asks what to improve.

### With Plugin, No Description

Presents options:

```
What would you like to improve in MicroGlitch?

1. From existing brief (preset-system.md found)
2. Describe the change
```

If improvement brief exists from `/dream`, you can implement it directly.

### With Vague Description

Example: `/improve MicroGlitch improve the filters`

Detects vagueness and offers choice:

```
Your request is a bit vague. How should I proceed?

1. Brainstorm approaches first
   Explore different ways to improve filters → creates brief

2. Just implement something reasonable
   I'll use my judgment and make improvements now
```

**Option 1:** Chains to `/dream` → creates improvement brief → return to implement later

**Option 2:** Proceeds with Phase 0.5 investigation, makes reasonable choices

### With Specific Description

Example: `/improve MicroGlitch add resonance parameter to state variable filter with range 0-1`

Routes directly to implementation with Phase 0.5 investigation.

## Phase 0.5: Investigation

**Before implementing, plugin-improve investigates:**

Three-tier investigation system:
1. **Basic inspection** - Read relevant code, understand current implementation
2. **Root cause analysis** - Understand why bug exists or how to add feature properly
3. **Deep research** - Invoke deep-research skill for complex issues

**Skipped only for:**
- Cosmetic UI changes (color, size, position)
- Simple parameter value tweaks

**Output:**
```
Investigation complete:

Root cause: [Technical explanation]
Affected components: [Files and functions]
Implications: [What needs to change]

Recommended approach:
[Specific implementation plan]

Ready to implement?
```

Wait for approval before proceeding.

## What The Skill Does

**Routes to:** `plugin-improve` skill

**Automatic workflow:**
1. **Investigation** (Phase 0.5) - Understand problem/change
2. **Approval** - Wait for confirmation
3. **Version selection** - Ask PATCH/MINOR/MAJOR
4. **Backup** - Copy current version to `backups/[Plugin]/v[X.Y.Z]/`
5. **Implementation** - Make code changes
6. **CHANGELOG** - Update with version and description
7. **Git commit** - Conventional format with version
8. **Git tag** - Tag release (v[X.Y.Z])
9. **Build** - Compile Release mode
10. **Install** - Deploy to system folders
11. **Test prompt** - Suggest testing in DAW

## After Successful Improvement

```
✓ Backup created: backups/MicroGlitch/v1.2.0/
✓ Changes implemented
✓ CHANGELOG.md updated
✓ Version bumped: v1.2.0 → v1.3.0
✓ Committed: improve: MicroGlitch v1.3.0 - add resonance (abc1234)
✓ Tagged: v1.3.0
✓ Built and installed to system folders

Next steps:
- Test in DAW (Ableton/Logic)
- /test MicroGlitch for automated stability tests
- /improve MicroGlitch again if you find more issues
```

## Vagueness Detection

**Request IS vague if it lacks:**
- Specific feature name
- Specific action
- Acceptance criteria

**Examples of vague requests:**
- "improve the filters" (improve how?)
- "better presets" (what makes them better?)
- "UI feels cramped" (what specific changes?)
- "not working in Logic" (which parameter? what behavior?)

**Examples of specific requests:**
- "add resonance parameter to state variable filter, range 0-1"
- "fix bypass parameter - it's not muting audio"
- "increase window height from 400px to 500px"
- "add factory presets for drums, bass, and pads"

## Brainstorm vs. Implement

**Choose "Brainstorm first" when:**
- You're not sure of the best approach
- Multiple solutions possible
- Want to explore options before committing
- Major feature addition

**Choose "Implement now" when:**
- You know what you want
- Simple, straightforward change
- Bug fix with obvious solution
- Trust the investigation phase to figure it out

## Error Handling

**Plugin in development:**
```
GranularGlitch is still in development (Stage 3).
Use /continue GranularGlitch to resume the workflow.
```

**Plugin not found:**
```
PluginName not found in PLUGINS.md.
Create it first with /implement PluginName.
```

**Plugin deprecated:**
```
OldPlugin is deprecated.
Restore from backups/ first if you want to work on it.
```

## Routes To

**Primary:** `plugin-improve` skill

**Conditional:** `plugin-ideation` skill (if "brainstorm first" selected)

## Examples

**Example 1: Interactive selection**
```
User: /improve
Assistant: [Lists 3 completed plugins]
User: [Selects MicroGlitch]
Assistant: What would you like to improve?
User: [Types "Add MIDI learn"]
Assistant: [Routes to plugin-improve → investigates → implements]
```

**Example 2: Vague request**
```
User: /improve MicroGlitch improve the filters
Assistant: Your request is vague. How should I proceed?
          1. Brainstorm approaches first
          2. Just implement something reasonable
User: [Selects 1]
Assistant: [Chains to /dream → creates brief → returns]
          Brief created at improvements/better-filters.md
          Run /improve MicroGlitch again to implement.
```

**Example 3: Specific request**
```
User: /improve MicroGlitch add resonance to filter, range 0-1
Assistant: [Investigates → shows findings → waits for approval]
User: Looks good
Assistant: [Implements → versions → commits → builds → installs]
```

**Example 4: From existing brief**
```
User: /improve MicroGlitch
Assistant: What would you like to improve?
          1. From existing brief (preset-system.md)
          2. Describe the change
User: [Selects 1]
Assistant: [Loads brief → implements preset system]
```

## Natural Language Equivalent

These phrases work the same:
- "Fix [bug] in MicroGlitch"
- "Add [feature] to MicroGlitch"
- "Change [aspect] in MicroGlitch"

## Related Commands

- `/dream` - Explore improvements before implementing
- `/test` - Validate changes after improvement
- `/continue` - Resume in-progress development

## When To Use

**Use /improve when:**
- Plugin is complete (✅ or 📦 status)
- You want to fix bugs
- You want to add features
- You want to enhance existing functionality

**Don't use /improve when:**
- Plugin is in development (use `/continue`)
- Starting new plugin (use `/implement`)
- Just exploring ideas (use `/dream`)

## Tips

**Be specific:** Clear descriptions skip the ambiguity handling.

**Use briefs:** Create improvement briefs with `/dream`, then implement with `/improve`.

**Trust investigation:** Phase 0.5 prevents band-aid fixes.

**Test after changes:** Always validate in DAW or with `/test`.

**Version control:** Every change creates backup and git tag—safe to experiment.

## Output

Changes are production-ready:
- Versioned and backed up
- Built in Release mode
- Installed to system folders
- Documented in CHANGELOG
- Git history preserved
- Ready to test immediately
