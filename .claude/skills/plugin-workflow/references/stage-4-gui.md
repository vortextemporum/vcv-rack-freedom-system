# Stage 3: GUI

**Context:** This file is part of the plugin-workflow skill.
**Invoked by:** Main workflow dispatcher after Stage 3 completion
**Purpose:** Integrate WebView UI with parameter bindings

---

**Goal:** Integrate WebView UI with parameter bindings

**Duration:** 20-60 minutes (depending on complexity)

**Preconditions:**

- Stage 3 complete and tests passed (DSP operational)
- Finalized UI mockup exists (v[N]-ui.html)
- parameter-spec.md exists

**Actions:**

### 1. Locate Finalized UI Mockup

**Scan for finalized mockup version:**

```bash
cd plugins/${PLUGIN_NAME}/.ideas/mockups/

# Find highest version number
LATEST_MOCKUP=$(ls -1 v*-ui.html 2>/dev/null | sort -V | tail -1)

if [ -z "$LATEST_MOCKUP" ]; then
  echo "
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ BLOCKED: Cannot proceed to Stage 5
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Missing required contract: Finalized UI mockup

Stage 5 requires a finalized UI mockup (v[N]-ui.html) to integrate.

WHY BLOCKED:
Without a finalized UI mockup, gui-agent cannot know:
- UI layout and styling
- Parameter control types (sliders, knobs, toggles)
- Visual design and branding
- Dimensions and layout

HOW TO UNBLOCK:
1. Run /mockup to create UI mockup
2. Design UI through iterative refinement
3. Finalize a design version (marks it as v[N])
4. Then resume Stage 5 with /continue ${PLUGIN_NAME}

Current status: No finalized mockup found in .ideas/mockups/

Cannot proceed without this contract.
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  "
  exit 1
fi

echo "✓ Found finalized mockup: $LATEST_MOCKUP"
```

**If no mockup found:** STOP IMMEDIATELY, exit skill, wait for user to create mockup.

### 2. Read Complexity and Check for Phases

Same as Stage 3 - check if phased implementation needed:

```typescript
const planContent = readFile(`plugins/${pluginName}/.ideas/plan.md`);
const complexityScore = extractComplexityScore(planContent);
const hasPhases =
  planContent.includes("### Phase 4.1") ||
  planContent.includes("## Stage 3: GUI Phases");

console.log(
  `Complexity: ${complexityScore} (${hasPhases ? "phased" : "single-pass"})`
);
```

### 3a. Single-Pass Implementation (Complexity ≤2)

**If complexity ≤2 OR no phases defined:**

**Read JUCE 8 critical patterns:**

```typescript
const criticalPatterns = await Read({
  file_path: "troubleshooting/patterns/juce8-critical-patterns.md"
});
```

Invoke gui-agent once for complete UI integration:

```typescript
const mockupPath = findLatestMockup(pluginName);

const guiResult = Task({
  subagent_type: "gui-agent",
  description: `Integrate WebView UI for ${pluginName}`,
  prompt: `CRITICAL PATTERNS (MUST FOLLOW):

${criticalPatterns}

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Integrate WebView UI for plugin at plugins/${pluginName}.

Inputs:
- Finalized UI mockup: ${mockupPath}
- parameter-spec.md: plugins/${pluginName}/.ideas/parameter-spec.md
- creative-brief.md: plugins/${pluginName}/.ideas/creative-brief.md
- Plugin name: ${pluginName}
- Complexity: ${complexityScore} (single-pass)

Tasks:
1. Read finalized UI mockup from ${mockupPath}
2. Create ui/public/ directory at plugins/${pluginName}/ui/public/
3. Copy mockup to plugins/${pluginName}/ui/public/index.html
4. Download JUCE frontend library to ui/public/juce-framework.js
5. Add relay members to PluginEditor.h (CRITICAL ORDER: Relays first, then WebView, then Attachments)
6. Implement parameter bindings in PluginEditor.cpp
7. Add WebView initialization in PluginEditor constructor
8. Update CMakeLists.txt to enable JUCE_WEB_BROWSER=1
9. Verify all parameter IDs from parameter-spec.md match HTML element IDs exactly
10. Return JSON report with binding count and member order verification

⚠️ CRITICAL: Member declaration order (Relays → WebView → Attachments) prevents 90% of release build crashes.

Build verification handled by workflow after agent completes.
  `,
});

// Parse and validate report
const report = parseSubagentReport(guiResult);

if (report.status === "success") {
  console.log(`✓ UI Integrated`);
  console.log(`   Your interface is connected and responding to parameter changes`);
  console.log(`  `);
  console.log(`  Mockup version: ${report.outputs.ui_mockup_version}`);
  console.log(`  Bindings: ${report.outputs.binding_count}`);
  console.log(
    `  All parameters bound: ${
      report.outputs.all_parameters_bound ? "Yes" : "No"
    }`
  );
  console.log(
    `  Member order correct: ${
      report.outputs.member_order_correct ? "Yes" : "No"
    }`
  );

  // Continue to auto-test (Step 5)
}
```

### 3b. Phased Implementation (Complexity ≥3)

**If complexity ≥3 AND plan.md defines UI phases:**

Parse phase breakdown (similar to Stage 3):

```typescript
// Extract Phase 4.1, 5.2, etc. from plan.md
const phasePattern = /### Phase (5\.\d+):\s*(.+?)\n/g;
const phases = [];
let match;

while ((match = phasePattern.exec(planContent)) !== null) {
  phases.push({
    number: match[1], // e.g., "5.1"
    description: match[2], // e.g., "Layout and Basic Controls"
  });
}

console.log(`Stage 5 will execute in ${phases.length} phases:`);
phases.forEach((phase) => {
  console.log(`  - Phase ${phase.number}: ${phase.description}`);
});

**Read JUCE 8 critical patterns (used for all phases):**

```typescript
const criticalPatterns = await Read({
  file_path: "troubleshooting/patterns/juce8-critical-patterns.md"
});
```

// Execute each phase sequentially (same pattern as Stage 3)
for (let i = 0; i < phases.length; i++) {
  const phase = phases[i];

  console.log(`\n━━━ Stage ${phase.number} - ${phase.description} ━━━\n`);

  const phaseResult = Task({
    subagent_type: "gui-agent",
    description: `Implement UI Phase ${phase.number} for ${pluginName}`,
    prompt: `CRITICAL PATTERNS (MUST FOLLOW):

${criticalPatterns}

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

You are gui-agent. Your task is to implement Phase ${phase.number} of UI for ${pluginName}.

**Current Phase:** ${phase.number} - ${phase.description}
**Total Phases:** ${phases.length}

[Include mockup, contracts, phase-specific instructions]

Return JSON report with phase_completed: "${phase.number}".
    `,
  });

  const phaseReport = parseSubagentReport(phaseResult);

  if (!phaseReport || phaseReport.status === "failure") {
    console.log(`✗ Phase ${phase.number} failed`);
    // 4-option menu, stop on failure
    return;
  }

  // Phase succeeded - git commit, update plan, handoff, decision menu
  console.log(`✓ Phase ${phase.number} complete`);

  // Git commit for this phase
  bash(`
git add plugins/${pluginName}/Source/
git add plugins/${pluginName}/ui/
git add plugins/${pluginName}/CMakeLists.txt
git add plugins/${pluginName}/.ideas/plan.md
git add plugins/${pluginName}/.continue-here.md

git commit -m "feat: ${pluginName} Stage ${phase.number} - ${phase.description}"
  `);

  // Decision menu between phases
  if (i < phases.length - 1) {
    console.log(`
✓ Phase ${phase.number} complete

Progress: ${i + 1} of ${phases.length} phases complete

What's next?
1. Continue to Phase ${phases[i + 1].number} (recommended)
2. Review Phase ${phase.number} code
3. Test UI in DAW
4. Pause here
5. Other

Choose (1-5): _
    `);

    const choice = getUserInput();
    if (choice === "4" || choice === "pause") {
      console.log(`\n⏸ Paused between UI phases. Resume with /continue.`);
      return;
    }
  }
}

console.log(`\n✓ All ${phases.length} UI phases complete!`);
```

### 4. Handle Non-Phased Success

**After single-pass success:**

```typescript
// Update state
updateHandoff(
  pluginName,
  5,
  "Stage 3: GUI - WebView UI integrated, all parameters bound",
  ["Auto-test Stage 5", "Test UI in DAW", "Review bindings"],
  complexityScore,
  false
);

updatePluginStatus(pluginName, "🚧 Stage 5");
updatePluginTimeline(
  pluginName,
  5,
  `GUI complete - ${report.outputs.binding_count} parameter bindings`
);

// Git commit
bash(`
git add plugins/${pluginName}/Source/
git add plugins/${pluginName}/ui/
git add plugins/${pluginName}/CMakeLists.txt
git add plugins/${pluginName}/.continue-here.md
git add PLUGINS.md

git commit -m "$(cat <<'EOF'
feat: ${pluginName} Stage 5 - GUI

WebView UI integrated from ${report.outputs.ui_mockup_version}
${report.outputs.binding_count} parameter bindings created
Member order: Relays → WebView → Attachments
CMakeLists.txt updated for WebView support

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"
`);
```

### 5. Handle Failure

**4-option failure menu:**

```typescript
if (report.status === "failure") {
  // Special handling for binding_mismatch error
  if (report.outputs.error_type === "binding_mismatch") {
    console.log(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ Stage 5 Failed: Parameter Binding Mismatch
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Error: Not all parameters from parameter-spec.md have UI bindings

Expected: ${report.outputs.expected_count} parameters
Implemented: ${report.outputs.actual_count} relay/attachment pairs
Missing: ${report.outputs.missing_bindings.join(", ")}

This is a ZERO-DRIFT violation. All parameters must be bound to UI.

What would you like to do?
1. Investigate (why were bindings missing?)
2. Show me the code (PluginEditor.h)
3. Show me the parameter-spec.md
4. I'll fix it manually (say "resume automation" when ready)

Choose (1-4): _
    `);
  } else {
    // Other errors (build failure, member order wrong, etc.)
    console.log(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ Stage 5 Failed: UI Integration
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Error: ${report.outputs.error_message}
Type: ${report.outputs.error_type}

Issues:
${report.issues.map((i) => `  - ${i}`).join("\n")}

What would you like to do?
1. Investigate
2. Show me the code (PluginEditor.h/cpp)
3. Show me the build output
4. I'll fix it manually (say "resume automation" when ready)

Choose (1-4): _
    `);
  }

  // Handle choice (same pattern as Stages 2-4)
}
```

### 6. Auto-Invoke plugin-testing Skill

**After Stage 5 succeeds (all phases if phased):**

```typescript
console.log("\n━━━ Running automated tests (including UI validation) ━━━\n");

const testResult = Task({
  subagent_type: "general-purpose",
  description: `Test ${pluginName} after Stage 5`,
  prompt: `
Run automated tests for ${pluginName} after Stage 5 UI integration.

Use the plugin-testing skill to run all 5 tests plus UI-specific checks:
1. Build test (already passed)
2. Load test (plugin loads without crash)
3. Process test (audio processing works)
4. Parameter test (parameters affect audio)
5. State test (save/load works)
6. UI test (WebView renders, parameters sync)

Report results.
  `,
});

console.log(testResult);

// Parse test results
const testsPassed =
  testResult.includes("✓ All tests passed") || testResult.includes("PASS");

if (!testsPassed) {
  console.log(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ Stage 5 Tests FAILED
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Stage 5 UI integration completed, but automated tests failed.

Test results:
${testResult}

CANNOT proceed to Stage 6 (validation) with failing UI tests.

What would you like to do?
1. Investigate test failures
2. Show me the UI code
3. Show me the test output
4. I'll fix it manually (say "resume automation" when ready)

Choose (1-4): _
  `);

  // STOP - Do not proceed to Stage 6 with failing tests
  return;
}

console.log("✓ All Stage 5 tests passed (including UI validation)");
```

### 7. Invoke validator for Complexity ≥4 Plugins

**For complexity ≥4 plugins, run semantic validation:**

````typescript
if (complexityScore >= 4) {
  console.log("\n━━━ Running semantic validation (complexity ≥4) ━━━\n");

  // Read contracts
  const paramSpecContent = readFile(
    `plugins/${pluginName}/.ideas/parameter-spec.md`
  );
  const architectureContent = readFile(
    `plugins/${pluginName}/.ideas/architecture.md`
  );
  const planContent = readFile(`plugins/${pluginName}/.ideas/plan.md`);

  const validationResult = Task({
    subagent_type: "validation-agent",
    description: `Validate ${pluginName} Stage 5`,
    prompt: `
Validate Stage 5 completion for ${pluginName}.

**Stage:** 5
**Plugin:** ${pluginName}
**Contracts:**
- parameter-spec.md: ${paramSpecContent}
- architecture.md: ${architectureContent}
- plan.md: ${planContent}

**Expected outputs for Stage 3:**
- Member declaration order correct (Relays → WebView → Attachments)
- All parameters from spec have UI bindings
- HTML element IDs match relay names
- UI aesthetic matches mockup
- Visual feedback (knobs respond to parameter changes)
- WebView initialization includes error handling
- Binary data embedded correctly

Return JSON validation report with status, checks, and recommendation.
    `,
  });

  // Parse JSON report (robust handling)
  let validationReport;
  try {
    // Try extracting JSON from markdown code blocks first
    const jsonMatch =
      validationResult.match(/```json\n([\s\S]*?)\n```/) ||
      validationResult.match(/```\n([\s\S]*?)\n```/);

    if (jsonMatch) {
      validationReport = JSON.parse(jsonMatch[1]);
    } else {
      // Try parsing entire response as JSON
      validationReport = JSON.parse(validationResult);
    }
  } catch (error) {
    console.log(`
⚠️ Warning: Could not parse validator report as JSON

Raw validator output:
${validationResult}

Continuing workflow (validation is advisory, not blocking).
    `);
    validationReport = null;
  }

  // Present findings if report parsed successfully
  if (validationReport) {
    const { status, checks, recommendation, continue_to_next_stage } =
      validationReport;

    console.log(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
${status === "PASS" ? "✓" : "✗"} Validator ${status}: Stage 5 Review
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    `);

    // Group checks by severity
    const errors = checks.filter((c) => c.severity === "error");
    const warnings = checks.filter((c) => c.severity === "warning");
    const info = checks.filter((c) => c.severity === "info");

    if (errors.length > 0) {
      console.log("\n❌ Errors:");
      errors.forEach((e) => console.log(`  - ${e.message}`));
    }

    if (warnings.length > 0) {
      console.log("\n⚠️  Warnings:");
      warnings.forEach((w) => console.log(`  - ${w.message}`));
    }

    if (info.length > 0 && info.length <= 5) {
      console.log("\nℹ️  Info:");
      info.forEach((i) => console.log(`  - ${i.message}`));
    }

    console.log(`\nRecommendation: ${recommendation}`);

    // Present decision menu with validation findings
    console.log(`

What's next?
1. Complete validation - Final polish and testing ${
      continue_to_next_stage ? "(recommended by validator)" : ""
    }
2. Address validator ${errors.length > 0 ? "errors" : "warnings"} first
3. Review validator report details
4. Test UI manually in DAW - Try out your interface
5. Review UI code
6. Other

Choose (1-6): _
    `);

    // User decides next action based on findings
    // (Advisory layer - user makes final call)
  }
}
````

### 8. Decision Menu (After Tests Pass)

```
✓ UI Integrated
   Your interface is connected and responding to parameter changes

Plugin: [PluginName]
UI: WebView integrated from [mockup version]
Bindings: [N] parameters bound to UI
Tests: All passed (including UI sync)
Status: Ready for final validation

What's next?
1. Complete validation - Final polish and testing (recommended)
2. Test UI in DAW - Try out your interface
3. Review UI code - See implementation details
4. Refine styling - Polish visual design
5. Pause workflow - Resume anytime
6. Other

Choose (1-6): _
```

**CRITICAL: Do NOT proceed to Stage 6 if tests fail.**

---

**Return to:** Main plugin-workflow orchestration in `SKILL.md`
