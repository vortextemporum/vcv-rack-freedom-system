<objective>
Refactor the existing plugin-workflow skill to handle ONLY stages 2-6 (implementation stages), removing stages 0-1 which are now handled by the plugin-planning skill created in prompt 003.

This skill becomes a pure orchestrator that ALWAYS invokes subagents for stages 2-5, then handles validation in stage 6. The goal is to enforce consistent checkpoint behavior where subagents do work, return to orchestrator, and orchestrator presents decision menus.
</objective>

<context>
This follows prompt 003 which created the plugin-planning skill for stages 0-1.

Current plugin-workflow skill at @.claude/skills/plugin-workflow/ needs refactoring to:
- Remove stages 0-1 (now in plugin-planning)
- Become pure orchestrator for stages 2-6
- ALWAYS use subagents for stages 2-5 (never implement directly)
- Present numbered decision menus after EVERY subagent completion

The skill has existing subagents:
- foundation-agent (Stage 2)
- shell-agent (Stage 3)
- dsp-agent (Stage 4)
- gui-agent (Stage 5)
- validator (Stage 6)

@.claude/skills/plugin-workflow/SKILL.md - Current skill to refactor
@.claude/skills/plugin-workflow/references/state-management.md - Keep and enhance
</context>

<requirements>
1. Update SKILL.md to remove Stage 0-1 logic
2. Add explicit enforcement: Stages 2-5 MUST invoke subagents via Task tool
3. After each subagent completes, orchestrator MUST:
   - Commit changes
   - Update .continue-here.md
   - Update PLUGINS.md
   - Present numbered decision menu
   - Wait for user response
4. Stage 6 (validation) can run directly or via validator subagent
5. Delete references/stage-0-research.md and references/stage-1-planning.md
6. Update preconditions to require architecture.md and plan.md exist
</requirements>

<implementation>
Key changes to SKILL.md:

1. Preconditions section should check:
   ```
   - architecture.md must exist (from /plan)
   - plan.md must exist (from /plan)
   - Status must be 🚧 Stage 1 or resuming from 🚧 Stage 2+
   ```

2. Stage dispatcher must enforce:
   ```javascript
   switch(stage) {
     case 2:
       result = await invokeSubagent('foundation-agent', context)
       break
     case 3:
       result = await invokeSubagent('shell-agent', context)
       break
     case 4:
       result = await invokeSubagent('dsp-agent', context)
       break
     case 5:
       result = await invokeSubagent('gui-agent', context)
       break
     case 6:
       result = executeValidation(context) // Can be direct or subagent
       break
   }
   // ALWAYS present decision menu after stage completes
   presentDecisionMenu(...)
   ```

3. Remove any direct implementation logic for stages 2-5

Decision menu format remains:
```
✓ Stage [N] complete: [description]

What's next?
1. Continue to Stage [N+1] (recommended)
2. [Alternative option]
3. [Discovery option] ← User discovers [feature]
4. Pause here
5. Other

Choose (1-5): _
```
</implementation>

<output>
Update these files:
- `.claude/skills/plugin-workflow/SKILL.md` - Refactored for stages 2-6 only
- Delete `.claude/skills/plugin-workflow/references/stage-0-research.md`
- Delete `.claude/skills/plugin-workflow/references/stage-1-planning.md`
- Update `.claude/skills/plugin-workflow/references/state-management.md` - Enhanced checkpoint enforcement
</output>

<verification>
After refactoring:
1. Verify SKILL.md has NO Stage 0-1 logic
2. Confirm stages 2-5 ALWAYS use Task tool to invoke subagents
3. Check decision menus appear after EVERY stage completion
4. Verify preconditions block if no architecture.md/plan.md
5. Test that resume from .continue-here.md works correctly
</verification>

<success_criteria>
- plugin-workflow handles ONLY stages 2-6
- Stages 2-5 are pure orchestration (subagent invocation)
- Numbered decision menus after every stage
- Clear handoff from plugin-planning skill
- No direct implementation in orchestrator for stages 2-5
</success_criteria>