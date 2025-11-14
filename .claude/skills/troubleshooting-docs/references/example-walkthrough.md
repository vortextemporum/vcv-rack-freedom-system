# Example Walkthrough: troubleshooting-docs Skill

This example demonstrates the complete 7-step workflow for capturing a solved problem.

## Scenario

A parameter state management issue was just fixed in ReverbPlugin. The user confirms: "That worked! The parameter is saving correctly now."

## Workflow Execution

### Step 1: Detect Confirmation

The phrase "That worked!" triggers automatic skill invocation (matches confirmation phrase in Step 1).

### Step 2: Gather Context

From conversation history:
- **Plugin**: ReverbPlugin
- **Symptom**: DECAY parameter not persisting after DAW reload
- **Stage**: Post-implementation (Stage 4 complete)
- **Failed attempts**: Checked APVTS creation, verified parameter ID
- **Solution**: Added `parameterValueChanged()` to update processor state
- **Root cause**: State only saved from APVTS, not reflecting UI changes

### Step 3: Check Existing Docs

Search troubleshooting/ for similar parameter issues:
```bash
grep -r "parameter not persisting" troubleshooting/
grep -r "DECAY" troubleshooting/parameter-issues/
```

Result: No similar issue found → proceed to Step 4

### Step 4: Generate Filename

Format: `[sanitized-symptom]-[plugin]-[YYYYMMDD].md`

Generated: `parameter-not-saving-decay-ReverbPlugin-20251110.md`

### Step 5: Validate YAML Schema

```yaml
---
plugin: ReverbPlugin
date: 2025-11-10
problem_type: state_management
component: apvts
symptoms:
  - "DECAY parameter not persisting after DAW reload"
  - "Parameter resets to default on plugin instance reload"
root_cause: state_sync
juce_version: 8.0.9
resolution_type: code_fix
severity: moderate
tags: [parameters, apvts, state-management, runtime]
---
```

Validation: ✅ All required fields present, enum values valid

### Step 6: Create Documentation

Category mapping: `state_management` → `parameter-issues/`

File created: `troubleshooting/parameter-issues/parameter-not-saving-decay-ReverbPlugin-20251110.md`

Uses template from `assets/resolution-template.md` with gathered context.

### Step 7: Cross-Reference & Pattern Detection

- **Cross-reference**: None (no similar issues found in Step 3)
- **Pattern detection**: Not critical severity, single plugin affected → no Required Reading suggestion

### Decision Menu

```
✓ Solution documented

File created:
- troubleshooting/parameter-issues/parameter-not-saving-decay-ReverbPlugin-20251110.md

What's next?
1. Continue workflow (recommended)
2. Add to Required Reading - Promote to critical patterns (juce8-critical-patterns.md)
3. Link related issues - Connect to similar problems
4. Update common patterns - Add to pattern library
5. View documentation - See what was captured
6. Other

Choose (1-6): _
```

User selects Option 1 → skill returns to calling workflow (plugin-improve in this case)

## Key Takeaways

1. **Auto-invocation**: Confirmation phrases trigger skill automatically
2. **Blocking requirements**: Plugin name, symptom, stage, and solution are mandatory
3. **YAML validation**: Schema validation prevents invalid categorization
4. **Progressive decision**: User decides whether to promote to Required Reading
5. **Terminal skill**: Returns control to calling skill after documentation

## See Also

- [SKILL.md](../SKILL.md) - Full skill documentation
- [references/yaml-schema.md](yaml-schema.md) - Complete schema reference
