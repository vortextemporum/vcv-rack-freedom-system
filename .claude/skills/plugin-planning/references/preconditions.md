# Precondition Validation

All preconditions that must be verified before plugin-planning skill executes.

## Contents
- Creative brief existence check
- Plugin status validation
- Resume logic for existing contracts

## Validation Protocol

<precondition_gate>
<validation_requirement>
MUST verify all preconditions before proceeding. If ANY check fails, BLOCK and report to user.
</validation_requirement>

### Check 1: Creative Brief Exists

```bash
if [ ! -f "plugins/${PLUGIN_NAME}/.ideas/creative-brief.md" ]; then
    echo "✗ creative-brief.md not found - SKILL BLOCKED"
    cat assets/precondition-failed.md
    exit 1
fi
```

**Verification:**
- File exists at expected path
- File is readable
- File is not empty

**If check fails:**
- Display error from assets/precondition-failed.md
- Exit skill immediately
- User must create creative brief first

### Check 2: Plugin Status Validation

```bash
STATUS=$(grep -A 2 "^### ${PLUGIN_NAME}$" PLUGINS.md | grep "Status:" | awk '{print $2}')
```

<blocking_condition>
IF status is 🚧 Stage N where N >= 2:
  THEN BLOCK with message "Plugin already past planning stage"
ELSE IF status is 💡 Ideated or not found:
  THEN Proceed
</blocking_condition>

**Valid statuses for proceeding:**
- 💡 Ideated
- 🚧 Stage 0
- 🚧 Stage 1
- Not found (new plugin)

**Blocking statuses:**
- 🚧 Stage 1 or higher
- ✅ Complete

### Check 3: Existing Contract Detection

```bash
# Check what already exists
test -f "plugins/${PLUGIN_NAME}/.ideas/architecture.md" && echo "✓ architecture.md exists"
test -f "plugins/${PLUGIN_NAME}/.ideas/plan.md" && echo "✓ plan.md exists"
```

**Resume logic:**

<resume_logic>
IF architecture.md exists AND plan.md missing:
  THEN skip to Stage 1
ELSE IF architecture.md exists AND plan.md exists:
  THEN ask user: "Both contracts exist. Regenerate both contracts or proceed to implementation?"
ELSE:
  THEN start at Stage 0
</resume_logic>

**User response handling:**
- "Regenerate" → Delete both contracts, restart Stage 0
- "Proceed" → Skip to handoff (go to implementation)
- "Cancel" → Exit skill

</precondition_gate>

## Integration with SKILL.md

SKILL.md should reference this file for detailed validation logic:

```markdown
**Check preconditions first:** See [references/preconditions.md](references/preconditions.md) for detailed validation logic.

Quick check:
1. creative-brief.md must exist
2. Plugin must NOT be past Stage 1
3. Detect existing contracts for resume logic
```
