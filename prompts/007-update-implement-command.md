<objective>
Update the existing /implement command to work with the new separated workflow where it handles ONLY stages 2-6 (after /plan has created contracts). This completes the workflow refactor started in prompts 003-006.

The command must now require that planning artifacts (architecture.md and plan.md) exist before allowing implementation to proceed.
</objective>

<context>
Building on prompts 003-006:
- Prompt 003 created plugin-planning skill (stages 0-1)
- Prompt 004 refactored plugin-workflow skill (stages 2-6)
- Prompt 005 added system-wide protocols
- Prompt 006 created /plan command

Now update /implement to:
- Require planning artifacts exist
- Start at stage 2 (not stage 0)
- Work with the refactored plugin-workflow skill

Complete workflow:
1. /dream → creative brief + mockup
2. /plan → architecture.md + plan.md
3. /implement → Source/ code

@.claude/commands/implement.md - Existing command to update
@.claude/skills/plugin-workflow/SKILL.md - Refactored skill (stages 2-6)
</context>

<requirements>
1. Update /implement command to check for planning prerequisites
2. Block execution if architecture.md or plan.md don't exist
3. Start workflow at stage 2 (foundation) not stage 0
4. Ensure proper status checking in PLUGINS.md
5. Guide users to /plan if prerequisites missing
</requirements>

<implementation>
Update the command to:

```markdown
# /implement Command

Build a plugin through implementation stages 2-6 (foundation through validation).

## Usage
/implement [PluginName]

## Prerequisites Check
1. Verify plugin exists in PLUGINS.md
2. REQUIRE architecture.md exists:
   - If missing: "Run /plan [PluginName] first to create architecture and plan"
3. REQUIRE plan.md exists:
   - If missing: "Run /plan [PluginName] first to create architecture and plan"
4. Check current status:
   - If 💡 Ideated → Block, run /plan first
   - If 🚧 Stage 0 → Block, complete /plan first
   - If 🚧 Stage 1 → OK, start at stage 2
   - If 🚧 Stage 2+ → Resume from current stage
   - If ✅ Working → Block, use /improve instead

## Invoke Skill
Use Task tool with subagent_type: "plugin-workflow"
Pass starting_stage: 2 (or resume stage)

## Implementation Flow
- Stage 2: Foundation (foundation-agent)
- Stage 3: Shell (shell-agent)
- Stage 4: DSP (dsp-agent)
- Stage 5: GUI (gui-agent)
- Stage 6: Validation (validator)

Each stage uses subagents, then presents decision menu.
```

Include clear error messages guiding users to the right command based on state.
</implementation>

<output>
Update:
- `.claude/commands/implement.md` - Modified to require planning artifacts
</output>

<verification>
After updating:
1. Verify command blocks if no architecture.md/plan.md
2. Check error messages guide to /plan command
3. Confirm starts at stage 2, not stage 0
4. Test resume functionality for stages 2-6
5. Verify clean handoff from /plan command
</verification>

<success_criteria>
- /implement requires planning artifacts
- Clear error messages guide workflow
- Starts at correct stage (2)
- Works with refactored plugin-workflow skill
- Complete workflow: /dream → /plan → /implement
</success_criteria>