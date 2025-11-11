<objective>
Safely migrate the Plugin Freedom System from the current .claude/ directory structure to the correct Claude Code plugin distribution structure at the repository root. This migration prepares the system for distribution as a Claude Code plugin that users can install via GitHub marketplace.

This is critical for proper plugin distribution - Claude Code requires specific directory locations for skills, commands, agents, and hooks to function correctly when installed as a plugin.

**Safety approach:** This prompt uses a branch-based workflow with full testing and rollback capability.
</objective>

<context>
Project: Plugin Freedom System - a comprehensive JUCE audio plugin development workflow
Current structure: Components in .claude/ subdirectory (local development pattern)
Target structure: Components at root level (plugin distribution pattern)
Platform: macOS initially, cross-platform later
Purpose: Enable distribution via GitHub so users can install with: /plugin marketplace add lexchristopherson/plugin-freedom-system

Key insight: Everything currently works in .claude/ for local development, but plugin distribution requires root-level directories. Most components need to move, but the content stays identical.

**Risk mitigation:** Work on a dedicated branch, test thoroughly, merge only after confirming everything works. Can delete branch and return to main if anything goes wrong.
</context>

<requirements>
1. Create safety checkpoint before any changes
2. Work on isolated branch (can discard if needed)
3. Move all plugin components from .claude/ to repository root while preserving structure
4. Create .claude-plugin/plugin.json with correct metadata
5. Update all internal references to reflect new paths
6. Configure hooks for plugin distribution
7. Update .gitignore to exclude personal audio plugins
8. Test all functionality before merging
9. Keep .claude/ directory minimal (only settings.json if needed for local config)
10. Ensure CLAUDE.md moves to root as system documentation
11. Document rollback procedure
</requirements>

<safety_protocol>

## Step 0: Pre-Migration Safety Checkpoint

**Create backup branch and verify clean state:**

```bash
# Verify no uncommitted changes
git status

# If there are changes, commit them first:
# git add -A && git commit -m "chore: save state before migration"

# Create migration branch
git checkout -b plugin-structure-migration

# Confirm you're on the new branch
git branch --show-current
# Should show: plugin-structure-migration
```

**If anything goes wrong during migration:**
```bash
# Discard all changes and return to main
git checkout main
git branch -D plugin-structure-migration
# You're back where you started, nothing lost
```

</safety_protocol>

<migration_steps>

## Phase 1: Create Plugin Metadata

Create `.claude-plugin/plugin.json`:
```bash
mkdir -p .claude-plugin
```

Write to `.claude-plugin/plugin.json`:
```json
{
  "name": "plugin-freedom-system",
  "description": "Complete JUCE audio plugin development workflow with 7-stage pipeline, WebView UI system, and automated testing",
  "version": "1.0.0",
  "author": "Lex Christopherson",
  "homepage": "https://github.com/lexchristopherson/plugin-freedom-system",
  "license": "MIT"
}
```

**Verify:**
```bash
cat .claude-plugin/plugin.json | python3 -m json.tool
# Should pretty-print valid JSON
```

## Phase 2: Move Components to Root

**Use git mv to preserve history:**

```bash
# Move skills
git mv .claude/skills skills

# Move commands
git mv .claude/commands commands

# Move agents
git mv .claude/agents agents

# Move CLAUDE.md
git mv .claude/CLAUDE.md CLAUDE.md

# Move aesthetics directory (empty it first, keep structure)
mkdir -p aesthetics
cp .claude/aesthetics/README.md aesthetics/
git add aesthetics/README.md

# Create hooks directory and move scripts
mkdir -p hooks
git mv .claude/hooks/SessionStart.sh hooks/
git mv .claude/hooks/PostToolUse.sh hooks/
git mv .claude/hooks/PreCompact.sh hooks/
```

**Check the moves:**
```bash
git status
# Should show "renamed:" entries, not "deleted:" and "new file:"
```

## Phase 3: Create Hooks Configuration

Create `hooks/hooks.json` for plugin distribution:

```json
{
  "SessionStart": [
    {
      "hooks": [
        {
          "type": "command",
          "command": "${CLAUDE_PLUGIN_ROOT}/hooks/SessionStart.sh",
          "timeout": 5000,
          "description": "Validate development environment at session start"
        }
      ]
    }
  ],
  "PostToolUse": [
    {
      "matcher": "Write|Edit",
      "hooks": [
        {
          "type": "command",
          "command": "${CLAUDE_PLUGIN_ROOT}/hooks/PostToolUse.sh",
          "timeout": 2000,
          "description": "Real-time code quality validation for JUCE best practices"
        }
      ]
    }
  ],
  "PreCompact": [
    {
      "hooks": [
        {
          "type": "command",
          "command": "${CLAUDE_PLUGIN_ROOT}/hooks/PreCompact.sh",
          "timeout": 10000,
          "description": "Preserve all contract files before context compaction"
        }
      ]
    }
  ]
}
```

**Important:** Use `${CLAUDE_PLUGIN_ROOT}` instead of `${CLAUDE_PROJECT_DIR}` for plugin distribution.

## Phase 4: Update Path References in Files

**In CLAUDE.md at root:**
Replace all occurrences:
- `.claude/skills/` → `skills/`
- `.claude/commands/` → `commands/`
- `.claude/agents/` → `agents/`
- `.claude/hooks/` → `hooks/`

**Search for any remaining references:**
```bash
grep -r "\.claude/" --exclude-dir=.git --exclude="settings.json" . | grep -v ".claude-plugin"
# Should return minimal results or nothing
```

**Update hook scripts if they reference .claude/ paths:**
Check each hook script and update any internal paths if needed.

## Phase 5: Update .gitignore

Add or update in `.gitignore`:
```
# Personal audio plugins (not distributed)
plugins/*/

# Keep example plugins if desired
!plugins/README.md

# Personal aesthetic templates (not distributed)
aesthetics/*/
!aesthetics/README.md

# Local development settings
.claude/settings.json
.claude/aesthetics/

# Build artifacts
*.o
*.d
Builds/
build/
**/Builds/
**/build/

# OS files
.DS_Store
**/.DS_Store
```

## Phase 6: Clean Up .claude/ Directory

**Evaluate what's left:**
```bash
ls -la .claude/
```

**What to keep in .claude/:**
- `.claude/settings.json` - Local development configuration (keep)
- `.claude/aesthetics/` - Your personal aesthetic templates (keep locally, not distributed)

**The .claude/ directory after migration:**
```
.claude/
├── settings.json        # Local config only
└── aesthetics/          # Your personal aesthetics (gitignored)
    ├── swiss-minimal-001/
    └── vintage-hardware-001/
```

Users will have an empty `aesthetics/` at root to store their own templates.

**Remove empty subdirectories if any:**
```bash
find .claude -type d -empty -delete
```

## Phase 7: Stage Changes

```bash
# Stage all moves and new files
git add -A

# Review what will be committed
git status
git diff --cached --stat

# Should see:
# - Renamed: .claude/skills -> skills
# - Renamed: .claude/commands -> commands
# - Renamed: .claude/agents -> agents
# - Renamed: .claude/CLAUDE.md -> CLAUDE.md
# - New: .claude-plugin/plugin.json
# - New: hooks/hooks.json
# - Modified: CLAUDE.md (path updates)
# - Modified: .gitignore
```

**Commit the migration:**
```bash
git commit -m "refactor: migrate to plugin distribution structure

- Move skills, commands, agents, hooks to root for Claude Code plugin distribution
- Create .claude-plugin/plugin.json for plugin metadata
- Configure hooks for plugin distribution with \${CLAUDE_PLUGIN_ROOT}
- Update CLAUDE.md path references
- Update .gitignore to exclude personal plugins

This structure enables distribution via: /plugin marketplace add lexchristopherson/plugin-freedom-system"
```

</migration_steps>

<testing_phase>

## Phase 8: Comprehensive Testing

**Test before merging to main. If ANY test fails, investigate and fix before proceeding.**

### Test 1: Command Discovery
```bash
# Trigger command autocomplete (type / in Claude)
# Verify all commands appear: /dream, /implement, /improve, etc.
```

### Test 2: Command Execution
```bash
# Test a simple command
/help

# Should work without errors
```

### Test 3: Skill Invocation
Try a skill:
```
Can you list all available plugin-workflow stages?
```
Claude should be able to access the skill from `skills/plugin-workflow/`.

### Test 4: Hook Verification (if applicable)
Create a test file and verify hooks trigger:
```bash
echo "test" > test.txt
# PostToolUse hook should run (check for validation output)
rm test.txt
```

### Test 5: Path Reference Check
```bash
# Verify no broken references
cat CLAUDE.md | grep "\.claude/" | grep -v "\.claude-plugin"
# Should return nothing or only valid references
```

### Test 6: Plugin Structure Validation
```bash
# Verify all required directories exist at root
ls -ld skills commands agents hooks .claude-plugin

# Verify plugin.json is valid
cat .claude-plugin/plugin.json | python3 -m json.tool
```

### Test 7: Git History Preservation
```bash
# Verify moves preserved history (not delete+add)
git log --follow skills/plugin-workflow/SKILL.md
# Should show history from when it was in .claude/skills/
```

</testing_phase>

<merge_or_rollback>

## Option A: All Tests Pass → Merge

```bash
# Switch back to main
git checkout main

# Merge the migration
git merge plugin-structure-migration

# Delete the migration branch
git branch -d plugin-structure-migration

# Verify on main
git log -1
git status
```

**Success! Your repository is now ready for plugin distribution.**

## Option B: Tests Fail → Rollback

```bash
# Switch back to main (discards all migration work)
git checkout main

# Delete the failed migration branch
git branch -D plugin-structure-migration

# You're back to the original state, nothing changed
```

**Investigate the failures, fix the prompt, try again on a new branch.**

## Option C: Partial Success → Fix on Branch

```bash
# Stay on plugin-structure-migration branch
# Fix the specific issues
# Re-test
# When all tests pass, proceed to Option A
```

</merge_or_rollback>

<final_structure>

After successful migration and merge, your repository structure:

```
plugin-freedom-system/                    (on main branch)
├── .claude-plugin/
│   └── plugin.json                       ✓ NEW
├── skills/                               ✓ MOVED from .claude/
│   ├── plugin-workflow/
│   ├── plugin-ideation/
│   ├── ui-mockup/
│   └── ... (all skills)
├── commands/                             ✓ MOVED from .claude/
│   ├── dream.md
│   ├── implement.md
│   └── ... (all commands)
├── agents/                               ✓ MOVED from .claude/
│   ├── foundation-agent/
│   ├── dsp-agent/
│   └── ... (all agents)
├── hooks/
│   ├── hooks.json                        ✓ NEW
│   ├── SessionStart.sh                   ✓ MOVED
│   ├── PostToolUse.sh                    ✓ MOVED
│   └── PreCompact.sh                     ✓ MOVED
├── aesthetics/
│   └── README.md                         ✓ COPIED (empty structure for users)
├── scripts/                              ✓ (already at root)
├── troubleshooting/                      ✓ (already at root)
├── CMakeLists.txt                       ✓ (already at root)
├── PLUGINS.md                           ✓ (already at root)
├── CLAUDE.md                            ✓ MOVED from .claude/
├── README.md                            ✓ (update after migration)
├── .gitignore                           ✓ UPDATED
└── plugins/                             ✓ (gitignored)

Optionally remaining:
.claude/
├── settings.json                        ✓ (local config only)
└── aesthetics/                          ✓ (if exists)
```

</final_structure>

<success_criteria>

Migration is successful when ALL of these are true:

1. ✓ All skills, commands, agents work identically after move
2. ✓ .claude-plugin/plugin.json exists with valid JSON
3. ✓ Hooks are configured with ${CLAUDE_PLUGIN_ROOT} references
4. ✓ No broken references to .claude/ paths (except settings.json)
5. ✓ Git shows moves as renames, preserving history
6. ✓ All tests in Testing Phase pass
7. ✓ CLAUDE.md at root with updated paths
8. ✓ Personal plugins/ directory is gitignored
9. ✓ All slash commands (/dream, /implement, etc.) function correctly
10. ✓ Changes merged to main branch
11. ✓ Migration branch deleted (cleanup complete)

</success_criteria>

<rollback_documentation>

## If You Need to Rollback After Merge

**If you merged but later discover issues:**

```bash
# Find the commit before merge
git log --oneline -5

# Assuming merge was last commit
git revert HEAD

# Or hard reset (destructive):
git reset --hard HEAD~1
```

**If you pushed to remote:**
```bash
# Revert the merge commit
git revert -m 1 HEAD
git push origin main
```

**Nuclear option (if nothing else works):**
```bash
# You can always restore from GitHub if you had pushed previously
git fetch origin
git reset --hard origin/main
```

</rollback_documentation>

<distribution_next_steps>

After successful migration and merge:

1. **Test plugin installation locally:**
   ```bash
   /plugin marketplace add /Users/lexchristopherson/Developer/plugin-freedom-system
   /plugin install plugin-freedom-system
   ```

2. **Push to GitHub:**
   ```bash
   git push origin main
   ```

3. **Users can now install:**
   ```bash
   /plugin marketplace add lexchristopherson/plugin-freedom-system
   /plugin install plugin-freedom-system
   ```

4. **Future updates:**
   - Make changes to skills/commands/agents
   - Bump version in .claude-plugin/plugin.json
   - Push to GitHub
   - Users run: `/plugin marketplace update` to get updates

</distribution_next_steps>