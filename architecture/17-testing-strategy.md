## Testing Strategy

### Testing Philosophy

**Goal:** Ensure v2 eliminates v1 pain points while maintaining quality and reliability

**Approach:** 3-tier testing pyramid
- **Unit tests** - Component isolation, edge cases
- **Integration tests** - Component interactions, workflows
- **System tests** - End-to-end plugin creation

### Unit Tests

**Scope:** Individual components in isolation

#### 1. Agent Report Parsing

**Test:** `parseAgentReport()` function handles edge cases

```typescript
describe('parseAgentReport', () => {
  test('valid JSON in markdown block', () => {
    const input = '```json\n{"agent": "dsp", "status": "success"}\n```'
    expect(parseAgentReport(input).status).toBe('success')
  })

  test('malformed JSON - missing brace', () => {
    const input = '{"agent": "dsp", "status": "success"'
    const result = parseAgentReport(input)
    expect(result.type).toBe('parse_error')
  })

  test('extra text before/after JSON', () => {
    const input = 'Here is my report:\n{"agent": "dsp", "status": "success"}\nDone!'
    expect(parseAgentReport(input).status).toBe('success')
  })

  test('missing required fields', () => {
    const input = '{"status": "success"}'  // Missing "agent"
    const result = parseAgentReport(input)
    expect(result.type).toBe('parse_error')
    expect(result.error).toContain('Missing required fields')
  })

  test('invalid enum value', () => {
    const input = '{"agent": "dsp", "status": "invalid"}'
    const result = parseAgentReport(input)
    expect(result.type).toBe('validation_error')
  })
})
```

#### 2. Validator Logic

**Test:** Validator correctly identifies contract violations

```typescript
describe('Validator', () => {
  test('passes valid implementation', async () => {
    // Setup: Create valid parameter-spec.md and PluginProcessor.cpp
    const validation = await validateStage3({
      spec: validParameterSpec,
      code: validPluginProcessor
    })
    expect(validation.status).toBe('PASS')
  })

  test('fails on parameter ID mismatch', async () => {
    // Spec defines "GAIN", code uses "gain"
    const validation = await validateStage3({
      spec: { params: [{id: "GAIN"}] },
      code: 'state.getParameter("gain")'
    })
    expect(validation.status).toBe('FAIL')
    expect(validation.issues).toContain('Parameter ID mismatch')
  })

  test('fails on missing JUCE best practice', async () => {
    // Code missing juce::ScopedNoDenormals
    const validation = await validateStage4({
      code: processBlockWithoutDenormalProtection
    })
    expect(validation.status).toBe('FAIL')
    expect(validation.issues).toContain('Missing denormal protection')
  })
})
```

#### 3. Hook Script Exit Codes

**Test:** Hooks block correctly on validation failures

```bash
#!/bin/bash
# test-juce-validator.sh

# Test 1: Valid code passes
echo "TEST: Valid JUCE code"
python .claude/hooks/scripts/juce-validator.py --code "juce::ScopedNoDenormals noDenormals;"
if [ $? -eq 0 ]; then echo "✓ PASS"; else echo "✗ FAIL"; exit 1; fi

# Test 2: Invalid code fails
echo "TEST: Code with allocation in processBlock"
python .claude/hooks/scripts/juce-validator.py --code "auto* buffer = new float[512];"
if [ $? -ne 0 ]; then echo "✓ PASS"; else echo "✗ FAIL"; exit 1; fi

# Test 3: Timeout handling
echo "TEST: Hook timeout"
timeout 5 python .claude/hooks/scripts/slow-hook.py
if [ $? -eq 124 ]; then echo "✓ PASS (timeout)"; else echo "✗ FAIL"; exit 1; fi
```

#### 4. State Machine Transitions

**Test:** PLUGINS.md state transitions follow rules

```typescript
describe('Plugin Lifecycle State Machine', () => {
  test('valid transition: 💡 → 🚧', () => {
    expect(canTransition('💡 Ideated', '🚧 Stage 0')).toBe(true)
  })

  test('invalid transition: 💡 → 📦', () => {
    // Cannot skip directly to installed
    expect(canTransition('💡 Ideated', '📦 Installed')).toBe(false)
  })

  test('valid transition: 🚧 Stage 6 → ✅', () => {
    expect(canTransition('🚧 Stage 6 complete', '✅ Working')).toBe(true)
  })

  test('cannot modify 🚧 with plugin-improve', () => {
    const plugin = { status: '🚧 Stage 4.2' }
    expect(() => pluginImprove(plugin)).toThrow('Plugin in development')
  })
})
```

### Integration Tests

**Scope:** Component interactions and workflow coordination

#### 1. Workflow Coordination

**Test:** Skills correctly coordinate and validate stages

```typescript
describe('plugin-workflow integration', () => {
  test('Stage 3 → Stage 4 transition', async () => {
    // Setup: Plugin at Stage 3 complete
    setupPlugin('TestPlugin', { status: '🚧 Stage 3 complete' })

    // Execute: Run Stage 4
    const result = await runWorkflowStage('TestPlugin', 4)

    // Verify:
    expect(result.agent).toBe('dsp-agent')
    expect(result.validation.status).toBe('PASS')
    expect(getPluginStatus('TestPlugin')).toContain('Stage 4')
    expect(gitLog()).toContain('Stage 4 complete')
  })

  test('Stage fails validation → offers recovery options', async () => {
    // Setup: Validator will fail
    mockValidator({ status: 'FAIL', issues: ['Missing component X'] })

    const result = await runWorkflowStage('TestPlugin', 4)

    expect(result.validationFailed).toBe(true)
    expect(result.options).toContain('Fix issues')
    expect(result.options).toContain('Re-run stage')
    expect(gitLog()).not.toContain('Stage 4 complete')  // No commit
  })
})
```

#### 2. Error Recovery

**Test:** System recovers from common failure modes

```typescript
describe('Error Recovery', () => {
  test('build failure → parse log → suggest fix', async () => {
    // Inject build failure
    mockBuildFailure({ error: 'juce_dsp/juce_dsp.h not found' })

    const result = await runWorkflowStage('TestPlugin', 4)

    expect(result.status).toBe('error')
    expect(result.recovery).toContain('Add juce::juce_dsp to CMakeLists.txt')
    expect(result.logPath).toMatch(/build_\d+\.log/)
  })

  test('agent timeout → show progress', async () => {
    mockAgentTimeout({ partialWork: ['Phase 4.1 done', 'Phase 4.2 started'] })

    const result = await runWorkflowStage('TestPlugin', 4)

    expect(result.status).toBe('timeout')
    expect(result.options).toContain('Show progress')
    expect(result.partialWork).toBeDefined()
  })
})
```

#### 3. Session Continuity

**Test:** `.continue-here.md` enables proper resumption

```typescript
describe('Session Continuity', () => {
  test('pause → resume at correct stage', async () => {
    // Stage 4.2 complete, user pauses
    await completeStagePhase('TestPlugin', 4, 2)
    simulateSessionEnd()

    // New session
    simulateSessionStart()
    const resumePoint = await readContinueFile('TestPlugin')

    expect(resumePoint.stage).toBe('4.3')
    expect(resumePoint.completed).toContain('4.1', '4.2')
    expect(resumePoint.nextSteps[0]).toContain('Continue to Stage 4.3')
  })

  test('build fails → handoff includes error', async () => {
    mockBuildFailure({ error: 'Syntax error PluginProcessor.cpp:78' })
    await runWorkflowStage('TestPlugin', 4)

    const resumePoint = await readContinueFile('TestPlugin')

    expect(resumePoint.knownIssues).toBeDefined()
    expect(resumePoint.knownIssues[0]).toContain('PluginProcessor.cpp:78')
    expect(resumePoint.suggestedNextSteps).toContain('Investigate error')
  })
})
```

### System Tests

**Scope:** End-to-end plugin creation across complexity levels

#### 1. Simple Plugin (Complexity 1-2)

**Test:** Create gain plugin from ideation to installation

```typescript
describe('E2E: Simple Gain Plugin', () => {
  test('complete workflow in single session', async () => {
    // Stage 0: Ideation
    await runCommand('/dream GainPlugin - simple volume control')
    expect(fileExists('.ideas/creative-brief.md')).toBe(true)

    // Finalize mockup and parameter spec
    await finalizeUI('GainPlugin')
    expect(fileExists('.ideas/parameter-spec.md')).toBe(true)

    // Stage 0-6: Implementation
    const result = await runCommand('/implement GainPlugin')

    // Verify complete plugin
    expect(result.status).toBe('✅ Working')
    expect(fileExists('Source/PluginProcessor.cpp')).toBe(true)
    expect(fileExists('ui/public/index.html')).toBe(true)

    // Verify validation passed
    const validationLog = readLog('pluginval')
    expect(validationLog).toContain('RESULT: PASS')

    // Install
    await runCommand('/install-plugin GainPlugin')
    expect(pluginInstalled('~/Library/Audio/Plug-Ins/VST3/Gain.vst3')).toBe(true)

    // Verify total time
    expect(getTotalTime()).toBeLessThan(40 * 60)  // < 40 minutes
  }, 40 * 60 * 1000)  // 40 minute timeout
})
```

#### 2. Medium Plugin (Complexity 3)

**Test:** Create reverb with multiple DSP components

```typescript
describe('E2E: Reverb Plugin (Complexity 3)', () => {
  test('complete workflow with DSP phases', async () => {
    await runCommand('/dream ReverbPlugin - lush algorithmic reverb')
    await finalizeUI('ReverbPlugin')

    const result = await runCommand('/implement ReverbPlugin')

    // Verify DSP architecture
    const architecture = readFile('.ideas/architecture.md')
    expect(architecture).toContain('juce::Reverb')
    expect(architecture).toContain('LadderFilter')

    // Verify parameters from spec
    const processor = readFile('Source/PluginProcessor.cpp')
    expect(processor).toContain('ROOM_SIZE')
    expect(processor).toContain('DAMPING')
    expect(processor).toContain('WET')

    // Verify UI bindings
    const ui = readFile('ui/public/index.html')
    expect(ui).toContain('getSliderState("ROOM_SIZE")')

    expect(result.status).toBe('✅ Working')
    expect(getTotalTime()).toBeLessThan(82 * 60)  // < 82 minutes
  }, 90 * 60 * 1000)
})
```

#### 3. Complex Plugin (Complexity 5)

**Test:** Wavetable synth with extended thinking

```typescript
describe('E2E: Wavetable Synth (Complexity 5)', () => {
  test('complete workflow with Opus + extended thinking', async () => {
    await runCommand('/dream WavetableSynth - modern wavetable synthesizer')
    await finalizeUI('WavetableSynth')

    // Verify extended thinking enabled
    const stageConfig = getAgentConfig('dsp-agent')
    expect(stageConfig.model).toBe('opus')
    expect(stageConfig.extended_thinking).toBe(true)

    const result = await runCommand('/implement WavetableSynth')

    // Verify complex DSP
    const processor = readFile('Source/PluginProcessor.cpp')
    expect(processor).toContain('wavetable')
    expect(processor).toContain('interpolation')
    expect(processor).toContain('anti-aliasing')

    // Verify no allocations in processBlock
    const validation = await validateJUCEBestPractices(processor)
    expect(validation.allocations_in_processBlock).toBe(false)
    expect(validation.denormal_protection).toBe(true)

    expect(result.status).toBe('✅ Working')
    expect(getTotalTime()).toBeLessThan(150 * 60)  // < 150 minutes
  }, 160 * 60 * 1000)
})
```

#### 4. Plugin Improvement

**Test:** Versioned enhancement with backup

```typescript
describe('E2E: Plugin Improvement', () => {
  test('add feature to existing plugin', async () => {
    // Setup: Install plugin v1.0.0
    await createAndInstallPlugin('DelayPlugin', '1.0.0')

    // Improve: Add tempo sync
    const result = await runCommand('/improve DelayPlugin - add tempo sync')

    // Verify backup created
    expect(fileExists('backups/DelayPlugin/v1.0.0/')).toBe(true)

    // Verify version bump
    const changelog = readFile('CHANGELOG.md')
    expect(changelog).toContain('v1.1.0')
    expect(changelog).toContain('tempo sync')

    // Verify plugin still works
    expect(result.validation.status).toBe('PASS')
    expect(result.version).toBe('1.1.0')

    // Verify can rollback
    await rollbackPlugin('DelayPlugin', '1.0.0')
    expect(getCurrentVersion('DelayPlugin')).toBe('1.0.0')
  })
})
```

### Acceptance Criteria

**Plugin Creation Success Rates:**
- Complexity 1-2: ≥ 95% first-attempt success
- Complexity 3: ≥ 85% first-attempt success
- Complexity 4-5: ≥ 70% first-attempt success

**Quality Gates:**
- 100% of validators pass on valid code
- 100% of validators fail on contract violations
- 0% false positives on validation (with override tracking)
- 100% of builds succeed after Stage 2 verification
- 100% of plugins pass pluginval at Stage 6

**Performance:**
- Simple plugin (1-2): < 40 minutes
- Medium plugin (3): < 85 minutes
- Complex plugin (4-5): < 150 minutes
- Validator overhead: < 2 minutes per stage

**Regression:**
- v1 plugins can be migrated to v2 structure
- No data loss during migration
- All v1 features available in v2

### Testing Workflow

**Phase 1: Unit Tests** (Day 1-2 of implementation)
```bash
# Run unit tests
npm test -- unit/

# Coverage
npm run coverage
# Target: >80% coverage for core components
```

**Phase 2: Integration Tests** (Day 3-4)
```bash
# Run integration tests
npm test -- integration/

# Test specific workflow
npm test -- integration/plugin-workflow.test.ts
```

**Phase 3: System Tests** (Day 5)
```bash
# Run E2E tests (slow)
npm test -- e2e/

# Test specific complexity
npm test -- e2e/simple-plugin.test.ts
```

**Phase 4: Regression Tests** (Day 6)
```bash
# Verify v1 compatibility
npm test -- regression/

# Migrate v1 plugin
./scripts/migrate-to-v2.sh TestPluginV1
npm test -- e2e/migrated-plugin.test.ts
```

**Continuous Testing:**
```bash
# Watch mode during development
npm run test:watch

# Quick smoke test
npm run test:quick  # Unit + critical integration only
```

### Success Metrics Dashboard

Track during testing and after deployment:

```markdown
## Test Results Summary

### Success Rates
- Simple plugins (1-2): 12/12 (100%) ✅
- Medium plugins (3): 8/10 (80%) ⚠️ [Below 85% target]
- Complex plugins (4-5): 5/8 (62.5%) ⚠️ [Below 70% target]

### Quality Gates
- Validator accuracy: 98% (2% false positives logged)
- Build success rate: 100%
- Pluginval pass rate: 95% (1 failure - denormal issue)

### Performance
- Simple: avg 35 min (budget: 40 min) ✅
- Medium: avg 78 min (budget: 82 min) ✅
- Complex: avg 145 min (budget: 143 min) ⚠️ [Slightly over]

### Issues Found
1. Validator false positive on custom denormal protection
2. Complex plugin timeout on Stage 4 (wavetable)
3. Tempo sync parameter binding issue

### Action Items
- [ ] Calibrate validator for custom denormal patterns
- [ ] Increase timeout for Stage 4 complexity 5 (45 → 60 min)
- [ ] Fix tempo sync parameter relay implementation
```

---

## Related Procedures

This testing strategy is implemented through:

- `procedures/skills/plugin-testing.md` - Automated testing skill implementation
- `procedures/skills/plugin-workflow.md` - Integration and system test specifications

---

**End of Architecture Document**
