## Implementation Roadmap

### Phase 0: Command Infrastructure (0.5 days)

**Goal:** Slash command routing foundation

**Tasks:**
1. Create `.claude/commands/` directory structure
2. Implement command files: implement.md, dream.md, improve.md, continue.md, test.md
3. Each command contains instructions that tell Claude which skill to invoke
4. Test autocomplete discovery (type "/" shows all commands)

**Success:** `/implement`, `/dream`, `/improve` route to correct skills

### Phase 1: Core Sub-Agent Architecture (2-3 days)

**Goal:** Single-session plugin creation

**Tasks:**
1. Create 4 agent definitions (foundation, shell, dsp, gui)
2. Modify plugin-workflow to dispatch Stages 2-5
3. Implement agent report parsing
4. Add interactive decision menus at stage boundaries (AskUserQuestion tool)
5. Test with simple plugin (complexity 1-2)

**Success:** Plugin created in single session without manual `/continue`, with contextual options at each checkpoint

### Phase 2: Validation System (1 day)

**Goal:** Independent verification of stage claims

**Tasks:**
1. Create validator agent
2. Add 7 validation points (after each stage)
3. Implement PASS/FAIL reporting
4. Test with intentional failures

**Success:** Validator catches drift, blocks on failures

### Phase 3: Quality Hooks (1 day)

**Goal:** Automatic quality enforcement

**Tasks:**
1. Create juce-validator.py hook
2. Add PostToolUse hook configuration
3. Test with real-time violations
4. Test with parameter mismatches

**Success:** Hooks catch violations before build

### Phase 4: Contract System (0.5 days)

**Goal:** Enforce parameter/architecture contracts

**Tasks:**
1. Modify Stage 1 to check prerequisites
2. Add contract validation to plugin-workflow skill
3. Update ui-mockup for two-phase design
4. Test blocking when contracts missing

**Success:** Stage 1 blocks if no parameter-spec.md or architecture.md

### Phase 5: Interactive Decision System (1 day)

**Goal:** Context-aware option menus at all checkpoints

**Tasks:**
1. Implement option generation logic (plugin state → relevant choices)
2. Add decision menus to: UI mockup workflow, error scenarios, investigation results
3. Create "Pause here" option templates for all workflows
4. Test progressive disclosure (features discovered through options)
5. Test checkpointing exploration (Esc+Esc to try different paths)

**Success:** Every checkpoint presents contextual numbered options, features discoverable without docs

### Phase 6: Testing & Refinement (1-2 days)

**Goal:** Validate complete system

**Tasks:**
1. Create simple plugin (complexity 1-2) using slash commands
2. Create complex plugin (complexity 4-5) exploring different options
3. Test error paths (build failures, validation failures, option menus)
4. Test parameter contract enforcement
5. Verify discovery mechanisms (autocomplete + contextual menus)
6. Refine based on findings

**Success:** Both simple and complex plugins complete successfully, features discovered organically

**Total estimated effort:** 7-9 days

---

## Summary

TÂCHES v2.0 is a **discoverable, slash-command-driven system** for conversational JUCE plugin development.

**Core innovations:**
1. **Slash command entry** - Autocomplete discovery, deterministic routing
2. **Interactive decision menus** - Contextual options at every checkpoint
3. **Dispatcher pattern** - Fresh context per stage eliminates accumulation
4. **Contract prerequisites** - parameter-spec + architecture prevent drift
5. **Independent validation** - Verifier catches stage completion claims
6. **Quality hooks** - Deterministic enforcement before builds
7. **Two-phase UI** - Fast iteration separated from finalization

**Design philosophy:**
- **Progressive disclosure** - Features discovered through use, not documentation
- **Risk-free exploration** - Claude Code checkpointing enables trying options safely
- **User control** - Every checkpoint offers choices, not just yes/no
- **Fun and personal** - System teaches itself, feels playful

**Architectural principles:**
- **Minimal coupling** - Clean interfaces, no circular dependencies
- **Single responsibility** - Each component has one clear job
- **Deterministic routing** - Slash commands map 1:1 to skills
- **Fail-fast validation** - Catch issues at earliest possible point
- **Explicit state** - One state machine, clear ownership

**Result:** A cohesive, discoverable, extensible system that enables professional plugin development through slash commands and contextual menus—in a single session, with automatic quality assurance, zero drift between design and implementation, and a minimal learning curve.

---

## Related Procedures

This roadmap is implemented through:

- `procedures/skills/plugin-workflow.md` - Phase 1-2: Core workflow and validation system
- `procedures/core/interactive-decision-system.md` - Phase 5: Interactive decision system implementation
- `procedures/commands/` - Phase 0: All command implementations (implement.md, dream.md, improve.md, continue.md)
- `procedures/agents/` - Phase 1: Subagent definitions
