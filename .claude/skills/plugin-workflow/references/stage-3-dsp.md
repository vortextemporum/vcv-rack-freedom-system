# Stage 3: DSP

**Context:** This file is part of the plugin-workflow skill.
**Invoked by:** Main workflow dispatcher after Stage 2 completion
**Purpose:** Implement audio processing where parameters control DSP

---

**Goal:** Implement audio processing, parameters control DSP

**Duration:** 15-45 minutes (depending on complexity)

**Preconditions:**

- Stage 2 complete (parameters implemented)
- architecture.md exists (DSP component specifications)
- plan.md exists (complexity score, phase breakdown)

**Actions:**

### 1. Read Complexity Score from plan.md

Determine if phased implementation is required:

```typescript
const planContent = readFile(`plugins/${pluginName}/.ideas/plan.md`);

// Extract complexity score
const complexityMatch = planContent.match(/Complexity Score:\*\*\s*([\d.]+)/);
const complexityScore = complexityMatch ? parseFloat(complexityMatch[1]) : 0;

// Check if plan specifies phases
const hasPhases =
  planContent.includes("### Phase 3.1") ||
  planContent.includes("## Stage 3: DSP Phases");

console.log(
  `Complexity: ${complexityScore} (${hasPhases ? "phased" : "single-pass"})`
);
```

### 2a. Single-Pass Implementation (Complexity ≤2)

**If complexity ≤2 OR no phases defined:**

**Read JUCE 8 critical patterns:**

```typescript
const criticalPatterns = await Read({
  file_path: "troubleshooting/patterns/juce8-critical-patterns.md"
});
```

Invoke dsp-agent once for complete DSP implementation:

```typescript
const dspResult = Task({
  subagent_type: "dsp-agent",
  description: `Implement DSP for ${pluginName}`,
  prompt: `CRITICAL PATTERNS (MUST FOLLOW):

${criticalPatterns}

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Implement audio processing for plugin at plugins/${pluginName}.

Inputs:
- architecture.md: plugins/${pluginName}/.ideas/architecture.md
- parameter-spec.md: plugins/${pluginName}/.ideas/parameter-spec.md
- plan.md: plugins/${pluginName}/.ideas/plan.md
- Plugin name: ${pluginName}
- Complexity: ${complexityScore} (single-pass)

Tasks:
1. Read architecture.md and identify all DSP components to implement
2. Read parameter-spec.md to map parameters to DSP controls
3. Add member variables for DSP components to PluginProcessor.h
4. Implement prepareToPlay() - initialize DSP at sample rate
5. Implement processBlock() with all DSP components from architecture.md
6. Connect all parameters to their DSP controls
7. Use juce::ScopedNoDenormals in processBlock for real-time safety
8. Ensure no memory allocations in audio thread
9. Return JSON report with components list and real-time safety status

Build verification handled by workflow after agent completes.
  `,
});

// Parse and validate report (same as Stage 2/3)
const report = parseSubagentReport(dspResult);

if (report.status === "success") {
  console.log(`✓ Audio Engine Working`);
  console.log(`   Your DSP processing is implemented and functional`);
  console.log(`  `);
  console.log(
    `  Components: ${report.outputs.dsp_components_implemented.length}`
  );
  console.log(
    `  Parameters connected: ${report.outputs.parameters_connected.length}`
  );
  console.log(
    `  Real-time safe: ${report.outputs.real_time_safe ? "Yes" : "No"}`
  );

  // Continue to auto-test (Step 5)
}
```

### 2b. Phased Implementation (Complexity ≥3)

**If complexity ≥3 AND plan.md defines phases:**

Parse phase breakdown from plan.md:

```typescript
// Extract phases from plan.md
const phasePattern = /### Phase (4\.\d+):\s*(.+?)\n/g;
const phases = [];
let match;

while ((match = phasePattern.exec(planContent)) !== null) {
  phases.push({
    number: match[1], // e.g., "4.1"
    description: match[2], // e.g., "Core Processing"
  });
}

console.log(`Stage 4 will execute in ${phases.length} phases:`);
phases.forEach((phase) => {
  console.log(`  - Phase ${phase.number}: ${phase.description}`);
});
```

**Read JUCE 8 critical patterns (used for all phases):**

```typescript
const criticalPatterns = await Read({
  file_path: "troubleshooting/patterns/juce8-critical-patterns.md"
});
```

**Execute each phase sequentially:**

```typescript
for (let i = 0; i < phases.length; i++) {
  const phase = phases[i];

  console.log(`\n━━━ Stage ${phase.number} - ${phase.description} ━━━\n`);

  // Determine model based on complexity
  const model = complexityScore >= 4 ? "opus" : "sonnet";

  const phaseResult = Task({
    subagent_type: "dsp-agent",
    description: `Implement DSP Phase ${phase.number} for ${pluginName}`,
    prompt: `CRITICAL PATTERNS (MUST FOLLOW):

${criticalPatterns}

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Implement Phase ${phase.number} of DSP for plugin at plugins/${pluginName}.

Inputs:
- architecture.md: plugins/${pluginName}/.ideas/architecture.md
- parameter-spec.md: plugins/${pluginName}/.ideas/parameter-spec.md
- plan.md (Phase ${phase.number}): plugins/${pluginName}/.ideas/plan.md
- Plugin name: ${pluginName}
- Complexity: ${complexityScore}
- Current phase: ${phase.number} - ${phase.description}
- Total phases: ${phases.length}

Tasks:
1. Read plan.md and extract Phase ${phase.number} components only
2. Read architecture.md for component specifications
3. Add member variables for Phase ${phase.number} DSP components
4. Implement Phase ${phase.number} components in processBlock()
5. Build on existing code from previous phases (do not remove)
6. Connect Phase ${phase.number} parameters only
7. Ensure real-time safety (no allocations, use juce::ScopedNoDenormals)
8. Update plan.md with phase completion timestamp
9. Return JSON report with phase_completed: "${phase.number}"

CRITICAL: Implement ONLY Phase ${phase.number} components, preserve all previous phase code.

Build verification handled by workflow after agent completes.
    `,
  });

  const phaseReport = parseSubagentReport(phaseResult);

  if (!phaseReport || phaseReport.status === "failure") {
    console.log(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ Stage ${phase.number} Failed
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Phase ${phase.number} (${phase.description}) failed.

Error: ${phaseReport?.outputs.error_message || "Unknown error"}

What would you like to do?
1. Investigate
2. Show me the code
3. Show me the build output
4. I'll fix it manually (say "resume automation" when ready)

Choose (1-4): _
    `);

    // Handle failure (same 4-option menu as Stage 2/3)
    return; // Stop phased execution on failure
  }

  // Phase succeeded
  console.log(`✓ Phase ${phase.number} complete: ${phase.description}`);
  console.log(
    `  - Components: ${
      phaseReport.outputs.components_this_phase?.join(", ") || "N/A"
    }`
  );

  // Git commit for this phase
  await bash(`
git add plugins/${pluginName}/Source/
git add plugins/${pluginName}/.ideas/plan.md
git add plugins/${pluginName}/.continue-here.md

git commit -m "$(cat <<'EOF'
feat: ${pluginName} Stage ${phase.number} - ${phase.description}

Phase ${i + 1} of ${phases.length} complete
Components implemented this phase

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"
  `);

  console.log(`✓ Committed: ${await bash('git log -1 --format="%h"')}`);

  // Update plan.md with phase timestamp
  const timestamp = new Date().toISOString();
  updatePlanMd(pluginName, phase.number, timestamp);

  // Update handoff
  updateHandoff(
    pluginName,
    phase.number, // e.g., "4.1"
    `Phase ${phase.number} complete: ${phase.description}`,
    i < phases.length - 1
      ? [
          `Phase ${phases[i + 1].number}: ${phases[i + 1].description}`,
          "Review current phase",
          "Test",
          "Pause",
        ]
      : ["Auto-test Stage 4 output", "Review complete DSP", "Pause"],
    complexityScore,
    true // phased
  );

  // Decision menu after each phase
  if (i < phases.length - 1) {
    // Not last phase
    console.log(`
✓ Phase ${phase.number} complete

Progress: ${i + 1} of ${phases.length} phases complete

What's next?
1. Continue to Phase ${phases[i + 1].number} (${
      phases[i + 1].description
    }) (recommended)
2. Review Phase ${phase.number} code
3. Test current state in DAW
4. Pause here
5. Other

Choose (1-5): _
    `);

    const choice = getUserInput();
    if (choice === "4" || choice === "pause") {
      console.log(
        "\n⏸ Paused between phases. Resume with /continue to continue Phase ${phases[i+1].number}."
      );
      return;
    }
    // Other choices: review, test, etc., then re-present menu
  }

  // Last phase - continue to auto-test
}

console.log(`\n✓ All ${phases.length} phases complete!`);
```

### 3. Handle Non-Phased Success

**After single-pass success:**

```typescript
// Update state
updateHandoff(
  pluginName,
  4,
  "Stage 4: DSP - Audio processing implemented",
  ["Auto-test Stage 4", "Review DSP code", "Test audio manually"],
  complexityScore,
  false
);

updatePluginStatus(pluginName, "🚧 Stage 4");
updatePluginTimeline(
  pluginName,
  4,
  `DSP complete - ${report.outputs.dsp_components_implemented.length} components`
);

// Git commit
bash(`
git add plugins/${pluginName}/Source/
git add plugins/${pluginName}/.continue-here.md
git add PLUGINS.md

git commit -m "$(cat <<'EOF'
feat: ${pluginName} Stage 4 - DSP

Audio processing implemented
${report.outputs.dsp_components_implemented.length} DSP components
All parameters connected to audio engine
Real-time safe implementation

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"
`);
```

### 4. Handle Failure (Single-Pass or Any Phase)

**4-option failure menu:**

```typescript
if (report.status === "failure") {
  console.log(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ Stage 4 Failed: DSP Implementation
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Error: ${report.outputs.error_message}
Type: ${report.outputs.error_type}

Issues:
${report.issues.map((i) => `  - ${i}`).join("\n")}

What would you like to do?
1. Investigate (invoke troubleshoot-agent)
2. Show me the code (PluginProcessor.cpp)
3. Show me the build output
4. I'll fix it manually (say "resume automation" when ready)

Choose (1-4): _
  `);

  // Handle choice (same as Stage 2/3)
}
```

### 5. Auto-Invoke plugin-testing Skill

**After Stage 4 succeeds (all phases if phased):**

```typescript
console.log("\n━━━ Running automated tests ━━━\n");

const testResult = Task({
  subagent_type: "general-purpose", // Or invoke plugin-testing skill directly
  description: `Test ${pluginName} after Stage 4`,
  prompt: `
Run automated tests for ${pluginName} after Stage 4 DSP implementation.

Use the plugin-testing skill to run the 5 automated tests:
1. Build test (already passed)
2. Load test (plugin loads without crash)
3. Process test (audio processing works)
4. Parameter test (parameters affect audio)
5. State test (save/load works)

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
✗ Stage 4 Tests FAILED
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Audio processing implementation completed, but automated tests failed.

Test results:
${testResult}

Cannot integrate UI until audio engine is stable.

What would you like to do?
1. Investigate test failures - Find root cause
2. Show me the DSP code - Review implementation
3. Show me the test output - See detailed errors
4. I'll fix it manually (say "resume automation" when ready)

Choose (1-4): _
  `);

  // STOP - Do not proceed to Stage 5 with failing tests
  return;
}

console.log("✓ All Stage 4 tests passed");
```

### 6. Invoke validation-agent for Complexity ≥4 Plugins

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
    description: `Validate ${pluginName} Stage 4`,
    prompt: `
Validate Stage 4 completion for ${pluginName}.

**Stage:** 4
**Plugin:** ${pluginName}
**Contracts:**
- parameter-spec.md: ${paramSpecContent}
- architecture.md: ${architectureContent}
- plan.md: ${planContent}

**Expected outputs for Stage 4:**
- All DSP components from architecture.md implemented
- processBlock() contains real-time safe audio processing
- Parameters modulate DSP components correctly
- prepareToPlay() allocates buffers
- No heap allocations in processBlock
- ScopedNoDenormals used
- Edge cases handled (zero-length buffers)

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
⚠️ Warning: Could not parse validation-agent report as JSON

Raw validation-agent output:
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
${status === "PASS" ? "✓" : "✗"} Validator ${status}: Stage 4 Review
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
1. Integrate UI - Connect your interface to the audio engine ${
      continue_to_next_stage ? "(recommended by validation-agent)" : ""
    }
2. Address validation-agent ${errors.length > 0 ? "errors" : "warnings"} first
3. Review validation-agent report details
4. Test audio manually in DAW - Listen to your processing
5. Review DSP code
6. Other

Choose (1-6): _
    `);

    // User decides next action based on findings
    // (Advisory layer - user makes final call)
  }
}
````

### 7. Decision Menu (After Tests Pass)

```
✓ Audio Engine Working
   Your DSP processing is implemented and functional

Plugin: [PluginName]
DSP components: [N]
Parameters connected: [N]
Tests: All passed
Status: Ready for UI integration

What's next?
1. Integrate UI - Connect your interface to the audio engine (recommended)
2. Test audio in DAW - Listen to your processing
3. Review DSP code - See implementation details
4. Fine-tune audio - Adjust processing before UI
5. Pause workflow - Resume anytime
6. Other

Choose (1-6): _
```

**CRITICAL: Do NOT proceed to Stage 5 if tests fail.**

---

**Return to:** Main plugin-workflow orchestration in `SKILL.md`
