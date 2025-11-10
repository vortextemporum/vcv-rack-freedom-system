# /doc-fix

**Purpose:** Document a recently solved problem for the troubleshooting knowledge base.

---

## Syntax

```bash
/doc-fix                    # Document the most recent fix
/doc-fix [brief context]    # Provide additional context hint
```

## What It Does

Immediately invokes the `troubleshooting-docs` skill to create structured documentation of the problem you just solved. Captures it while context is fresh.

**Use when:**
- You just confirmed a fix works
- The problem was non-trivial (not a simple typo)
- Future sessions would benefit from knowing the solution
- You want to preserve the knowledge

## Routes To

**Skill:** `troubleshooting-docs`

## Context Needed

The skill extracts from recent conversation:

**Problem:** Error messages, symptoms, unexpected behavior
**Context:** Plugin name, JUCE version, component, stage
**Failed Attempts:** What didn't work and why
**Solution:** What actually fixed it (code/config changes)
**Root Cause:** Technical explanation
**Prevention:** How to avoid in future

## What It Creates

**Location:** `docs/troubleshooting/[descriptive-name].md`

**Format:**
- Validated YAML front matter (problem_type, component, symptoms, root_cause, severity, tags)
- Problem description
- Environment details
- What didn't work
- Actual solution
- Technical explanation
- Prevention guidance
- Related issue links

## Example

```
User: That fixed it! The sliders are positioned correctly now.
      /doc-fix
Assistant: [Invokes troubleshooting-docs skill]
          [Gathers context from conversation]
          [Creates documentation]

          ✓ Documentation created
          File: docs/troubleshooting/flexbox-layout-overlapping.md

          Staged with fix commit.
```

## When Skill Activates

**Automatically after:**
- "that worked", "it's fixed", "working now"
- Non-trivial bug solved
- Multiple attempts needed

**Manually with:**
- `/doc-fix` command
- Any time you want to document a solution

## Related Commands

- `/dream` - Research problems before solving
- Natural language: "investigate this error" triggers deep-research skill

## Why This Matters

**Builds knowledge base:**
- Future sessions find solutions faster
- deep-research searches docs first (Level 1 Fast Path)
- No solving same problem repeatedly
- Institutional knowledge compounds

**The feedback loop:**
1. Hit problem → deep-research searches local docs
2. Fix problem → troubleshooting-docs creates doc
3. Hit similar problem → Found instantly
4. Knowledge grows over time

## Tips

**Document immediately:** Context is fresh, details are clear

**Be specific:** Exact errors, file:line references, observable symptoms

**Explain why:** Not just what changed, but why it works

**Trust the process:** Skill handles YAML validation, cross-references, commit staging

## Output

Documentation committed with the fix, searchable by future sessions, feeds into deep-research skill.

