# Phase 2 Plan Updates - Alignment with Phase 1 Restructuring

**Date:** 2025-11-10
**Phase:** 2 (Workflow Engine)
**Updated By:** Prompt 004 - Documentation Alignment

---

## Summary

Updated `plan/phase-2-plan.md` to align with Phase 1 restructuring changes. All references to the deprecated `/templates/` directory have been migrated to skill-specific `assets/` directories following Anthropic's modular skill structure.

---

## Changes Made

### 1. Prerequisites Section (Lines 7-18)

**BEFORE:**
```markdown
- ✅ `templates/` - 4 contract templates exist
```

**AFTER:**
```markdown
- ✅ `.claude/skills/` - 7 skills with modular structure (SKILL.md + references/ + assets/ subdirectories)
  - plugin-workflow, plugin-ideation, plugin-improve, context-resume, ui-mockup (two-phase workflow), plugin-testing, plugin-lifecycle
- ✅ Skill templates - Contract templates migrated to skill-specific assets/ directories:
  - `.claude/skills/ui-mockup/assets/parameter-spec-template.md`
  - `.claude/skills/plugin-workflow/assets/architecture-template.md`
  - `.claude/skills/plugin-workflow/assets/plan-template.md`
  - `.claude/skills/plugin-ideation/assets/creative-brief-template.md`
```

**WHY:** Phase 1 removed `/templates/` directory and migrated templates to skill assets. Prerequisites must reflect actual deliverables.

---

### 2. Handoff Template Path (Task 1, Lines 141, 174)

**BEFORE:**
```markdown
templates/.continue-here-template.md
```

**AFTER:**
```markdown
.claude/skills/plugin-workflow/assets/continue-here-template.md
```

**WHY:** Handoff template belongs with plugin-workflow skill that creates/manages it.

---

### 3. Contract Enforcement - ui-mockup Two-Phase Workflow (Task 2, 7)

**BEFORE:**
```markdown
- **Contract-based transitions:** Stage 1 transition requires parameter-spec.md and architecture.md
```

**AFTER:**
```markdown
- **Contract-based transitions:** Stage 1 transition requires parameter-spec.md and architecture.md (generated via ui-mockup two-phase workflow)
```

**ADDED CLARIFICATIONS:**
- Line 563: `action: "Complete ui-mockup two-phase workflow (design approval generates parameter-spec.md), then create architecture.md"`
- Line 573: `Suggest: "Complete ui-mockup two-phase workflow (Phase 4.5 design approval generates parameter-spec.md)"`
- Line 608: `Read contracts: parameter-spec.md (from ui-mockup Phase 4.5), architecture.md, research.md`
- Line 624: `parameter-spec.md: [N] parameters defined (from ui-mockup two-phase workflow)`
- Line 644: `**Contract enforcement BLOCKS** if parameter-spec.md (from ui-mockup) or architecture.md missing`

**WHY:** Phase 1 implemented ui-mockup as two-phase workflow where parameter-spec.md is generated at Phase 4.5 (design approval). Contract enforcement must reference correct workflow.

---

### 4. Meta Section File Paths (Lines 1913-1922, 1701)

**BEFORE:**
```markdown
- `templates/.continue-here-template.md` (handoff template)
- `research.md` (Stage 0 output)
- `plan.md` (Stage 1 output)

# Handoff template
ls -la templates/.continue-here-template.md
```

**AFTER:**
```markdown
- `.claude/skills/plugin-workflow/assets/continue-here-template.md` (handoff template)
- `plugins/[Name]/.ideas/research.md` (Stage 0 output)
- `plugins/[Name]/.ideas/plan.md` (Stage 1 output)

# Handoff template
ls -la .claude/skills/plugin-workflow/assets/continue-here-template.md
```

**WHY:** File paths must be accurate for verification scripts and developer reference.

---

### 5. Manual Verification Instructions (Lines 249, 654)

**BEFORE:**
```markdown
"Please verify the state machine prevents transitioning to Stage 1 without parameter-spec.md and architecture.md"
```

**AFTER:**
```markdown
"Please verify the state machine prevents transitioning to Stage 1 without parameter-spec.md and architecture.md (which come from ui-mockup two-phase workflow)"

"Please create a test plugin without parameter-spec.md and verify Stage 1 BLOCKS with error message listing missing contracts and suggesting ui-mockup workflow. Then add both contracts (parameter-spec.md from ui-mockup two-phase workflow, architecture.md manually)..."
```

**WHY:** Manual verification steps must reference correct workflow for generating contracts.

---

## Verification

### Automated Verification

```bash
# Verify no old /templates/ references remain
grep -n "templates/" plan/phase-2-plan.md
# Expected: No matches found ✓
```

**Result:** ✅ Zero references to deprecated `/templates/` directory

### Contract Location Verification

```bash
# Verify skill-specific template paths are documented
grep "assets/.*template" plan/phase-2-plan.md
```

**Result:** ✅ All template paths point to skill assets:
- `.claude/skills/ui-mockup/assets/parameter-spec-template.md`
- `.claude/skills/plugin-workflow/assets/architecture-template.md`
- `.claude/skills/plugin-workflow/assets/plan-template.md`
- `.claude/skills/plugin-ideation/assets/creative-brief-template.md`
- `.claude/skills/plugin-workflow/assets/continue-here-template.md`

### Skill Structure Acknowledgment

**Result:** ✅ Prerequisites section accurately describes modular skill structure with subdirectories

---

## Implications for Phase 2 Implementation

### 1. Template File Locations

When implementing Task 1 (Checkpoint System), create handoff template at:
```
.claude/skills/plugin-workflow/assets/continue-here-template.md
```

NOT at deprecated `templates/.continue-here-template.md`

### 2. Contract Enforcement Messages

Stage 1 precondition errors must guide users to correct workflow:
```
Next step: Complete ui-mockup two-phase workflow (design approval generates parameter-spec.md)
```

NOT: "Create parameter-spec.md manually"

### 3. ui-mockup Integration

Stage 0 and Stage 1 should acknowledge that parameter-spec.md comes from ui-mockup skill's two-phase workflow (Phase 4.5 design approval), not from templates or manual creation.

### 4. No Breaking Changes

Phase 2 implementation can proceed immediately - all updates are documentation corrections, not architectural changes.

---

## Success Criteria Met

✅ **Zero references to `/templates/` directory**
✅ **All template paths point to skill assets/**
✅ **Contract enforcement references correct workflow (ui-mockup two-phase)**
✅ **Prerequisites accurately reflect Phase 1 deliverables**
✅ **ui-mockup two-phase workflow documented**
✅ **Skill structure acknowledged as modular (SKILL.md + subdirectories)**
✅ **No outdated assumptions about file locations**
✅ **Plan is immediately executable with current codebase**

---

## Files Changed

- **Modified:** `plan/phase-2-plan.md`
  - Lines 7-18: Prerequisites section
  - Line 141: Handoff template path
  - Line 174: Expected output path
  - Line 222: Contract transition note
  - Line 249: Manual verification
  - Line 563: Contract enforcement action
  - Line 573: Suggestion text
  - Line 608: Planning logic contracts
  - Line 624: Contract references
  - Line 644: Expected output
  - Line 654: Manual verification
  - Lines 1913-1922: Meta section file paths
  - Line 1701: Verification script path

- **Created:** `phase-2-plan-updates.md` (this file)

---

## Next Steps

Phase 2 implementation can proceed with updated plan. Key reminders:

1. Create handoff template in plugin-workflow skill assets, not /templates/
2. Contract enforcement should reference ui-mockup two-phase workflow
3. All contract templates live in skill-specific assets/ directories
4. parameter-spec.md is generated by ui-mockup at Phase 4.5 (design approval)

---

**Status:** ✅ Complete
**Executable:** Yes - Phase 2 can begin implementation immediately
