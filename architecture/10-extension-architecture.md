## Extension Architecture

### Adding New Skills

**1. Create skill directory:**
```bash
mkdir -p .claude/skills/my-new-skill
```

**2. Create SKILL.md with frontmatter:**
```yaml
---
name: my-new-skill
description: Brief description of what this skill does and when to use it
model: sonnet
allowed-tools:
  - Read
  - Write
  - Edit
  - Bash
preconditions:
  - Specific conditions that must be true
---
```

**3. Define workflow logic in markdown:**
```markdown
# my-new-skill

## Purpose
What this skill accomplishes.

## When Invoked
Trigger phrases and preconditions.

## Process
Step-by-step workflow.

## Output
What files/artifacts get created.

## Integration
How this skill interacts with others.
```

**4. Add to CLAUDE.md index:**
```markdown
## System Components

**Skills** (`.claude/skills/`)
- `plugin-workflow` - 7-stage plugin creation workflow
- `plugin-ideation` - Creative concept development
- `plugin-improve` - Versioned plugin enhancements
- `ui-mockup` - Two-phase UI design workflow
- `my-new-skill` - Brief description of what it does

...rest of CLAUDE.md...
```

**Important:** CLAUDE.md is a navigation index, not a routing table. It tells Claude WHERE skills are located, not HOW to route to them. The slash command contains the instruction to invoke the skill.

**5. Test:**
```
User: "[trigger phrase]"
→ Should route to my-new-skill
→ Skill executes workflow
→ Verify results
```

### Customizing Workflows

**Modify existing skill:**

1. Read skill's SKILL.md
2. Identify section to customize
3. Edit markdown content (not frontmatter unless changing preconditions)
4. Test with representative plugin

**Example customization:**
```markdown
<!-- In plugin-workflow/SKILL.md -->

### Stage 4: DSP

<!-- OLD: Single-pass only -->
- Implement all DSP in one go

<!-- NEW: Always use phased approach -->
- Phase 4.1: Core processing
- Phase 4.2: Modulation
- Phase 4.3: Effects
```

### Plugin/Hook System

#### Hooks (Quality Gates)

**Available hook events (6 of 9 used):**

1. **PostToolUse** - After Edit/Write operations
   - Use case: Validate JUCE best practices after code edits
   - Example: Check for memory allocations in processBlock()

2. **UserPromptSubmit** - Before Claude processes user prompt
   - Use case: Auto-inject plugin context when plugin mentioned
   - Example: User says "continue DelayPlugin" → Hook automatically injects handoff.md and PLUGINS.md status
   - Critical for seamless workflow resumption

3. **Stop** - When main Claude finishes responding
   - Use case: Enforce stage commits actually happened
   - Example: Stage 3 completes → Hook verifies git commit with "Stage 3" exists
   - Critical for "zero drift" guarantee

4. **SubagentStop** - After subagent completion
   - Use case: Validate subagent output against contracts AND task
   - Example: foundation-agent subagent finishes → Hook checks CMakeLists.txt exists and builds successfully
   - Blocks workflow if validation fails (exit 2)

5. **PreCompact** - Before context compaction
   - Use case: Preserve contracts verbatim during compaction
   - Example: Context window fills after 60 minutes → Hook tells compaction to keep parameter-spec.md, architecture.md exact
   - Critical for long sessions (40-80 min workflows)

6. **SessionStart** - At session initialization
   - Use case: Setup validation, environment checks
   - Example: Verify JUCE installed, check git status

**Conditional hook execution pattern:**

Every hook must check if it's relevant before executing:

```bash
#!/bin/bash
# Example: verify-stage-commit.sh (Stop hook)

INPUT=$(cat)

# 1. Check if hook applies
if [ ! -f ".claude/handoff.md" ]; then
  exit 0  # No active workflow, skip gracefully
fi

# 2. Check if we're in implementation stages
CURRENT_STAGE=$(grep "current_stage:" .claude/handoff.md | cut -d: -f2 | xargs)
if [[ ! "$CURRENT_STAGE" =~ ^[2-6]$ ]]; then
  exit 0  # Not in implementation stages
fi

# 3. Now do actual validation
LAST_COMMIT=$(git log -1 --oneline --since="5 minutes ago")
if [[ "$LAST_COMMIT" =~ "Stage $CURRENT_STAGE" ]]; then
  exit 0  # Valid - allow continuation
else
  echo "Stage $CURRENT_STAGE incomplete: no git commit found" >&2
  exit 2  # Block Claude from continuing
fi
```

**Pattern:** Every hook starts with relevance checks → graceful skip if not applicable → validate → exit 0 (pass) or exit 2 (block with feedback)

**Other available hooks (not currently used):**
- **PreToolUse** - Before tool execution (system is post-execution validation focused)
- **Notification** - When notifications sent (no use case yet)
- **SessionEnd** - When session ends (cleanup/logging, not architecturally critical)

**Adding a new hook:**

**1. Create hook script:**
```bash
touch .claude/hooks/scripts/my-hook.py
chmod +x .claude/hooks/scripts/my-hook.py
```

**2. Implement hook logic:**
```python
#!/usr/bin/env python3
import json
import sys

# Read hook input from stdin
input_data = json.load(sys.stdin)
tool_input = input_data.get('tool_input', {})

# Validate/check something
if some_condition_fails:
    print("Error: Something wrong", file=sys.stderr)
    sys.exit(2)  # Exit code 2 blocks Claude

sys.exit(0)  # Exit code 0 allows continuation
```

**3. Register in hooks.json:**
```json
{
  "hooks": {
    "PostToolUse": [
      {
        "matcher": "Write|Edit",
        "hooks": [
          {
            "type": "command",
            "command": "${CLAUDE_PROJECT_DIR}/.claude/hooks/scripts/my-hook.py",
            "timeout": 30
          }
        ]
      }
    ]
  }
}
```

**4. Test:**
```
Make edit that violates hook condition
→ Hook should block with error message
→ Fix issue
→ Hook should allow
```

#### MCP Servers

**Integrating Context7 (already configured):**

```json
{
  "mcpServers": {
    "context7": {
      "command": "npx",
      "args": ["-y", "@context7/mcp-server"],
      "env": {}
    }
  }
}
```

**Adding new MCP server:**

```json
{
  "mcpServers": {
    "my-server": {
      "command": "command-to-run-server",
      "args": ["--flag", "value"],
      "env": {
        "API_KEY": "xxx"
      }
    }
  }
}
```

**Using in skills/agents:**
```yaml
allowed-tools:
  - mcp__my_server__tool_name
```

### Versioning Strategy

**System versioning:**
- **Major (2.0):** Architectural changes (subagents, hooks)
- **Minor (2.1):** New skills, agents, significant features
- **Patch (2.0.1):** Bug fixes, small improvements

**Plugin versioning:**
- **Major (2.0):** Breaking changes (parameter removal, format changes)
- **Minor (1.1):** New features (added parameters)
- **Patch (1.0.1):** Bug fixes (no new features)

**Backward compatibility:**
- v2 system can work with v1 plugins (read old structure)
- Migrate via: "Copy to v2 structure" command
- Old skills deprecated gracefully (warn, still work)

---
