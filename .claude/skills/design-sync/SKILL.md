---
name: design-sync
description: Validate mockup ↔ creative brief consistency, catch design drift
allowed-tools:
  - Read # Read contracts (brief, parameter-spec, mockup YAML)
  - Edit # Update brief's Evolution section
preconditions:
  - creative-brief.md exists
  - Mockup has been finalized (parameter-spec.md generated)
extended-thinking: true
thinking-budget: 8000 # Moderate creative reasoning
---

# design-sync Skill

**Purpose:** Validate mockup ↔ creative brief consistency and catch design drift before implementation begins.

## Overview

This skill compares the finalized UI mockup against the original creative brief to detect drift—where the implemented design diverges from the original vision. Catches misalignments before Stage 5 (GUI implementation) starts, preventing wasted work.

**Why this matters:**

Creative briefs capture intent. Mockups capture reality. During design iteration, these can diverge:

- Brief says 8 parameters → Mockup has 12
- Brief says "vintage warmth" → Mockup is "modern minimal"
- Brief mentions preset browser → Mockup lacks it

Detecting drift early allows course correction before implementation locks in the design.

**Key innovation:** Dual validation (quantitative + semantic) with categorized drift levels and appropriate decision menus.

---

## Entry Points

**Invoked by:**

1. **Auto-invoked by ui-mockup** after Phase 4.5 finalization (before C++ generation)
2. **Manual:** `/sync-design [PluginName]`
3. **Stage 1 Planning:** Optional pre-check before implementation starts

**Entry parameters:**

- **Plugin name**: Which plugin to validate
- **Mockup version** (optional): Specific version to check (defaults to latest)

---

## Workflow

### Step 1: Load Contracts

Read three files:

- `plugins/[PluginName]/.ideas/creative-brief.md` - Original vision
- `plugins/[PluginName]/.ideas/parameter-spec.md` - Finalized parameters from mockup
- `plugins/[PluginName]/.ideas/mockups/vN-ui.yaml` - Finalized mockup spec

**Error handling:** If any missing, BLOCK with clear message:

```
❌ Cannot validate design sync

Missing required files:
- creative-brief.md: [EXISTS/MISSING]
- parameter-spec.md: [EXISTS/MISSING]
- mockups/v[N]-ui.yaml: [EXISTS/MISSING]

design-sync requires finalized mockup (parameter-spec.md generated).

Actions:
1. Generate parameter-spec.md - Finalize mockup first
2. Create creative brief - Document vision before mockup
3. Skip validation - Proceed without sync (not recommended)
4. Other
```

### Step 2: Quantitative Checks

**Extract data from contracts:**

```typescript
// From creative-brief.md
briefParamCount = extractParameterCount(brief); // Count mentioned parameters
briefFeatures = extractFeatures(brief); // Grep for keywords

// From parameter-spec.md
mockupParamCount = countParameters(parameterSpec);

// From mockup YAML
mockupControls = countControls(mockupYAML); // All UI elements

// Compare
paramCountDelta = mockupParamCount - briefParamCount;
missingFeatures = briefFeatures.filter((f) => !presentInMockup(f));
```

**Parameter count thresholds:**

- Match (0-1 difference): No drift
- Small increase (2-4): Acceptable evolution
- Large increase (5+): Drift requiring attention
- Massive (2x or more): Critical drift
- Any decrease: Drift requiring attention (scope reduction)

**Feature detection:**
Keywords to search in brief:

- "preset", "presets"
- "bypass"
- "meter", "visualization"
- "undo/redo"
- "A/B compare"

Check if mockup YAML contains corresponding components.

### Step 3: Semantic Validation (Extended Thinking)

**Use extended thinking to answer:**

1. **Visual style alignment:**

   - Brief aesthetic: [extract quotes from "UI Vision" section]
   - Mockup aesthetic: [analyze YAML design system: colors, typography, layout style]
   - Match? Yes/No + reasoning

2. **Feature completeness:**

   - Brief promises: [list all mentioned features]
   - Mockup delivers: [list all implemented features]
   - Missing: [list gaps]
   - Assessment: Complete / Partial / Missing core features

3. **Scope assessment:**
   - Additions justified? (evolution vs creep)
   - Reductions justified? (simplification vs missing)
   - Core concept preserved?

**Extended thinking prompt:**

```
Compare creative brief with mockup to assess alignment:

Creative Brief:
- Concept: [extract]
- Use Cases: [extract]
- UI Vision: [extract]
- Parameters: [list]

Mockup:
- Layout: [from YAML]
- Components: [from YAML]
- Visual style: [colors, typography from YAML]
- Parameters: [from parameter-spec.md]

Answer:
1. Does mockup visual style match brief aesthetic intent? (vintage warmth vs modern, minimal vs complex, etc.)
2. Are all brief-mentioned features present?
3. Are mockup additions reasonable evolution or scope creep?
4. Does mockup support the use cases in brief?

Assess confidence: HIGH / MEDIUM / LOW
```

### Step 4: Categorize Drift

Based on findings:

**No drift detected:**

- Parameter counts match (±1)
- All features present
- Style aligned (semantic validation: YES)
- Mockup delivers on brief promise

**Acceptable evolution:**

- Parameter count increased slightly (2-4)
- Visual polish added
- Layout refined for UX
- Additions improve design (justified)
- Core concept intact

**Attention needed:**

- Missing features from brief
- Style mismatch (different direction)
- Significant scope change (±5 parameters)
- Brief and mockup tell different stories

**Critical drift:**

- Mockup contradicts brief's core concept
- Missing essential features
- Massive scope change (2x parameters or more)
- Completely opposite visual style

### Step 5: Present Findings

#### If No Drift:

```
✓ Design-brief alignment verified

- Parameter count: 12 (matches brief)
- All features present: preset browser, bypass, meter
- Visual style aligned: Modern minimal with analog warmth

What's next?
1. Continue implementation (recommended) - Alignment confirmed
2. Review details - See full comparison
3. Other
```

#### If Acceptable Evolution:

```
⚠️ Design evolution detected (acceptable)

**Changes from brief:**
- Parameter count: 12 (brief: 8) +4 parameters
  - Added: TONE, DRIVE, MIX, WIDTH
- Added features: Visual feedback meter (not in brief)
- Visual refinements: Animation polish, gradient backgrounds

**Assessment:** Reasonable evolution based on design iteration

**Reasoning:**
Added parameters (TONE, DRIVE, MIX, WIDTH) provide necessary tonal shaping not
anticipated in original brief. Visual meter improves usability during performance.
Core concept ("vintage tape delay") preserved.

What's next?
1. Update brief and continue (recommended) - Document evolution
2. Review changes - See detailed comparison
3. Revert to original - Simplify mockup to match brief
4. Other
```

#### If Drift Requiring Attention:

```
⚠️ Design drift detected

**Issues found:**
1. Missing feature: "Preset browser" mentioned in brief, absent in mockup
   - Brief line 42: "Include preset system for patch recall"
   - Mockup: No preset-related components
2. Visual style mismatch:
   - Brief: "Vintage warmth with analog aesthetics"
   - Mockup: Stark modern minimal with flat colors
3. Parameter count: 5 (brief: 12) - significant reduction
   - Removed: FEEDBACK_TONE, MOD_DEPTH, SPREAD, MIX, WIDTH, DRY, WET

**Recommendation:** Address drift before implementation

**Confidence:** HIGH (clear quantitative + semantic misalignment)

What's next?
1. Update mockup - Add missing features, adjust style to match brief
2. Update brief - Brief overpromised, mockup is realistic scope
3. Continue anyway (override) - Accept drift, proceed with mockup as-is
4. Review detailed comparison - See side-by-side analysis
5. Other
```

#### If Critical Drift:

```
❌ Critical design drift - Implementation BLOCKED

**Critical issues:**
1. Brief core concept: "Tempo-synced rhythmic delay with modulation"
   Mockup delivers: Basic feedback delay (no tempo sync, no modulation)
2. Parameter count: 25 (brief: 5) - 5x scope creep
   - Brief: DELAY_TIME, FEEDBACK, MIX, TONE, DRIVE
   - Mockup: 20 additional parameters for features not mentioned in brief

**Action required:** Resolve drift before implementation can proceed

**Why blocking:**
Mockup contradicts core concept. Implementation would not match user's vision.
High risk of complete rework.

What's next?
1. Update mockup - Align with brief's core concept (tempo sync + modulation)
2. Update brief - Revise concept to match mockup's approach (basic delay)
3. Start over - Create new mockup from brief
4. Other

(Option to override not provided - critical drift must be resolved)
```

### Step 6: Execute User Choice

**Option 1: Update brief and continue**

Add "Design Evolution" section to creative-brief.md:

```markdown
## Design Evolution

### [Date] - UI Mockup v[N] Finalization

**Changes from original vision:**

**Parameters:**

- Added: TONE, DRIVE, MIX, WIDTH (brief: 8 parameters, mockup: 12)

**Features:**

- Added: Visual feedback meter for delay time visualization

**Visual Style:**

- Refined: Gradient backgrounds for depth (brief: solid colors)
- Refined: Animation polish on parameter changes

**Rationale:**

- Additional parameters (TONE, DRIVE, MIX, WIDTH) provide necessary tonal shaping
  not anticipated in original brief.
- Visual meter improves usability during performance by showing delay time graphically.
- Gradient backgrounds enhance vintage aesthetic mentioned in brief.

**User approval:** [Date] - Confirmed these changes align with plugin vision
```

Update brief's "UI Vision" section to reflect current mockup (preserve original in "Evolution" section).

**Option 2: Update mockup**

- Return user to ui-mockup skill
- Present drift findings
- Iterate design to align with brief
- Re-run design-sync after changes

**Option 3: Continue anyway (override)**

- Log override to `.validator-overrides.yaml`:
  ```yaml
  - date: 2025-11-10
    validator: design-sync
    finding: parameter-count-mismatch
    severity: attention
    override-reason: User confirmed evolution is intentional
    mockup-version: v3
  ```
- Return success (allow implementation)
- Warn: "Implementation may not match original vision"

### Step 7: Route Back to ui-mockup

**After Step 6 actions complete (brief updated, mockup changed, or override logged), return to ui-mockup Phase 5.5 decision menu.**

**Present this menu:**

```
✓ Design-brief alignment complete

What's next?
1. Finalize and create implementation files (if satisfied and aligned)
2. Provide more refinements (iterate on design) ← Creates v[N+1]
3. Test in browser (open v[N]-ui-test.html)
4. Save as aesthetic template (add to library for reuse)
5. Finalize AND save aesthetic (do both operations)
6. Other

Choose (1-6): _
```

**This is the same decision point as ui-mockup Phase 5.5, minus the "Check alignment" option (already done).**

**Option handling:**
- **Option 1**: Proceed to ui-mockup Phase 6-9 (generate remaining 5 files)
- **Option 2**: Return to ui-mockup Phase 2 with new version number (iterate design)
- **Option 3**: Open test HTML in browser for review
- **Option 4**: Invoke ui-template-library "save" operation
- **Option 5**: Save aesthetic, then proceed to Phase 6-9
- **Option 6**: Other

**Why route back:**
- Validates the design is aligned before generating implementation files
- Prevents generating C++ boilerplate for misaligned mockups
- Maintains checkpoint protocol (user decides next action after validation)

---

## Integration with ui-mockup

**ui-mockup Phase 4.5** (after finalization, before C++ generation):

```markdown
✓ Mockup finalized: v3

parameter-spec.md generated (12 parameters)

What's next?

1. Check alignment - Run design-sync validation (recommended)
2. Generate implementation files - Proceed to Phase 5
3. Iterate design - Make more changes
4. Other
```

If user chooses "Check alignment" → invoke design-sync skill

**Flow:**

1. ui-mockup finishes Phase 4 (generates YAML + test HTML + parameter-spec.md)
2. Presents decision menu with "Check alignment" option
3. If chosen: Invokes design-sync skill
4. design-sync validates, presents findings
5. User resolves any drift
6. Returns to ui-mockup to continue Phase 5 (or iterate)

---

## Integration with plugin-workflow Stage 1

**Optional pre-check before planning:**

Stage 1 Planning detects finalized mockup:

```markdown
Starting Stage 1: Planning

Contracts loaded:

- creative-brief.md: EXISTS
- parameter-spec.md: EXISTS (from mockup v3)

Mockup finalized: Yes (v3)
design-sync validation: Not yet run

Recommendation: Validate brief ↔ mockup alignment before planning

What's next?

1. Run design-sync - Validate alignment (recommended)
2. Skip validation - Proceed with planning (trust mockup)
3. Other
```

If user chooses "Run design-sync" → invoke this skill

**Why pre-check matters:**

Stage 1 generates plan.md based on parameter-spec.md. If parameter-spec doesn't match brief vision, plan will be misaligned. Better to catch drift before planning.

---

## Success Criteria

Validation is successful when:

- ✅ Both contracts loaded (brief + parameter-spec + mockup YAML)
- ✅ Quantitative checks completed (parameter count, feature detection)
- ✅ Semantic validation performed (extended thinking analysis)
- ✅ Drift category assigned (none / acceptable / attention / critical)
- ✅ Appropriate decision menu presented
- ✅ User action executed (update brief / update mockup / override)

---

## Error Handling

**Missing contracts:**

```
❌ Cannot validate: creative-brief.md not found

design-sync requires creative brief to validate against mockup.

Actions:
1. Create brief - Document vision before validation
2. Skip validation - Proceed without sync (not recommended)
3. Other
```

**No mockup finalized:**

```
❌ Cannot validate: No finalized mockup found

design-sync requires:
- mockups/v[N]-ui.yaml (finalized mockup spec)
- parameter-spec.md (generated from mockup)

Current state:
- Mockups: v1-ui.yaml, v2-ui.yaml (no parameter-spec.md)

Actions:
1. Finalize mockup - Generate parameter-spec.md via ui-mockup
2. Skip validation - Proceed without sync (not recommended)
3. Other
```

**Ambiguous findings:**

```
⚠️ Drift assessment uncertain

Quantitative checks:
- Parameter count: 10 (brief: 8) +2 parameters ← Minor difference
- Features: All present

Semantic validation:
- Visual style: MEDIUM confidence alignment
  - Brief aesthetic: "Vintage warmth"
  - Mockup aesthetic: "Brushed metal with warm colors"
  - Ambiguous: Is brushed metal "vintage"?

**Asking for user input:**
Is this mockup style aligned with your "vintage warmth" vision?

1. Yes - Style matches intent (acceptable evolution)
2. No - Style misses the mark (drift requiring attention)
3. Review side-by-side - See comparison
4. Other
```

**Override tracking:**

All overrides logged to `.validator-overrides.yaml`:

```yaml
overrides:
  - timestamp: 2025-11-10T14:32:00Z
    validator: design-sync
    plugin: DelayPlugin
    mockup-version: v3
    finding: parameter-count-increase
    severity: attention
    details: "Brief: 8 parameters, Mockup: 12 parameters (+4)"
    override-reason: "User confirmed: added parameters are intentional (TONE, DRIVE, MIX, WIDTH)"
    approved-by: User
```

Enables audit trail: "Why did we proceed with drift?"

---

## Notes for Claude

**When executing this skill:**

1. Always load all three contracts first (brief, parameter-spec, mockup YAML)
2. Run quantitative checks before semantic validation (faster)
3. Use extended thinking for semantic validation (8k budget)
4. Categorize drift objectively (use thresholds from references/drift-detection.md)
5. Present appropriate decision menu (based on drift category)
6. Update brief's Evolution section if user approves evolution
7. Log overrides to .validator-overrides.yaml
8. Never auto-override (user must explicitly choose)

**Common pitfalls:**

- Forgetting to use extended thinking for semantic validation (critical for accuracy)
- Auto-categorizing as "acceptable" without checking (be objective)
- Not presenting "Continue anyway" for non-critical drift (user should have option)
- Providing "override" for critical drift (should be blocked)
- Updating mockup instead of brief (mockup is source of truth, brief gets updated)
- Not logging overrides (audit trail required)

---

## Quality Guidelines

**Good drift detection:**

- ✅ Specific findings ("Missing feature: preset browser from brief line 42")
- ✅ Quantitative evidence (parameter counts, feature lists)
- ✅ Semantic reasoning (why style does/doesn't match aesthetic)
- ✅ Confidence levels stated (HIGH/MEDIUM/LOW)
- ✅ Actionable recommendations

**Avoid:**

- ❌ Vague assessments ("Mockup looks different")
- ❌ Missing quantitative data (no parameter counts)
- ❌ No semantic analysis (just counting, no reasoning)
- ❌ Unclear confidence (is this certain or uncertain?)
- ❌ No clear recommendations (what should user do?)

---

## Example Scenarios

### Scenario 1: No Drift (Quick Validation)

**Plugin:** SimpleGain
**Brief:** 1 parameter (GAIN), minimal UI, slider only
**Mockup:** 1 parameter (GAIN), single vertical slider, dark background

**Quantitative:**

- Parameter count: 1 (matches brief)
- Features: None mentioned, none in mockup
- Delta: 0

**Semantic:**

- Brief aesthetic: "Minimal, single slider"
- Mockup aesthetic: Single vertical slider, dark theme
- Alignment: HIGH confidence YES

**Finding:** No drift

**Output:**

```
✓ Design-brief alignment verified

- Parameter count: 1 (matches brief: GAIN)
- Features: None (as specified)
- Visual style aligned: Minimal single-slider design

What's next?
1. Continue implementation (recommended) - Alignment confirmed
2. Review details - See full comparison
3. Other
```

**Time:** 2 minutes

### Scenario 2: Acceptable Evolution

**Plugin:** ReverbPlugin
**Brief:** 3 parameters (SIZE, DAMPING, MIX), "Dark minimal theme"
**Mockup:** 5 parameters (SIZE, DAMPING, PRE_DELAY, TONE, MIX), dark theme with gradient background

**Quantitative:**

- Parameter count: 5 (brief: 3) +2 parameters
- Features: All mentioned present, no missing
- Delta: +2

**Semantic:**

- Brief aesthetic: "Dark minimal theme"
- Mockup aesthetic: Dark gradient background, clean layout
- Added: PRE_DELAY (common reverb parameter), TONE (tonal shaping)
- Alignment: HIGH confidence YES (evolution, not drift)

**Finding:** Acceptable evolution

**Output:**

```
⚠️ Design evolution detected (acceptable)

**Changes from brief:**
- Parameter count: 5 (brief: 3) +2 parameters
  - Added: PRE_DELAY, TONE
- Visual refinements: Gradient background (brief: solid dark)

**Assessment:** Reasonable evolution

**Reasoning:**
PRE_DELAY and TONE are standard reverb parameters that enhance usability. Gradient
background adds depth while preserving dark minimal aesthetic. Core concept intact.

What's next?
1. Update brief and continue (recommended) - Document evolution
2. Review changes - See detailed comparison
3. Revert to original - Simplify mockup to match brief
4. Other
```

**Time:** 4 minutes

### Scenario 3: Drift Requiring Attention

**Plugin:** DelayPlugin
**Brief:** 8 parameters including "TEMPO_SYNC" for tempo-synced delay, "Dark theme with blue accents"
**Mockup:** 5 parameters (no TEMPO_SYNC), bright orange theme

**Quantitative:**

- Parameter count: 5 (brief: 8) -3 parameters
- Missing features: TEMPO_SYNC (mentioned in brief)
- Delta: -3 (scope reduction)

**Semantic:**

- Brief aesthetic: "Dark theme with blue accents"
- Mockup aesthetic: Bright orange on white
- Feature: Brief mentions tempo sync, mockup lacks it
- Alignment: HIGH confidence NO (clear mismatch)

**Finding:** Drift requiring attention

**Output:**

```
⚠️ Design drift detected

**Issues found:**
1. Missing core feature: "TEMPO_SYNC" mentioned in brief (line 28)
   - Brief: "Tempo-synced delay times locked to DAW BPM"
   - Mockup: No tempo sync parameter or control
2. Visual style mismatch:
   - Brief: "Dark theme with blue accents"
   - Mockup: Bright orange on white background
3. Parameter count: 5 (brief: 8) -3 parameters (scope reduction)

**Recommendation:** Address drift before implementation

**Confidence:** HIGH (quantitative: -3 parameters, semantic: clear mismatch)

What's next?
1. Update mockup - Add TEMPO_SYNC, change to dark/blue theme
2. Update brief - Remove tempo sync requirement, adjust aesthetic
3. Continue anyway (override) - Accept drift, proceed with mockup
4. Review detailed comparison
5. Other
```

**Time:** 6 minutes

### Scenario 4: Critical Drift (Blocked)

**Plugin:** WavetableSynth
**Brief:** 5 parameters (OSC_SHAPE, PITCH, FILTER_CUTOFF, RESONANCE, VOLUME), "Wavetable oscillator with filter"
**Mockup:** 25 parameters including granular synthesis, effects chain, modulation matrix, "Complex modular synth"

**Quantitative:**

- Parameter count: 25 (brief: 5) +20 parameters (5x scope creep)
- Features: Brief mentions wavetable + filter. Mockup adds granular, effects, modulation.
- Delta: +20 (massive increase)

**Semantic:**

- Brief concept: "Simple wavetable synth with filter"
- Mockup concept: "Complex modular synth with granular + effects"
- Alignment: HIGH confidence NO (contradicts core concept)

**Finding:** Critical drift (BLOCKING)

**Output:**

```
❌ Critical design drift - Implementation BLOCKED

**Critical issues:**
1. Brief core concept: "Simple wavetable synth with filter (5 parameters)"
   Mockup delivers: Complex modular synth (25 parameters) with granular synthesis,
   effects chain, and modulation matrix
2. Parameter count: 25 (brief: 5) - 5x scope creep
3. Complexity: Brief = "Simple", Mockup = "Complex modular"

**Why blocking:**
Mockup fundamentally contradicts brief's core concept of simplicity. Implementation
would deliver a completely different plugin than envisioned. High risk of rework.

**Action required:** Resolve drift before implementation can proceed

What's next?
1. Update mockup - Return to simple wavetable + filter concept
2. Update brief - Revise vision to match complex modular design
3. Start over - Create new mockup aligned with brief
4. Other

(Continue anyway option not provided - critical drift must be resolved)
```

**Time:** 8 minutes (extended thinking for thorough analysis)

---

## Future Enhancements

**Not in Phase 7 scope, but potential:**

- Visual diff tool (side-by-side brief vs mockup comparison)
- Automated brief updating (AI-generated Evolution sections)
- Historical drift tracking (drift patterns over time)
- Confidence calibration (track prediction accuracy)
- Integration with version control (git diff for brief changes)
