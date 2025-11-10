# troubleshooting-docs

**Purpose:** Automatically document solved problems to build searchable institutional knowledge.

**Version:** 1.0.0

---

## What It Does

Creates structured troubleshooting documentation after you confirm a bug fix works. Captures the problem, failed attempts, actual solution, technical explanation, and prevention strategies.

## When To Use

**Automatic invocation after:**
- You confirm a fix works: "that worked", "it's fixed", "working now"
- Non-trivial problems are solved (not typos or obvious errors)
- Multiple attempts were needed to find the solution
- Tricky debugging that took investigation

**Do NOT use for:**
- Simple typos or syntax errors
- Obvious mistakes immediately corrected
- Trivial issues that don't warrant documentation
- General questions with straightforward answers

**Key principle:** If future sessions would benefit from knowing this solution, document it.

## How It Works

### Step 1: Detect Confirmation
Listens for phrases like:
- "that worked"
- "it's fixed"
- "that solved it"
- "working now"

### Step 2: Gather Context
Extracts from conversation history:
- **Problem**: Exact error message or symptom
- **Context**: JUCE version, component, file/line
- **Failed attempts**: What didn't work and why
- **Solution**: What actually fixed it
- **Root cause**: Why the fix works
- **Prevention**: How to avoid this

### Step 3: Check Existing Docs
Searches `docs/troubleshooting/*.md` for similar issues:
- Exact error messages
- Similar symptoms
- Related components

If found, considers:
- Adding as subsection
- Creating separate doc with cross-reference
- Linking related issues

### Step 4: Generate Filename
Creates searchable filename:
```
[component]-[problem-type]-[key-symptom].md
```

**Examples:**
- `flexbox-layout-overlapping-components.md`
- `cmake-linking-undefined-symbols.md`
- `apvts-parameter-not-saving-state.md`

### Step 5: Validate YAML Schema
**CRITICAL:** All docs require validated YAML front matter.

**Loads schema:**
```bash
cat .claude/skills/troubleshooting-docs/schema.yaml
```

**Gets JUCE version:**
```bash
cat logs/system/juce_version.txt
```

**Classifies problem:**
- `problem_type`: build_error, runtime_error, ui_layout, etc.
- `component`: cmake, juce_gui_basics, juce_dsp, etc.
- `symptoms`: Observable behavior (array, 1-5 items)
- `root_cause`: missing_module, wrong_api, thread_violation, etc.
- `resolution_type`: code_fix, config_change, api_migration, etc.
- `severity`: critical, moderate, minor
- `tags`: Searchable keywords (lowercase, hyphenated)

### Step 6: Create Documentation
Writes file to `docs/troubleshooting/[filename].md`:

```markdown
---
problem_type: ui_layout
component: juce_gui_basics
symptoms:
  - All rotary sliders render in same position
  - FlexBox performLayout() called but has no effect
root_cause: missing_constraint
juce_version: 8.0.9
resolution_type: code_fix
severity: moderate
tags: [flexbox, layout, slider, sizing]
---

# Troubleshooting: Rotary Sliders Overlapping in FlexBox

## Problem
[1-2 sentence clear description]

## Environment
- JUCE Version: 8.0.9
- Affected: PluginEditor.cpp resized(), Stage 5
- Date: 2025-11-07

## Symptoms
- Observable symptom 1
- Observable symptom 2

## What Didn't Work

**Attempted Solution 1:** [Description]
- **Why it failed:** [Technical reason]

**Attempted Solution 2:** [Description]
- **Why it failed:** [Technical reason]

## Solution

[The actual fix with code examples]

## Why This Works

[Technical explanation of root cause and why fix addresses it]

## Prevention

- How to avoid this in future
- What to watch out for
- How to catch early

## Related Issues
- See also: [other-doc.md](other-doc.md)
```

### Step 7: Cross-Reference
Updates related docs with links to new documentation.

### Step 8: Stage with Fix
Documentation is committed WITH the fix, not separately:

```bash
git add docs/troubleshooting/[filename].md
git add [fixed files]
```

Commit message:
```
fix: [brief description]

[explanation]

Documented in docs/troubleshooting/[filename].md
```

---

## What It Creates

**Location:** `docs/troubleshooting/[descriptive-name].md`

**Example filenames:**
- `flexbox-layout-rotary-sliders-overlapping.md`
- `cmake-undefined-symbols-juce-dsp-compressor.md`
- `apvts-state-not-persisting-after-reload.md`

**Each doc includes:**
- Validated YAML front matter (required)
- Clear problem description
- Environment details
- Observable symptoms
- Failed attempts (what didn't work)
- Actual solution (what did work)
- Technical explanation (why it works)
- Prevention guidance
- Related issue links

---

## Integration

**Feeds into:**
- `deep-research` skill - Reads these docs as Level 1 (Fast Path)

**Called by:**
- Any skill after you confirm fix works
- Automatic detection of success phrases
- `/doc-fix` slash command

**Outputs:**
- `docs/troubleshooting/[descriptive-name].md`

---

## Quality Guidelines

**Good docs have:**
✅ Clear problem description (recognizable symptoms)
✅ Specific environment details (JUCE version, component)
✅ Observable symptoms (what you actually saw)
✅ Failed attempts documented (helps avoid wrong paths)
✅ Technical explanation (not just "what" but "why")
✅ Prevention guidance (how to avoid)

**Avoid:**
❌ Vague descriptions ("Something was wrong")
❌ Missing technical details ("Fixed the code")
❌ No context (which version? which component?)
❌ Just code dumps (explain WHY it works)
❌ No prevention guidance

---

## Example Scenarios

### Scenario 1: UI Layout Bug
**Problem:** Rotary sliders overlapping in FlexBox

**Failed attempts:**
1. Used setBounds() - Failed because FlexBox manages bounds
2. Added margins - Failed because no size constraints

**Solution:** Added .withFlex(1) to FlexItems

**Documentation created:**
- File: `flexbox-layout-rotary-sliders-overlapping.md`
- Problem type: ui_layout
- Root cause: missing_constraint
- Prevention: Always provide size constraints with FlexBox

### Scenario 2: Build Error
**Problem:** Linker error for juce::dsp::Compressor

**Failed attempts:**
1. Added include - Failed because linking issue, not include
2. Clean rebuild - Failed because module not linked

**Solution:** Added `juce::juce_dsp` to target_link_libraries()

**Documentation created:**
- File: `cmake-undefined-symbols-juce-dsp-compressor.md`
- Problem type: build_error
- Root cause: missing_module
- Prevention: Always link JUCE modules in CMakeLists.txt

---

## Why This Matters

**Without troubleshooting docs:**
- Same problems solved repeatedly
- No learning across sessions
- Slow research every time

**With troubleshooting docs:**
- Fast lookup for known issues
- Learn from past solutions
- Build institutional knowledge
- deep-research finds answers immediately

**The feedback loop:**
1. Hit a problem → deep-research searches local docs first
2. Fix the problem → troubleshooting-docs creates documentation
3. Hit similar problem → deep-research finds it instantly
4. Knowledge compounds over time

---

## Best Practices

**Document immediately:**
- Create docs while context is fresh
- Don't wait until end of session

**Be specific:**
- Exact error messages
- Specific file:line references
- Observable symptoms

**Explain why:**
- Not just what changed
- Why the fix works
- Root cause analysis

**Think like grep:**
- Use searchable filenames
- Include error message keywords
- Tag with relevant terms

**Cross-reference:**
- Link related issues
- Build knowledge web

---

## Verification Checklist

After creating documentation:

**Schema validation:**
- ✅ YAML front matter exists
- ✅ All required fields present
- ✅ Enum values are valid
- ✅ JUCE version matches logs/system/juce_version.txt
- ✅ Symptoms are specific and observable
- ✅ Tags are lowercase, hyphenated

**Content quality:**
- ✅ Problem is clearly described
- ✅ Environment details included
- ✅ Failed attempts documented
- ✅ Solution is specific with code examples
- ✅ Technical explanation explains why
- ✅ Prevention guidance provided
- ✅ Related docs cross-referenced
- ✅ Staged with fix commit

---

## Common Questions

**"Do I document every fix?"**
No. Only non-trivial problems that future sessions would benefit from.

**"What if I'm not sure about root cause?"**
Document what you know. Mark uncertainty. Better to capture partial knowledge than none.

**"Should I update existing docs?"**
Yes, if new solution is better or adds important context.

**"What if the fix was obvious?"**
Skip documentation. Only document things worth remembering.

**"How long should docs be?"**
As long as needed to be clear and useful. Quality over brevity.

---

## Output

After successful documentation:

```
✓ Troubleshooting documentation created

File: docs/troubleshooting/flexbox-layout-overlapping.md
Problem: Rotary sliders overlapping in FlexBox layout
Solution: Added size constraints with .withFlex()

This documentation will be searched by deep-research skill
as Level 1 (Fast Path) when similar issues occur.

Staged with fix commit. Ready to commit.
```
