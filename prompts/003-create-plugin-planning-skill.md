<objective>
Create a new plugin-planning skill that handles stages 0-1 (Research and Planning) of the plugin development workflow. This skill will be invoked by the new /plan command and focuses on interactive contract creation without using subagents.

This is part of a larger refactor to separate planning (thinking/contracts) from implementation (building/code). The end goal is clearer separation of concerns and more consistent checkpoint behavior.
</objective>

<context>
This is a JUCE plugin development system that uses a 7-stage workflow. We're splitting the massive plugin-workflow skill (2200+ lines) into two focused skills:
- plugin-planning (stages 0-1) - interactive, no subagents
- plugin-workflow (stages 2-6) - pure orchestration with subagents

Current structure at @.claude/skills/plugin-workflow/ contains all stages 0-6. We need to extract stages 0-1 into a new skill.

Key files to reference:
@.claude/skills/plugin-workflow/references/stage-0-research.md - Research stage implementation
@.claude/skills/plugin-workflow/references/stage-1-planning.md - Planning stage implementation
@.claude/skills/plugin-workflow/SKILL.md - Current combined skill (for reference)
</context>

<requirements>
1. Create new skill directory: .claude/skills/plugin-planning/
2. Create SKILL.md that orchestrates stages 0-1 only
3. Copy and adapt stage-0-research.md and stage-1-planning.md to references/
4. Ensure proper handoff file creation for resume capability
5. Use numbered decision menus at EVERY checkpoint (not AskUserQuestion)
6. Stage 0 creates architecture.md, Stage 1 creates plan.md
7. Update PLUGINS.md status tracking appropriately
8. Git commits after each stage completion
</requirements>

<implementation>
The skill should:
- Check prerequisites (creative-brief.md must exist)
- Execute Stage 0 (Research) - create architecture.md
- Present numbered decision menu
- Execute Stage 1 (Planning) - create plan.md
- Present final numbered decision menu
- Create handoff for /implement to pick up

Decision menu format MUST be:
```
✓ [Completion statement]

What's next?
1. [Primary action] (recommended)
2. [Secondary action]
3. [Discovery option] ← User discovers [feature]
4. [Alternative path]
5. Other

Choose (1-5): _
```

No subagents should be invoked - this is direct implementation only.
</implementation>

<output>
Create these files:
- `.claude/skills/plugin-planning/SKILL.md` - Main skill orchestrator
- `.claude/skills/plugin-planning/references/stage-0-research.md` - Research implementation
- `.claude/skills/plugin-planning/references/stage-1-planning.md` - Planning implementation
- `.claude/skills/plugin-planning/assets/plan-template.md` - Template for plan.md
- `.claude/skills/plugin-planning/assets/architecture-template.md` - Template for architecture.md
</output>

<verification>
After creating the skill:
1. Verify SKILL.md includes proper YAML frontmatter with allowed-tools
2. Confirm decision menus use numbered format (not AskUserQuestion)
3. Check that handoff file format matches what /implement expects
4. Ensure git commit happens after each stage
5. Verify status updates in PLUGINS.md work correctly
</verification>

<success_criteria>
- New plugin-planning skill exists with stages 0-1 only
- Clear separation from implementation stages (2-6)
- Numbered decision menus at every checkpoint
- Proper handoff to plugin-workflow skill
- Git commits after each stage
</success_criteria>