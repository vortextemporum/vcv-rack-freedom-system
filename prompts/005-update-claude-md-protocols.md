<objective>
Update the project-level CLAUDE.md files to establish system-wide protocols for checkpoint behavior and subagent orchestration. This ensures consistent behavior across all skills and prevents regression to the old patterns.

These updates work in conjunction with the skill-specific changes from prompts 003-004, providing defense-in-depth by making expectations clear at the system level.
</objective>

<context>
Following prompts 003-004 which separated plugin-planning from plugin-workflow, we need system-wide rules in CLAUDE.md to ensure:
1. Numbered decision menus are used consistently everywhere
2. Subagent orchestration pattern is followed for stages 2-5
3. The checkpoint protocol is non-negotiable

Files to update:
@.claude/CLAUDE.md - Project-specific instructions
@CLAUDE.md (in project root if exists) - Additional project instructions
</context>

<requirements>
Add two new sections to .claude/CLAUDE.md:

1. **Checkpoint Protocol (System-Wide)** - Mandate numbered decision menus
2. **Subagent Invocation Protocol** - Enforce orchestration pattern for stages 2-5

These should be clear, non-negotiable rules that apply across the entire system.
</requirements>

<implementation>
Add these exact sections to .claude/CLAUDE.md:

```markdown
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
```

Place these sections prominently in CLAUDE.md, ideally after the existing priority_rules or development_environment sections.
</implementation>

<output>
Update:
- `.claude/CLAUDE.md` - Add both protocol sections
</output>

<verification>
After updating:
1. Verify both sections are clearly visible in CLAUDE.md
2. Check formatting is correct (markdown headers, code blocks)
3. Confirm the rules are unambiguous and enforceable
4. Test that Claude sees these rules in new contexts
</verification>

<success_criteria>
- System-wide checkpoint protocol is documented
- Subagent invocation pattern is mandatory for stages 2-5
- Rules are clear and non-negotiable
- Defense-in-depth with skill-level enforcement
</success_criteria>