# Stage 2: Foundation

**Context:** This file is part of the plugin-workflow skill.
**Invoked by:** Main workflow dispatcher after Stage 1 completion
**Purpose:** Create plugin structure that compiles successfully with JUCE build system

---

**Goal:** Create plugin structure that compiles

**Duration:** 5 minutes

**Preconditions:**
- Stage 1 complete (plan.md exists)
- creative-brief.md exists
- architecture.md exists

## Actions

### 1. Prepare Contracts for Subagent

Read contract files that foundation-agent needs:

```bash
cat plugins/[PluginName]/.ideas/creative-brief.md
cat plugins/[PluginName]/.ideas/architecture.md
cat plugins/[PluginName]/.ideas/plan.md
```

### 2. Invoke foundation-agent via Task Tool

Call foundation-agent subagent with complete specification:

```typescript
const foundationResult = Task({
  subagent_type: "foundation-agent",
  model: "sonnet",
  description: `Create build system for ${pluginName}`,
  prompt: `
You are foundation-agent. Your task is to create the JUCE plugin build system and verify compilation for ${pluginName}.

**Plugin Name:** ${pluginName}
**Plugin Location:** plugins/${pluginName}/

**Contract Files:**

creative-brief.md:
\`\`\`
${creativeBriefContent}
\`\`\`

architecture.md:
\`\`\`
${architectureContent}
\`\`\`

plan.md:
\`\`\`
${planContent}
\`\`\`

Follow the instructions in .claude/agents/foundation-agent.md exactly.

Extract PRODUCT_NAME from creative-brief, determine plugin type from architecture, create CMakeLists.txt, create minimal PluginProcessor and PluginEditor classes, build with --no-install flag.

Return JSON report in the exact format specified in foundation-agent.md.
  `
})
```

### 3. Parse JSON Report with Error Handling

**Implement robust JSON parsing:**

```typescript
function parseSubagentReport(rawOutput: string): object | null {
  try {
    // Strategy 1: Extract from markdown code blocks
    const codeBlockMatch = rawOutput.match(/```json\n([\s\S]*?)\n```/)
    if (codeBlockMatch) {
      return JSON.parse(codeBlockMatch[1])
    }

    // Strategy 2: Find JSON object via brace matching
    const braceStart = rawOutput.indexOf('{')
    const braceEnd = rawOutput.lastIndexOf('}')
    if (braceStart !== -1 && braceEnd !== -1 && braceEnd > braceStart) {
      const jsonCandidate = rawOutput.substring(braceStart, braceEnd + 1)
      return JSON.parse(jsonCandidate)
    }

    // Strategy 3: Try parsing entire output
    return JSON.parse(rawOutput)
  } catch (error) {
    console.error("JSON parsing failed:", error)
    return null
  }
}

const report = parseSubagentReport(foundationResult)

if (!report) {
  console.log(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ Stage 2 Error: Could not parse foundation-agent report
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

The foundation-agent subagent completed but returned malformed output.

Raw output:
${foundationResult}

What would you like to do?
1. Retry foundation-agent dispatch
2. Show full subagent output
3. Report bug (subagent should return valid JSON)
4. Manual intervention (I'll fix and say "resume automation")

Choose (1-4): _
  `)

  // Wait for user response, handle accordingly
  return
}
```

### 4. Validate Required Fields

**Check JSON structure:**

```typescript
function validateFoundationReport(report: any): { valid: boolean; error?: string } {
  if (!report.agent || report.agent !== "foundation-agent") {
    return { valid: false, error: "Missing or wrong 'agent' field" }
  }

  if (!report.status || !["success", "failure"].includes(report.status)) {
    return { valid: false, error: "Missing or invalid 'status' field" }
  }

  if (!report.outputs || typeof report.outputs !== 'object') {
    return { valid: false, error: "Missing 'outputs' object" }
  }

  if (!report.hasOwnProperty('ready_for_next_stage')) {
    return { valid: false, error: "Missing 'ready_for_next_stage' field" }
  }

  return { valid: true }
}

const validation = validateFoundationReport(report)
if (!validation.valid) {
  console.log(`
✗ Invalid report format: ${validation.error}

Report received:
${JSON.stringify(report, null, 2)}

What would you like to do?
1. Retry with fresh foundation-agent
2. Report bug (malformed JSON structure)
3. Manual intervention

Choose (1-3): _
  `)
  return
}
```

### 5. Handle foundation-agent Success/Failure

**If status="success" (files created):**

```typescript
if (report.status === "success" && report.ready_for_next_stage) {
  console.log(`✓ foundation-agent complete: Source files created`)
  console.log(`  - Plugin: ${report.outputs.plugin_name}`)
  console.log(`  - Files: ${report.outputs.source_files_created?.length || 0}`)

  // Now invoke build-automation to verify compilation
  console.log(`\nInvoking build-automation to verify compilation...`)

  // Proceed to section 5a (Build Verification)
}
```

**If status="failure" (file creation failed):**

Present 4-option failure menu with investigation options.

### 5a. Build Verification via build-automation

**After foundation-agent succeeds, invoke build-automation skill:**

```typescript
console.log("Invoking build-automation skill with --no-install flag...")

Skill({
  skill: "build-automation",
  context: {
    plugin_name: pluginName,
    stage: 2,
    flags: ["--no-install"],
    invoking_skill: "plugin-workflow",
    purpose: "Verify Stage 2 foundation compiles successfully"
  }
})
```

**build-automation will:**
1. Run build script with --no-install flag
2. If build succeeds: Display success message, return control to plugin-workflow
3. If build fails: Present 4-option failure protocol

**Note:** Build failures are handled entirely by build-automation skill.

### 6. Update State Files

```typescript
updateHandoff(
  pluginName,
  2,
  "Stage 2: Foundation - Build system created, compilation verified",
  ["Stage 3: Implement parameters", "Review build artifacts", "Test compilation"],
  complexityScore,
  phased
)

updatePluginStatus(pluginName, "🚧 Stage 2")
updatePluginTimeline(pluginName, 2, "Foundation complete - build system operational")
```

### 7. Git Commit

```bash
git add plugins/[PluginName]/Source/
git add plugins/[PluginName]/CMakeLists.txt
git add plugins/[PluginName]/.continue-here.md
git add PLUGINS.md

git commit -m "$(cat <<'EOF'
feat: [PluginName] Stage 2 - foundation

Build system created with JUCE 8 configuration
Minimal PluginProcessor and PluginEditor classes
Compilation verified (VST3, AU, Standalone)

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"
```

### 8. Decision Menu

```
✓ Stage 2 complete: Build system operational

Plugin: [PluginName]
Build artifacts: VST3, AU, Standalone
Status: Compiles successfully (no DSP yet)

What's next?
1. Continue to Stage 3 (implement parameters) (recommended)
2. Review build artifacts
3. Test compilation manually
4. Review Stage 2 code
5. Pause here
6. Other

Choose (1-6): _
```

**Handle responses:**
- 1 or "continue": Proceed to Stage 3
- 2: Show build artifacts list
- 3: Provide manual test instructions
- 4: Display CMakeLists.txt and source files
- 5 or "pause": Update handoff, exit
- 6 or "other": Ask "What would you like to do?" then re-present menu

---

**Return to:** Main plugin-workflow orchestration in `SKILL.md`
