## Communication Architecture

### Inter-Component Messaging

#### Dispatcher → Subagent Communication

**Pattern:** Structured prompt with complete specifications

```typescript
interface SubagentInvocation {
  task_description: string
  inputs: {
    plugin_name: string
    stage: string
    contracts: {
      "parameter-spec.md": string
      "architecture.md": string
      "plan.md": string
    }
    current_files?: {
      "PluginProcessor.cpp": string
      // ...
    }
    rules: string[]  // e.g., juce-foundation rules
  }
  expected_output_format: JSONSchema
}
```

**Example invocation:**

```
Task(
  subagent_type="dsp-agent",
  description="Implement DSP for ReverbPlugin (dsp-agent subagent)",
  prompt=`
Build the DSP implementation for ReverbPlugin.

## architecture.md DSP Components:
- juce::Reverb (core reverb processing)
- juce::dsp::LadderFilter (tone shaping)
- juce::SmoothedValue (parameter smoothing)

Processing chain: Input → Reverb → Filter → Output

## Parameters from parameter-spec.md:
- ROOM_SIZE (Float: 0-100%, default 50%)
- DAMPING (Float: 0-100%, default 50%)
- WET (Float: 0-100%, default 33%)
- TONE (Float: 20-20000 Hz, default 8000 Hz, logarithmic)

## Current PluginProcessor.cpp:
[paste code]

## JUCE Foundation Rules:
- NO allocations in processBlock()
- Use juce::ScopedNoDenormals
- Pre-allocate buffers in prepareToPlay()

## Return Format:
{
  "agent": "dsp-agent",
  "status": "success" | "failure",
  "phases_completed": ["4.1", "4.2", ...],
  "build_result": "success" | "failed",
  "test_result": "passed" | "failed" | "none",
  "safety_checks": {
    "allocations_in_processBlock": false,
    "denormal_protection": true
  },
  "issues": [],
  "ready_for_stage": 5
}
`
)
```

#### Subagent → Dispatcher Communication

**Pattern:** Structured JSON report

```json
{
  "agent": "dsp-agent",  // Subagent identifier
  "status": "success",
  "phases_completed": ["4.1", "4.2", "4.3"],
  "build_result": "success",
  "test_result": "none",
  "safety_checks": {
    "allocations_in_processBlock": false,
    "denormal_protection": true,
    "nan_checks": true
  },
  "files_modified": [
    "PluginProcessor.cpp",
    "PluginProcessor.h"
  ],
  "issues": [],
  "ready_for_stage": 5
}
```

**Dispatcher processing with robust error handling:**

```typescript
/**
 * Parse subagent report with comprehensive error handling
 * Handles: malformed JSON, extra text, markdown code blocks, partial output
 */
function parseSubagentReport(rawOutput: string): SubagentReport | ParseError {
  // 1. Try to extract JSON from markdown code blocks first
  const jsonBlockMatch = rawOutput.match(/```json\s*([\s\S]*?)\s*```/)
  let jsonText = jsonBlockMatch ? jsonBlockMatch[1] : rawOutput

  // 2. Fallback: Find first '{' and last '}' (handles extra text)
  if (!jsonBlockMatch) {
    const firstBrace = jsonText.indexOf('{')
    const lastBrace = jsonText.lastIndexOf('}')
    if (firstBrace !== -1 && lastBrace !== -1) {
      jsonText = jsonText.substring(firstBrace, lastBrace + 1)
    }
  }

  // 3. Parse with error handling
  try {
    const report = JSON.parse(jsonText)

    // 4. Validate required fields (schema validation)
    const requiredFields = ['agent', 'status']
    const missingFields = requiredFields.filter(f => !(f in report))

    if (missingFields.length > 0) {
      return {
        type: 'parse_error',
        error: `Missing required fields: ${missingFields.join(', ')}`,
        rawOutput,
        partialData: report
      }
    }

    // 5. Validate enum values
    if (!['success', 'failure', 'partial'].includes(report.status)) {
      return {
        type: 'validation_error',
        error: `Invalid status: ${report.status}`,
        rawOutput,
        partialData: report
      }
    }

    return report as SubagentReport

  } catch (e) {
    return {
      type: 'parse_error',
      error: `JSON parse failed: ${e.message}`,
      rawOutput,
      partialData: null
    }
  }
}

/**
 * Main dispatcher logic with error handling
 */
async function processStage(subagent: string, inputs: SubagentInputs) {
  let rawOutput: string

  try {
    rawOutput = await invokeSubagent(subagent, inputs, {
      timeout: 45 * 60,  // 45 minutes for complex stages
      onTimeout: () => {
        offerOptions([
          "1. **Extend timeout** (add 30 minutes)",
          "2. **Show progress** (check partial work)",
          "3. **Cancel and investigate**"
        ])
      }
    })
  } catch (e) {
    // Subagent invocation failed (Task tool error, context limit, etc.)
    return {
      status: 'error',
      error: `Subagent invocation failed: ${e.message}`,
      recovery: [
        "1. Check subagent exists in .claude/agents/",
        "2. Verify Task tool is available",
        "3. Check context limit not exceeded"
      ]
    }
  }

  // Parse report with robust handling
  const parseResult = parseSubagentReport(rawOutput)

  if ('type' in parseResult && parseResult.type === 'parse_error') {
    // Parsing failed - present error with recovery options
    return {
      status: 'error',
      error: parseResult.error,
      rawOutput: parseResult.rawOutput,
      recovery: [
        "1. **Retry subagent** (may have been interrupted)",
        "2. **Manual inspection** (check what subagent did)",
        "3. **Report bug** (capture for subagent improvement)"
      ]
    }
  }

  const report = parseResult as SubagentReport

  if (report.status === "success") {
    // Validate with validator subagent
    const validation = await validate(report, contracts)

    if (validation.status === "PASS") {
      // Commit changes
      git.commit(`feat: ${pluginName} Stage 4 - DSP implementation`)

      // Update state
      updatePluginsStatus(pluginName, "🚧 Stage 4 complete")
      updateHandoff(pluginName, nextStage=5)

      // Present to user
      showSuccessMessage(report)
      askToContinue(nextStage=5)
    } else {
      // Validation failed
      presentValidationFailure(validation)
      offerOptions([
        "1. **Fix issues** (validation found problems)",
        "2. **Re-run stage** (try implementation again)",
        "3. **Override** (log justification, continue anyway)",
        "4. **Report false positive** (improve validator)"
      ])
    }
  } else {
  // Subagent failed
  presentSubagentFailure(report)
  offerOptions(["investigate", "show-code", "retry", "manual"])
}
```

### Event System

**No explicit event bus** - communication is call-and-return via Task tool

**Why:**
- Simpler than event-driven architecture
- Clear control flow
- No hidden coupling
- Easier to debug

**Pattern:**
```
Workflow
   └─> spawns Subagent (Task tool)
       └─> Subagent executes
           └─> Subagent returns report
               └─> Workflow processes report
                   └─> Workflow spawns Validator subagent (Task tool)
                       └─> Validator subagent executes
                           └─> Validator subagent returns result
                               └─> Workflow continues or pauses
```

### Error Propagation

**Errors never auto-retry** - Always surface to user with options

**Subagent errors:**
```typescript
if (subagentReport.status === "failure") {
  presentToUser({
    message: "Subagent failed",
    agentReport: subagentReport,
    options: [
      "1. **Investigate** - Deep research via troubleshooter",
      "2. **Show code** - Display files with errors",
      "3. **Retry** - Run subagent again",
      "4. **Manual** - I'll fix it, then say 'continue'"
    ]
  })

  pauseWorkflow()
  waitForUserChoice()
}
```

**Build errors:**
```typescript
if (subagentReport.build_result === "failed") {
  const buildLog = readFile(`logs/${pluginName}/build_latest.log`)

  presentToUser({
    message: "Build failed",
    error: extractError(buildLog),
    options: [
      "1. **Investigate** - Research the error",
      "2. **Show code** - Display problematic file",
      "3. **Show output** - Full build log",
      "4. **Wait** - Fix manually"
    ]
  })

  pauseWorkflow()
  waitForUserChoice()
}
```

**Validation errors:**
```typescript
if (validatorSubagentReport.status === "FAIL") {
  presentToUser({
    message: "Validation failed",
    checks: validatorSubagentReport.checks.filter(c => !c.passed),
    recommendation: validatorSubagentReport.recommendation,
    options: [
      "1. **Fix and re-validate** - Make changes, then re-check",
      "2. **Re-run stage** - Start stage over with feedback",
      "3. **Override** - Continue despite failures (not recommended)"
    ]
  })

  pauseWorkflow()
  waitForUserChoice()
}
```

### Parallel Execution Model

**Stages run sequentially** - No parallel stage execution in v2.0

**Why:**
- Parameter consistency requires Shell (Stage 3) before DSP/GUI
- DSP must complete before GUI can test bindings
- Simpler state management
- Easier to debug

**Future consideration:**
- Stage 4 (DSP) and Stage 5 (GUI) *could* run in parallel if:
  - Stage 3 produces canonical parameter list
  - Both subagents consume same parameter contract
  - Build conflicts resolved via separate source files
- Trade-off: Speed vs. complexity
- v2.0 prioritizes simplicity

---
