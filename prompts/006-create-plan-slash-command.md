<objective>
Create the /plan slash command that invokes the plugin-planning skill (created in prompt 003) to handle stages 0-1 of plugin development. This command handles research and planning phases, creating architecture.md and plan.md contracts.

This completes the workflow separation where /plan handles thinking/contracts and /implement handles building/code.
</objective>

<context>
Building on prompts 003-005:
- Prompt 003 created the plugin-planning skill for stages 0-1
- Prompt 004 refactored plugin-workflow for stages 2-6 only
- Prompt 005 added system-wide protocols

Now we need the /plan command to invoke plugin-planning.

Current workflow will become:
1. /dream → creative brief + mockup
2. /plan → architecture.md + plan.md (stages 0-1)
3. /implement → Source/ code (stages 2-6)

@.claude/commands/ - Directory for slash commands
@.claude/skills/plugin-planning/SKILL.md - Skill to invoke
</context>

<requirements>
1. Create /plan command file in .claude/commands/
2. Command should accept plugin name as argument
3. Check prerequisites:
   - Plugin must exist in PLUGINS.md
   - creative-brief.md should exist
   - parameter-spec.md should exist (from mockup)
4. Invoke plugin-planning skill
5. Handle both new planning and resume scenarios
</requirements>

<implementation>
The command should:

```markdown
# /plan Command

Invoke the plugin-planning skill to research and plan a plugin (stages 0-1).

## Usage
/plan [PluginName]

## Prerequisites Check
1. Verify plugin exists in PLUGINS.md
2. Check for creative-brief.md (warn if missing but don't block)
3. Check for parameter-spec.md (warn if missing but don't block)
4. Check current status:
   - If 💡 Ideated → Start fresh planning
   - If 🚧 Stage 0 → Resume research
   - If 🚧 Stage 1 → Resume planning
   - If 🚧 Stage 2+ → Block (use /continue or /implement)

## Invoke Skill
Use Task tool with subagent_type: "plugin-planning"

## Output
- architecture.md (Stage 0)
- plan.md (Stage 1)
- Updated PLUGINS.md status
- .continue-here.md for handoff
```

After planning completes, the user can run /implement to build the plugin.
</implementation>

<output>
Create:
- `.claude/commands/plan.md` - The slash command definition
</output>

<verification>
After creating:
1. Verify command accepts plugin name argument
2. Check prerequisite validation works
3. Confirm skill invocation uses Task tool correctly
4. Test status checking logic
5. Verify handoff to /implement works
</verification>

<success_criteria>
- /plan command exists and invokes plugin-planning skill
- Prerequisites are properly checked
- Status transitions are validated
- Clean handoff to /implement command
- Part of complete workflow: /dream → /plan → /implement
</success_criteria>