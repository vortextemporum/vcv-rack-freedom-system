# Phase 2 Implementation Plan: Workflow Engine

## Prerequisites Status

**Previous Phase(s) Required:** Phase 0, Phase 1

**Deliverables Check:**
- ✅ `.claude/commands/` - 9 slash commands exist (dream.md, implement.md, improve.md, continue.md, test.md, install-plugin.md, show-standalone.md, troubleshoot-juce.md, doc-fix.md)
- ✅ `.claude/skills/` - 7 skills with modular structure (SKILL.md + references/ + assets/ subdirectories)
  - plugin-workflow, plugin-ideation, plugin-improve, context-resume, ui-mockup (two-phase workflow), plugin-testing, plugin-lifecycle
- ✅ Skill templates - Contract templates migrated to skill-specific assets/ directories:
  - `.claude/skills/ui-mockup/assets/parameter-spec-template.md`
  - `.claude/skills/plugin-workflow/assets/architecture-template.md`
  - `.claude/skills/plugin-workflow/assets/plan-template.md`
  - `.claude/skills/plugin-ideation/assets/creative-brief-template.md`
- ✅ `PLUGINS.md` - Registry file exists with status legend
- ✅ `.claude/CLAUDE.md` - Navigation index exists
- ✅ `plugins/`, `logs/`, `backups/`, `scripts/` - Core directories exist

**Status:** Complete

---

## Phase Overview

Phase 2 implements the core workflow engine by building out plugin-workflow skill to handle Stages 0, 1, and 6 (main context stages) with full checkpoint system, state management, validator integration, and hook-based validation. This establishes the complete workflow orchestration pattern before adding implementation subagents in Phase 3.

**Why this phase is critical:**
- Establishes checkpoint-based workflow architecture enabling session continuity
- Implements contract enforcement system (prevents "code first, design later")
- Creates complete state management infrastructure (PLUGINS.md, handoff files, git workflow)
- Integrates validator subagent for quality gates at Stages 0, 1, 6
- Implements hook system for context injection and stage enforcement
- Enables end-to-end workflow testing before code generation capability
- Users experience complete workflow structure with decision-driven navigation

---

## Sub-Phase Breakdown

This phase splits into 3 sub-phases based on architectural dependencies:

**Phase 2a: Checkpoint & State Infrastructure**
- Checkpoint system (.continue-here.md format and hard checkpoints)
- PLUGINS.md state machine (💡 → 🚧 → ✅ → 📦)
- Git commit workflow (atomic state transitions)
- context-resume skill integration
- Interactive decision menu system

**Phase 2b: Workflow Stages 0 & 1**
- Stage 0: Research (Context7 JUCE docs, professional research, feasibility)
- Stage 1: Planning (complexity scoring, contract enforcement, phase breakdown)
- Contract prerequisite enforcement (BLOCKS if parameter-spec.md or architecture.md missing)
- Stage dispatch logic
- Validator integration for Stages 0 & 1

**Phase 2c: Stage 6, Validator & Hooks**
- Stage 6: Validation & Presets (pluginval, CHANGELOG, factory presets)
- Validator subagent implementation
- Hook system (UserPromptSubmit, Stop, PreCompact)
- Stages 2-5 stub implementations
- End-to-end workflow integration

---

## Required Documentation

### Architecture Files (MUST READ IN ENTIRETY)

**CRITICAL (Phase 2 directly implements these):**
- `architecture/00-PHILOSOPHY.md` (lines 108-275) - Interactive decision system philosophy
- `architecture/02-core-abstractions.md` (all) - Workflows, Contracts, Hybrid Validation (lines 48-252)
- `architecture/04-component-architecture.md` (lines 115-487) - Skill interface, Validator interface, Hybrid validation flow
- `architecture/05-routing-architecture.md` (lines 160-614) - Interactive decision system, precondition verification, error handling
- `architecture/06-state-architecture.md` (all) - PLUGINS.md state machine, handoff files, CHANGELOG, logs
- `architecture/10-extension-architecture.md` (lines 96-165) - Hook system architecture
- `architecture/13-error-handling-recovery.md` (lines 180-683) - Hook-based validation, 6 hook specs, performance budgets
- `architecture/16-implementation-roadmap.md` (lines 27-90) - Phase 2 specification

**HIGH (Essential Phase 2 context):**
- `architecture/01-executive-summary.md` (lines 22-54) - Core decisions and related procedures
- `architecture/03-model-selection-extended-thinking-strategy.md` (lines 50-162) - Stage 0/1 extended thinking
- `architecture/09-file-system-design.md` (lines 64-183) - File system structure, version control
- `architecture/14-design-decisions.md` (lines 26-237) - Design rationale for contracts, validation, menus, state

**MEDIUM (Phase 2 reference):**
- `architecture/07-communication-architecture.md` (lines 1-242) - Subagent dispatch patterns (for Phase 3 prep)
- `architecture/08-data-flow-diagrams.md` (lines 104-209) - State transitions and critical paths
- `architecture/11-build-automation-architecture.md` (lines 1-83) - Build script spec for Stage 6
- `architecture/17-testing-strategy.md` (lines 58-236) - Validator logic, state machine, session continuity tests

### Procedure Files (MUST READ IN ENTIRETY)

**CRITICAL (Phase 2 directly implements these):**
- `procedures/core/checkpoint-system.md` (all 440 lines) - Checkpoint architecture, handoff format, resume workflows
- `procedures/core/interactive-decision-system.md` (all 521 lines) - Decision menu format, context-aware generation, patterns
- `procedures/skills/plugin-workflow.md` (all) - Complete 7-stage workflow spec, stage boundary protocol, complexity adaptation
- `procedures/skills/context-resume.md` (lines 23-327) - Handoff file parsing, context loading, resumption logic
- `procedures/commands/implement.md` (all 243 lines) - Entry point, preconditions, 7-stage overview, complexity intelligence
- `procedures/commands/continue.md` (all 250 lines) - Handoff loading, context recovery, routing to context-resume
- `procedures/commands/dream.md` (lines 18-76) - Discovery and brainstorming menu options
- `procedures/commands/test.md` (lines 23-242) - Auto-invocation after Stage 4/5, testing methods

**HIGH (Essential Phase 2 integration):**
- `procedures/skills/deep-research.md` (lines 59-115) - Stage 0 research protocol, graduated depth
- `procedures/skills/build-automation.md` (all) - Build process, failure protocol (4-option menu)
- `procedures/skills/plugin-testing.md` (lines 23-134) - Auto-invocation specs, test suite
- `procedures/skills/plugin-lifecycle.md` (lines 23-113) - Stage 6 installation process
- `procedures/skills/juce-foundation.md` (lines 20-115) - API verification, real-time safety (reference)
- `procedures/skills/design-sync.md` (lines 36-90) - Stage 0 brief ↔ mockup sync (optional)
- `procedures/commands/doc-fix.md` (lines 15-66) - Error documentation during development
- `procedures/commands/install-plugin.md` (lines 29-88) - Post-Stage-6 installation
- `procedures/commands/show-standalone.md` (lines 78-88) - Stage 5 UI inspection

**MEDIUM (Context/reference):**
- `procedures/skills/plugin-ideation.md` (lines 25-147) - Pre-research brainstorming, creative brief
- `procedures/skills/ui-mockup.md` (lines 79-120) - Stage 0 UI requirements reference
- `procedures/skills/troubleshooting-docs.md` (lines 29-90) - Problem documentation format
- `procedures/agents/troubleshooter.md` (lines 31-208) - Research agent protocol
- `procedures/commands/improve.md` (lines 28-144) - Post-Phase-2 context
- `procedures/scripts/build-and-install.md` (all) - Centralized build script

---

## Implementation Sequence

### Phase 2a: Checkpoint & State Infrastructure

#### Task 1: Implement Checkpoint System Foundation

**Description**: Create the checkpoint architecture that enables session continuity through handoff files.

**Required Reading:**
- `procedures/core/checkpoint-system.md` (all 440 lines) - Complete checkpoint specification
- `architecture/06-state-architecture.md` (lines 156-315) - Session state and handoff files
- `architecture/09-file-system-design.md` (lines 95-98) - Handoff file location

**Dependencies**: None (foundational task)

**Implementation Steps:**
1. Create handoff file template in `.claude/skills/plugin-workflow/assets/continue-here-template.md` with exact format from checkpoint-system.md lines 110-185:
   ```yaml
   ---
   plugin: PluginName
   stage: N
   phase: M (optional)
   status: in_progress | complete
   last_updated: YYYY-MM-DD HH:MM:SS
   complexity_score: X.Y
   phased_implementation: true | false
   ---

   # Resume Point
   ## Current State: Stage N(.M) - Description
   ## Completed So Far: [Details with commit hashes]
   ## Next Steps: [Numbered list]
   ## Context to Preserve: [Key decisions, files, status]
   ## How to Resume: [Instructions]
   ```
2. Implement handoff functions in plugin-workflow skill:
   - `createHandoff(pluginName, stage, context)` - Creates after Stage 0
   - `updateHandoff(pluginName, stage, completed, nextSteps, complexityScore, phased)` - Updates after each stage
   - `deleteHandoff(pluginName)` - Removes when reaching ✅ Working or 📦 Installed
3. Define checkpoint types (from checkpoint-system.md lines 38-90):
   - **Hard checkpoints:** Stage 0, Stage 1, Stage 6 (MUST pause for user decision)
   - **Soft checkpoints:** Stage 2-5 (can auto-continue if user preference set)
   - **Decision checkpoints:** Build failures, validation failures (4-option menus)
4. Integrate with context-resume skill:
   - Verify handoff format matches what context-resume expects (context-resume.md lines 44-185)
   - Test YAML parsing with frontmatter validation
   - Test markdown section parsing (Resume Point, Completed, Next Steps, etc.)

**Expected Output:**
- `.claude/skills/plugin-workflow/assets/continue-here-template.md` created with exact spec format
- Handoff functions implemented in plugin-workflow skill
- Checkpoint type classification logic
- Integration verified with context-resume

**Verification:**
- Automated: `scripts/test-handoff-format.sh` - Validates YAML frontmatter and markdown structure
- Manual: STOP AND ASK LEX: "Please verify the handoff template matches the exact format in checkpoint-system.md lines 110-185, including all required YAML fields (plugin, stage, phase, status, last_updated, complexity_score, phased_implementation) and markdown sections"

---

#### Task 2: Implement PLUGINS.md State Machine

**Description**: Create state machine that tracks all plugins through lifecycle stages.

**Required Reading:**
- `architecture/06-state-architecture.md` (lines 7-154) - Complete state machine spec
- `architecture/08-data-flow-diagrams.md` (lines 104-186) - State transition diagram
- `architecture/14-design-decisions.md` (lines 214-237) - State model rationale

**Dependencies**: None (parallel with Task 1)

**Implementation Steps:**
1. Define state machine transitions in plugin-workflow skill:
   ```
   💡 Ideated (creative-brief.md exists, no Source/)
     ↓ /implement Stage 0 start
   🚧 Stage 0 (research in progress)
     ↓ Stage 0 complete
   🚧 Stage 1 (planning in progress)
     ↓ Stage 1 complete
   🚧 Stage 2-5 (implementation in progress)
     ↓ Stage 6 start
   🚧 Stage 6 (validation in progress)
     ↓ Stage 6 complete
   ✅ Working (validation passed, not installed)
     ↓ /install-plugin
   📦 Installed (in system folders)
   ```
2. Implement state update functions:
   - `updatePluginStatus(pluginName, newStatus)` - Updates emoji in PLUGINS.md
   - `createPluginEntry(pluginName, type, brief)` - Creates initial 💡 entry
   - `updatePluginTimeline(pluginName, stage, description)` - Adds timeline entry
   - `getPluginStatus(pluginName)` - Returns current status for routing
   - `validateTransition(currentStatus, newStatus)` - Enforces legal transitions
3. Implement state machine constraints:
   - **Single 🚧 constraint:** Only ONE plugin can be 🚧 at a time (prevents concurrent modification)
   - **Sequential stage enforcement:** Cannot skip from 🚧 Stage 0 to 🚧 Stage 6
   - **Contract-based transitions:** Stage 1 transition requires parameter-spec.md and architecture.md (generated via ui-mockup two-phase workflow)
4. Create PLUGINS.md entry structure (from state-architecture.md lines 42-83):
   ```markdown
   ### [PluginName]
   **Status:** [emoji] [Status Text]
   **Created:** YYYY-MM-DD
   **Version:** X.Y.Z (after Stage 6)

   **Type:** [Audio Effect | MIDI Instrument | Synth]
   - [Brief description from creative-brief.md]

   **Lifecycle Timeline:**
   - **YYYY-MM-DD (Stage 0):** Research completed
   - **YYYY-MM-DD (Stage 1):** Planning - Complexity N
   - ...

   **Last Updated:** YYYY-MM-DD
   ```

**Expected Output:**
- State machine functions implemented
- Legal transition validation enforced
- Single 🚧 constraint enforced
- PLUGINS.md entry format standardized

**Verification:**
- Automated: `scripts/test-state-machine.sh` - Tests legal/illegal transitions, single 🚧 constraint
- Manual: STOP AND ASK LEX: "Please verify the state machine prevents: (1) skipping from 💡 directly to ✅, (2) starting a second plugin when one is already 🚧, and (3) transitioning to Stage 1 without parameter-spec.md and architecture.md (which come from ui-mockup two-phase workflow)"

---

#### Task 3: Implement Git Commit Workflow

**Description**: Create standardized git commit integration with atomic state transitions.

**Required Reading:**
- `procedures/core/checkpoint-system.md` (lines 328-385) - Git commit integration
- `architecture/06-state-architecture.md` (lines 456-470) - Atomic state transitions, consistency guarantees
- `architecture/09-file-system-design.md` (lines 145-183) - Version control strategy

**Dependencies**: Task 1 (commits include handoff updates), Task 2 (commits include PLUGINS.md updates)

**Implementation Steps:**
1. Create commit helper function in plugin-workflow:
   ```typescript
   function commitStage(pluginName, stage, description) {
     // Standardized message format
     const message = `feat: ${pluginName} Stage ${stage} - ${description}

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>`

     // Stage changes atomically
     git.add([
       `plugins/${pluginName}/Source/`,  // If exists
       `plugins/${pluginName}/.ideas/`,  // Contracts
       `plugins/${pluginName}/.continue-here.md`,
       `PLUGINS.md`
     ])

     git.commit(message)

     // Verify commit succeeded
     const hash = git.log('-1', '--format=%h')
     return { success: true, hash, message }
   }
   ```
2. Implement atomic commit protocol:
   - Stage completes → Update PLUGINS.md → Update handoff → Single git commit
   - If commit fails → Rollback PLUGINS.md and handoff changes
   - Display commit hash to user for reference
3. Add commit message variations:
   - Simple stages: `feat: [Plugin] Stage N - description`
   - Phased stages: `feat: [Plugin] Stage 4.1 - core processing`
   - Stage 6: `feat: [Plugin] Stage 6 - validation complete`
4. Implement commit verification:
   - Check git availability before workflow starts
   - Verify commit succeeded with `git log -1`
   - Handle commit failures gracefully (warn user, suggest manual commit)

**Expected Output:**
- Git commits created after each stage
- Standardized commit message format
- Atomic state + code commits
- Commit hash displayed to user

**Verification:**
- Automated: `git log --grep="feat:.*Stage" --format="%s" | grep -E "feat: \w+ Stage [0-6]"` - Validates commit format
- Manual: STOP AND ASK LEX: "Please verify that after completing Stage 0, exactly one git commit is created with format 'feat: [Plugin] Stage 0 - research complete' and it includes changes to .ideas/research.md, PLUGINS.md, and .continue-here.md in a single atomic commit"

---

#### Task 4: Implement Interactive Decision Menu System

**Description**: Create context-aware decision menu system that appears at all checkpoints.

**Required Reading:**
- `procedures/core/interactive-decision-system.md` (all 521 lines) - Complete decision menu specification
- `architecture/05-routing-architecture.md` (lines 160-435) - Interactive decision system architecture
- `architecture/00-PHILOSOPHY.md` (lines 108-275) - Discovery-through-play philosophy

**Dependencies**: Task 1 (menus appear at checkpoints)

**Implementation Steps:**
1. Create decision menu generator function in plugin-workflow:
   ```typescript
   function presentDecisionMenu(context: MenuContext) {
     // Context includes: stage, status, errors, options
     const options = generateContextualOptions(context)

     // Format: inline numbered list (NOT AskUserQuestion)
     const menu = `
✓ ${context.completionStatement}

What's next?
${options.map((opt, i) => `${i+1}. ${opt.label}${opt.recommended ? ' (recommended)' : ''}${opt.discovery ? ' ← ' + opt.discovery : ''}`).join('\n')}

Choose (1-${options.length}): _`

     return displayAndAwaitChoice(menu, options)
   }
   ```
2. Implement context-aware option generation (interactive-decision-system.md lines 82-221):
   - **After Stage 0:** Continue to Stage 1 / Review research / Improve brief / Pause / Other
   - **After Stage 1:** Continue to Stage 2 / Review plan / Adjust complexity / Pause / Other
   - **After Stage 6:** Install plugin / Test in DAW / Create another plugin / Review code / Other
   - **Build failure:** Investigate / Show code / Show output / Manual fix / Other
   - **Validation failure:** Fix and re-validate / Re-run stage / Override / Other
3. Implement inline numbered list format (NOT AskUserQuestion):
   - No 4-option limit (can show unlimited options)
   - Dynamic lists for plugins, templates, etc.
   - Discovery markers: `← User discovers [feature]`
   - Recommendation markers: `(recommended)`
4. Implement response handling:
   - Accept number input (1-N)
   - Accept keyword shortcuts ("continue", "pause", "review")
   - Accept "Other" for free-form requests
   - Re-present menu after handling "Other"
5. Implement progressive disclosure (interactive-decision-system.md lines 170-238):
   - Surface hidden features contextually
   - Use discovery comments to teach system capabilities
   - Enable risk-free exploration via Claude Code checkpointing

**Expected Output:**
- Decision menus appear at all checkpoints
- Inline numbered lists (not AskUserQuestion)
- Context-aware option generation
- Keyword shortcuts functional
- Progressive disclosure working

**Verification:**
- Automated: `scripts/test-decision-menus.sh` - Validates menu format and option generation
- Manual: STOP AND ASK LEX: "Please verify that after Stage 0 completes, a decision menu with 5 numbered options appears (NOT AskUserQuestion dialog), option 1 is 'Continue to Stage 1 (recommended)', and typing 'pause' (keyword shortcut) creates a clean checkpoint"

---

#### Task 5: Integrate context-resume Skill

**Description**: Implement handoff file parsing and workflow resumption.

**Required Reading:**
- `procedures/skills/context-resume.md` (all 327 lines) - Complete context-resume specification
- `procedures/commands/continue.md` (all 250 lines) - /continue command integration
- `procedures/core/checkpoint-system.md` (lines 197-327) - Resume workflows

**Dependencies**: Task 1 (handoff format must match), Task 4 (presents resumption menu)

**Implementation Steps:**
1. Implement handoff file search in context-resume skill (context-resume.md lines 23-41):
   ```typescript
   function findHandoffFile(pluginName?: string) {
     if (pluginName) {
       // Search specific plugin
       return checkPaths([
         `plugins/${pluginName}/.continue-here.md`,
         `plugins/${pluginName}/.ideas/.continue-here.md`
       ])
     } else {
       // Search all plugins for most recent handoff
       return findMostRecentHandoff('plugins/**/.continue-here.md')
     }
   }
   ```
2. Implement YAML frontmatter parser:
   - Extract: plugin, stage, phase, status, last_updated, complexity_score, phased_implementation
   - Validate all required fields present
   - Handle malformed YAML gracefully with error message
3. Implement markdown section parser:
   - Parse "Resume Point" heading → extract current stage/phase
   - Parse "Completed So Far" → extract commit hashes and timeline
   - Parse "Next Steps" → extract numbered action list
   - Parse "Context to Preserve" → extract key decisions and files
4. Implement context loading (context-resume.md lines 111-164):
   - Load relevant contracts (parameter-spec.md, architecture.md, plan.md)
   - Load recent git commits for context
   - Load handoff-specified files for review
   - Reconstruct plugin state from PLUGINS.md
5. Present resumption options menu:
   ```
   Resuming: [PluginName] (Stage N.M)
   Last worked on: [timestamp]

   What would you like to do?
   1. Continue from Stage N.M (recommended)
   2. Review what was completed
   3. Skip to different stage
   4. Start Stage N.M over
   5. Other
   ```
6. Route to plugin-workflow at correct stage

**Expected Output:**
- Handoff files parsed correctly
- Context loaded (contracts, commits, files)
- Resumption menu presented
- Workflow resumes at exact checkpoint

**Verification:**
- Automated: `scripts/test-context-resume.sh` - Parses test handoff, validates context loading
- Manual: STOP AND ASK LEX: "Please verify that after pausing at Stage 1, exiting, and running /continue [Plugin], the context-resume skill successfully loads the handoff file, presents a resumption menu with the current stage (Stage 1), and continuing resumes plugin-workflow at the exact checkpoint with all contracts loaded"

---

### Phase 2b: Workflow Stages 0 & 1

#### Task 6: Implement Stage 0 (Research)

**Description**: Build research stage with Context7 JUCE docs lookup and professional plugin research.

**Required Reading:**
- `procedures/skills/plugin-workflow.md` (lines 25-38) - Stage 0 specification
- `procedures/skills/deep-research.md` (lines 59-115) - Graduated depth research protocol
- `architecture/03-model-selection-extended-thinking-strategy.md` (lines 50-93) - Stage 0 extended thinking
- `procedures/commands/implement.md` (lines 28-136) - Entry point and stage overview

**Dependencies**: Task 1-5 (checkpoint system, state machine, menus)

**Implementation Steps:**
1. Implement Stage 0 entry point in plugin-workflow skill:
   - Check preconditions: Plugin exists with creative-brief.md (💡 status in PLUGINS.md)
   - If creative-brief.md missing: Offer `/dream` to create one
   - Update PLUGINS.md status: 💡 → 🚧 Stage 0
   - Create handoff file (first checkpoint)
2. Configure Stage 0 research model (model-selection.md lines 50-68):
   - Model: Opus (complex reasoning for algorithm comparison)
   - Extended thinking: ENABLED
   - Budget: 10000 tokens
3. Implement research workflow:
   a. Read creative-brief.md for plugin concept
   b. Identify plugin type (Audio Effect / MIDI Instrument / Synth)
   c. **Context7 JUCE documentation lookup:**
      - Resolve library ID: `mcp__context7__resolve-library-id("JUCE")`
      - Get docs: `mcp__context7__get-library-docs("/juce-framework/JUCE", topic="dsp modules")`
      - Extract relevant JUCE DSP modules for plugin type
   d. Research professional plugin examples:
      - Identify industry leaders (FabFilter, Waves, etc.)
      - Document sonic characteristics
      - Note parameter ranges used
   e. Check design-sync (if mockup exists):
      - Invoke design-sync skill to validate brief ↔ mockup consistency
      - Document any conflicts found
   f. Assess technical feasibility:
      - Verify JUCE modules support requirements
      - Note complexity factors
      - Identify potential challenges
4. Generate `plugins/[Name]/.ideas/research.md` with sections:
   - Plugin Type Analysis
   - JUCE Modules Identified (with Context7 references)
   - Professional Examples Researched
   - Parameter Range Recommendations
   - Technical Feasibility Assessment
   - Design Sync Results (if applicable)
5. Update state:
   - Update handoff file: stage=0, status=complete
   - Add timeline entry to PLUGINS.md: "Stage 0: Research completed"
   - Git commit: `feat: [Plugin] Stage 0 - research complete`
6. Present decision menu (hard checkpoint):
   ```
   ✓ Stage 0 complete: research documented

   What's next?
   1. Continue to Stage 1 (recommended)
   2. Review research findings
   3. Improve creative brief based on research
   4. Run deeper investigation (deep-research skill)
   5. Pause here
   6. Other
   ```

**Expected Output:**
- `plugins/[Name]/.ideas/research.md` created with Context7 JUCE references
- Professional plugin research documented
- Technical feasibility assessed
- Handoff file created
- PLUGINS.md updated: 🚧 Stage 0 complete
- Git commit created
- Decision menu presented

**Verification:**
- Automated: `scripts/test-stage0.sh` - Validates research.md structure, Context7 references, git commit
- Manual: STOP AND ASK LEX: "Please verify Stage 0 research.md contains: (1) Context7 JUCE module references with library IDs, (2) professional plugin examples with specific parameter ranges, (3) technical feasibility assessment, and (4) decision menu appears with 6 options after completion"

---

#### Task 7: Implement Stage 1 (Planning with Contract Enforcement)

**Description**: Build planning stage with complexity scoring, contract enforcement blocking, and phase breakdown.

**Required Reading:**
- `procedures/skills/plugin-workflow.md` (lines 40-52, 157-180) - Stage 1 spec and complexity scoring
- `architecture/02-core-abstractions.md` (lines 110-183) - Contract definitions and enforcement
- `architecture/05-routing-architecture.md` (lines 513-578) - Precondition verification
- `ROADMAP.md` (lines 251-265) - Contract prerequisite enforcement (BLOCKS if missing)

**Dependencies**: Task 6 (Stage 0 must complete first)

**Implementation Steps:**
1. Implement Stage 1 precondition enforcement (CRITICAL):
   ```typescript
   function checkStage1Preconditions(pluginName) {
     const paramSpecExists = fileExists(`plugins/${pluginName}/.ideas/parameter-spec.md`)
     const archSpecExists = fileExists(`plugins/${pluginName}/.ideas/architecture.md`)
     const researchExists = fileExists(`plugins/${pluginName}/.ideas/research.md`)

     if (!researchExists) {
       return {
         allowed: false,
         reason: "Stage 0 research must complete before Stage 1",
         action: "Complete Stage 0 first"
       }
     }

     if (!paramSpecExists || !archSpecExists) {
       return {
         allowed: false,
         reason: "Cannot proceed to Stage 1 - missing implementation contracts",
         requiredContracts: {
           "parameter-spec.md": paramSpecExists ? "✓ exists" : "✗ MISSING (required)",
           "architecture.md": archSpecExists ? "✓ exists" : "✗ MISSING (required)"
         },
         action: "Complete ui-mockup two-phase workflow (design approval generates parameter-spec.md), then create architecture.md"
       }
     }

     return { allowed: true }
   }
   ```
2. **BLOCK execution if contracts missing** (non-negotiable per ROADMAP.md line 252):
   - Display clear error message explaining WHY blocked
   - Show which contracts exist vs missing
   - Suggest: "Complete ui-mockup two-phase workflow (Phase 4.5 design approval generates parameter-spec.md)"
   - Do NOT proceed to planning without contracts
3. Implement complexity scoring algorithm (plugin-workflow.md lines 170-180):
   ```typescript
   function calculateComplexity(paramSpec, archSpec) {
     // Parse parameter-spec.md
     const paramCount = extractParameterCount(paramSpec)

     // Parse architecture.md
     const algorithms = extractAlgorithms(archSpec) // DSP components

     // Identify special features
     const features = {
       feedbackLoops: containsFeedback(archSpec),
       fftProcessing: containsFFT(archSpec),
       multibandProcessing: containsMultiband(archSpec),
       modulationSystems: containsModulation(archSpec),
       externalMIDI: containsMIDI(archSpec)
     }

     // Calculate score
     const paramScore = Math.min(paramCount / 5, 2)
     const algoScore = algorithms.length
     const featureScore = Object.values(features).filter(Boolean).length

     const totalScore = Math.min(paramScore + algoScore + featureScore, 5)

     return {
       score: totalScore,
       breakdown: { paramScore, algoScore, featureScore },
       phased: totalScore >= 3
     }
   }
   ```
4. Implement planning logic:
   - Read contracts: parameter-spec.md (from ui-mockup Phase 4.5), architecture.md, research.md
   - Calculate complexity score
   - If score ≥3: Generate phase breakdown for Stage 4 (DSP) and Stage 5 (GUI)
   - If score ≤2: Single-pass implementation (one commit per stage)
5. Generate `plugins/[Name]/.ideas/plan.md` with sections:
   - **Complexity Assessment:** Score X/5 (breakdown shown)
   - **Implementation Strategy:** Single-pass vs Phased
   - **Stage Breakdown:**
     - Stage 2: Foundation (CMakeLists, empty files)
     - Stage 3: Shell (APVTS, basic processBlock)
     - Stage 4: DSP
       - If phased: Stage 4.1, 4.2, 4.3 with test criteria
     - Stage 5: GUI
       - If phased: Stage 5.1, 5.2 with test criteria
     - Stage 6: Validation (pluginval, presets)
   - **Contract References:**
     - parameter-spec.md: [N] parameters defined (from ui-mockup two-phase workflow)
     - architecture.md: [M] DSP components
6. Update state:
   - Update PLUGINS.md: Add timeline entry "Stage 1: Planning - Complexity X.Y"
   - Update handoff: stage=1, complexity_score=X.Y, phased_implementation=true/false
   - Git commit: `feat: [Plugin] Stage 1 - planning complete`
7. Present decision menu (hard checkpoint):
   ```
   ✓ Stage 1 complete: plan created (Complexity X.Y, [single-pass/phased])

   What's next?
   1. Continue to Stage 2 (recommended)
   2. Review plan details
   3. Adjust complexity assessment
   4. Review contracts
   5. Pause here
   6. Other
   ```

**Expected Output:**
- **Contract enforcement BLOCKS** if parameter-spec.md (from ui-mockup) or architecture.md missing
- Complexity score calculated correctly
- `plan.md` generated with phase breakdown if complexity ≥3
- Handoff includes complexity_score and phased_implementation
- Git commit created
- Decision menu presented

**Verification:**
- Automated: `scripts/test-stage1-contracts.sh` - Validates contract enforcement blocks without specs
- Automated: `scripts/test-complexity-scoring.sh` - Tests complexity calculation with various inputs
- Manual: STOP AND ASK LEX: "Please create a test plugin without parameter-spec.md and verify Stage 1 BLOCKS with error message listing missing contracts and suggesting ui-mockup workflow. Then add both contracts (parameter-spec.md from ui-mockup two-phase workflow, architecture.md manually) and verify Stage 1 proceeds, calculates complexity score correctly (show breakdown), and generates plan.md with appropriate phasing for score ≥3"

---

#### Task 8: Implement Stage Dispatch Logic

**Description**: Create stage dispatcher that routes to correct stage implementation or stub.

**Required Reading:**
- `procedures/skills/plugin-workflow.md` (lines 140-154) - Stage boundary protocol
- `architecture/04-component-architecture.md` (lines 420-487) - Lifecycle management and stage execution

**Dependencies**: Task 6, 7 (Stages 0-1 must exist)

**Implementation Steps:**
1. Create stage dispatcher function in plugin-workflow skill:
   ```typescript
   function dispatchStage(pluginName, stageNumber) {
     // Verify preconditions before dispatch
     const preconditionCheck = checkStagePreconditions(pluginName, stageNumber)
     if (!preconditionCheck.allowed) {
       displayError(preconditionCheck.reason, preconditionCheck.action)
       return { status: 'blocked', reason: preconditionCheck.reason }
     }

     // Route to stage implementation
     switch(stageNumber) {
       case 0:
         return executeStage0Research(pluginName)
       case 1:
         return executeStage1Planning(pluginName)
       case 2:
         return executeStage2FoundationStub(pluginName) // Phase 3
       case 3:
         return executeStage3ShellStub(pluginName) // Phase 3
       case 4:
         return executeStage4DSPStub(pluginName) // Phase 3
       case 5:
         return executeStage5GUIStub(pluginName) // Phase 3
       case 6:
         return executeStage6Validation(pluginName) // Task 11
       default:
         throw new Error(`Invalid stage: ${stageNumber}`)
     }
   }
   ```
2. Implement stage precondition checker:
   ```typescript
   function checkStagePreconditions(pluginName, stage) {
     // Stage 1: Requires research.md + contracts
     if (stage === 1) {
       return checkStage1Preconditions(pluginName) // From Task 7
     }

     // Stage 2-6: Requires previous stage complete
     if (stage >= 2) {
       const status = getPluginStatus(pluginName)
       const expectedPrevious = `🚧 Stage ${stage - 1}`
       if (!status.includes(expectedPrevious) && !status.includes('complete')) {
         return {
           allowed: false,
           reason: `Stage ${stage - 1} must complete before Stage ${stage}`,
           action: `Complete Stage ${stage - 1} first or use /continue`
         }
       }
     }

     return { allowed: true }
   }
   ```
3. Implement stage execution flow:
   - Check preconditions
   - Execute stage logic
   - Update state (PLUGINS.md, handoff)
   - Create git commit
   - Present decision menu
   - Handle user choice
   - Route to next stage or pause
4. Add stage loop for continuous execution:
   ```typescript
   function runWorkflow(pluginName, startStage = 0) {
     let currentStage = startStage
     let shouldContinue = true

     while (shouldContinue && currentStage <= 6) {
       const result = dispatchStage(pluginName, currentStage)

       if (result.status === 'blocked') {
         return result // Stop workflow
       }

       // Present decision menu
       const choice = presentDecisionMenu({
         stage: currentStage,
         completionStatement: result.completionStatement,
         pluginName: pluginName
       })

       if (choice === 'continue') {
         currentStage++
       } else if (choice === 'pause') {
         shouldContinue = false
       } else {
         // Handle other choices (review, test, etc.)
         handleMenuChoice(choice, pluginName, currentStage)
       }
     }
   }
   ```

**Expected Output:**
- Stage dispatcher routes correctly
- Preconditions checked before each stage
- Stage loop allows continuous execution or pausing
- Blocked stages display clear error messages

**Verification:**
- Automated: `scripts/test-stage-dispatcher.sh` - Tests routing to all stages, precondition enforcement
- Manual: STOP AND ASK LEX: "Please verify the stage dispatcher: (1) routes to Stage 0 when starting workflow, (2) blocks Stage 1 if contracts missing, (3) presents decision menu after each stage, and (4) continues to next stage when choosing option 1 'Continue'"

---

### Phase 2c: Stage 6, Validator & Hooks

#### Task 9: Implement Validator Subagent

**Description**: Create validator subagent for semantic quality validation at Stages 0, 1, 6.

**Required Reading:**
- `architecture/02-core-abstractions.md` (lines 184-252) - Hybrid validation strategy
- `architecture/04-component-architecture.md` (lines 164-409) - Validator interface and flow
- `architecture/17-testing-strategy.md` (lines 58-86) - Validator logic tests

**Dependencies**: None (parallel with other tasks, called by Stages 0, 1, 6)

**Implementation Steps:**
1. Create validator subagent file: `.claude/agents/validator.md`
2. Define validator frontmatter:
   ```yaml
   ---
   name: validator
   model: sonnet  # Quality over speed
   extended_thinking: false  # Deterministic validation
   tools: [Read, Grep, Bash]
   preconditions:
     - stage_complete: true
     - contracts_exist: true
   ---
   ```
3. Implement validator logic with 7 validation points (component-architecture.md lines 253-409):
   - **After Stage 0:** research.md complete with Context7 references
   - **After Stage 1:** plan.md references all contracts, complexity score valid
   - **After Stage 2:** CMakeLists.txt exists, builds successfully (stub in Phase 2)
   - **After Stage 3:** APVTS parameters match parameter-spec.md (stub in Phase 2)
   - **After Stage 4:** DSP components match architecture.md (stub in Phase 2)
   - **After Stage 5:** GUI bindings match parameter-spec.md (stub in Phase 2)
   - **After Stage 6:** pluginval passed, CHANGELOG exists
4. Define validation report format (JSON):
   ```json
   {
     "agent": "validator",
     "stage": N,
     "status": "PASS" | "FAIL",
     "checks": [
       {
         "name": "check_name",
         "passed": true,
         "message": "Validation message",
         "severity": "error" | "warning" | "info"
       }
     ],
     "recommendation": "What to do next",
     "continue_to_next_stage": true
   }
   ```
5. Implement Stage 0 validation:
   - Check research.md exists
   - Verify Context7 JUCE references present
   - Confirm professional examples documented
   - Validate technical feasibility section
6. Implement Stage 1 validation:
   - Check plan.md exists
   - Verify complexity score calculated correctly
   - Confirm all contracts referenced (parameter-spec, architecture)
   - Validate phase breakdown if complexity ≥3
   - Ensure stage breakdown includes all 7 stages
7. Implement Stage 6 validation:
   - Check pluginval log exists and passed
   - Verify CHANGELOG.md follows Keep a Changelog format
   - Confirm Presets/ directory has 3+ presets
   - Validate PLUGINS.md updated to ✅ Working
8. Create validator invocation in plugin-workflow:
   ```typescript
   function validateStage(pluginName, stage) {
     const result = Task({
       subagent_type: "validator",
       description: `Validate ${pluginName} Stage ${stage}`,
       prompt: `
Validate Stage ${stage} completion for ${pluginName}.

**Stage:** ${stage}
**Plugin:** ${pluginName}
**Contracts:**
- parameter-spec.md: [paste content]
- architecture.md: [paste content]
- plan.md: [paste content]

**Expected outputs for Stage ${stage}:**
[stage-specific outputs]

Return JSON validation report with status, checks, and recommendation.
       `
     })

     const report = JSON.parse(result)

     if (report.status === "FAIL") {
       presentValidationFailure(report)
       // Decision menu: Fix / Re-run / Override
     }

     return report
   }
   ```

**Expected Output:**
- `.claude/agents/validator.md` created
- Validation logic for Stages 0, 1, 6
- JSON report format standardized
- Integration with plugin-workflow

**Verification:**
- Automated: `scripts/test-validator.sh` - Tests Stage 0, 1, 6 validation with pass/fail cases
- Manual: STOP AND ASK LEX: "Please verify the validator subagent: (1) validates Stage 0 research.md has Context7 references, (2) validates Stage 1 plan.md references contracts correctly, and (3) returns JSON report with status PASS/FAIL, checks array, and recommendation"

---

#### Task 10: Implement Hook System

**Description**: Create hook scripts for context injection, stage enforcement, and contract preservation.

**Required Reading:**
- `architecture/10-extension-architecture.md` (lines 96-165) - Hook system architecture
- `architecture/13-error-handling-recovery.md` (lines 180-683) - Complete hook specifications
- `procedures/core/checkpoint-system.md` (lines 328-385) - Hook integration with checkpoints

**Dependencies**: Task 1-2 (hooks read/update state files)

**Implementation Steps:**
1. Create hook directory structure:
   ```
   .claude/hooks/
   ├── UserPromptSubmit.sh
   ├── Stop.sh
   ├── PreCompact.sh
   ├── PostToolUse.sh (future)
   ├── SubagentStop.sh (future - Phase 3)
   ├── SessionStart.sh (future)
   └── validators/ (Python scripts for Phase 3)
   ```
2. Implement UserPromptSubmit hook (error-handling.md lines 187-220):
   ```bash
   #!/bin/bash
   # UserPromptSubmit - Auto-inject context for /continue

   # Check relevance FIRST
   if [[ ! "$USER_PROMPT" =~ ^/continue ]]; then
     echo "Hook not relevant (not /continue command), skipping gracefully"
     exit 0
   fi

   # Extract plugin name
   PLUGIN_NAME=$(echo "$USER_PROMPT" | awk '{print $2}')

   # Find handoff file
   if [ -n "$PLUGIN_NAME" ]; then
     HANDOFF="plugins/$PLUGIN_NAME/.continue-here.md"
   else
     HANDOFF=$(find plugins -name ".continue-here.md" -type f -printf '%T@ %p\n' | sort -rn | head -1 | cut -d' ' -f2)
   fi

   if [ ! -f "$HANDOFF" ]; then
     echo "No handoff file found"
     exit 0
   fi

   # Inject handoff content into context
   echo "Loading context from $HANDOFF..."
   cat "$HANDOFF"

   # Load referenced contracts
   PLUGIN=$(dirname "$HANDOFF" | xargs basename)
   echo "\n--- Contracts ---"
   [ -f "plugins/$PLUGIN/.ideas/parameter-spec.md" ] && cat "plugins/$PLUGIN/.ideas/parameter-spec.md"
   [ -f "plugins/$PLUGIN/.ideas/architecture.md" ] && cat "plugins/$PLUGIN/.ideas/architecture.md"

   exit 0
   ```
3. Implement Stop hook (error-handling.md lines 224-257):
   ```bash
   #!/bin/bash
   # Stop - Stage completion enforcement

   # Check if workflow in progress
   PLUGIN_STATUS=$(grep "🚧" PLUGINS.md | head -1)
   if [ -z "$PLUGIN_STATUS" ]; then
     echo "No workflow in progress, skipping"
     exit 0
   fi

   # Extract plugin name and stage
   PLUGIN_NAME=$(echo "$PLUGIN_STATUS" | grep -oP '### \K\w+')
   CURRENT_STAGE=$(echo "$PLUGIN_STATUS" | grep -oP 'Stage \K\d+')

   # Verify stage committed
   LAST_COMMIT=$(git log -1 --format="%s")
   if [[ ! "$LAST_COMMIT" =~ "Stage $CURRENT_STAGE" ]]; then
     echo "⚠️ Warning: Stage $CURRENT_STAGE not committed"
     echo "Expected commit for Stage $CURRENT_STAGE, found: $LAST_COMMIT"
     exit 1  # Block if stage not properly committed
   fi

   echo "Stage $CURRENT_STAGE properly committed"
   exit 0
   ```
4. Implement PreCompact hook (error-handling.md lines 310-337):
   ```bash
   #!/bin/bash
   # PreCompact - Preserve contracts before context compaction

   # Find all plugins with contracts
   PLUGINS=$(find plugins -type d -maxdepth 1 -mindepth 1)

   PRESERVED=""
   for PLUGIN in $PLUGINS; do
     PLUGIN_NAME=$(basename "$PLUGIN")

     # Preserve critical contracts
     if [ -f "$PLUGIN/.ideas/parameter-spec.md" ]; then
       PRESERVED="$PRESERVED\n--- $PLUGIN_NAME parameter-spec.md ---\n"
       PRESERVED="$PRESERVED$(cat "$PLUGIN/.ideas/parameter-spec.md")\n"
     fi

     if [ -f "$PLUGIN/.ideas/architecture.md" ]; then
       PRESERVED="$PRESERVED\n--- $PLUGIN_NAME architecture.md ---\n"
       PRESERVED="$PRESERVED$(cat "$PLUGIN/.ideas/architecture.md")\n"
     fi
   done

   if [ -n "$PRESERVED" ]; then
     echo "Preserving contracts before compaction:"
     echo -e "$PRESERVED"
   fi

   exit 0
   ```
5. Make hooks executable:
   ```bash
   chmod +x .claude/hooks/*.sh
   ```
6. Test hook conditional execution pattern (CRITICAL from error-handling.md lines 127-159):
   - All hooks MUST check relevance first
   - Exit 0 (success) when not relevant - NEVER exit 1
   - Only execute hook logic if conditions met
   - Graceful skipping prevents false failures

**Expected Output:**
- 3 hook scripts created (UserPromptSubmit, Stop, PreCompact)
- All hooks executable
- Conditional execution pattern implemented
- Context injection working on /continue
- Stage enforcement prevents uncommitted stages
- Contract preservation before compaction

**Verification:**
- Automated: `scripts/test-hooks.sh` - Tests all 3 hooks with relevant/irrelevant conditions
- Manual: STOP AND ASK LEX: "Please verify: (1) /continue command auto-loads handoff file via UserPromptSubmit hook, (2) Stop hook blocks if stage not committed to git, (3) PreCompact hook preserves parameter-spec.md and architecture.md before context compaction, and (4) all hooks exit 0 (not 1) when conditions not relevant"

---

#### Task 11: Implement Stage 6 (Validation & Presets)

**Description**: Build validation stage with pluginval, factory presets, and CHANGELOG generation.

**Required Reading:**
- `procedures/skills/plugin-workflow.md` (lines 122-137) - Stage 6 specification
- `procedures/skills/plugin-testing.md` (lines 23-134) - Testing and validation
- `procedures/skills/plugin-lifecycle.md` (lines 23-113) - Installation process
- `architecture/06-state-architecture.md` (lines 317-339) - CHANGELOG format

**Dependencies**: Task 8 (stage dispatch), Task 9 (validator), Task 10 (hooks)

**Implementation Steps:**
1. Implement Stage 6 entry point:
   - Precondition: Stage 5 complete (or Stage 5 stub in Phase 2)
   - Update PLUGINS.md: 🚧 Stage 6
   - Update handoff: stage=6, status=in_progress
2. Create factory presets (even for stubs):
   ```typescript
   function generateFactoryPresets(pluginName) {
     const paramSpec = readFile(`plugins/${pluginName}/.ideas/parameter-spec.md`)
     const parameters = parseParameters(paramSpec)

     // Generate 3-5 musically useful presets
     const presets = [
       createPreset("Default", parameters, "default_values"),
       createPreset("Subtle", parameters, "conservative_values"),
       createPreset("Extreme", parameters, "maximum_values")
     ]

     // Create Presets/ directory
     mkdir(`plugins/${pluginName}/Presets`)

     // Write preset files (JUCE preset format)
     presets.forEach(preset => {
       writeFile(
         `plugins/${pluginName}/Presets/${preset.name}.preset`,
         formatJUCEPreset(preset)
       )
     })

     return presets.map(p => p.name)
   }
   ```
3. Invoke pluginval (if plugin built):
   ```typescript
   function runPluginval(pluginName) {
     // Check if build exists
     const vst3Path = `plugins/${pluginName}/build/${pluginName}_artefacts/VST3/${pluginName}.vst3`

     if (!fileExists(vst3Path)) {
       return {
         skipped: true,
         reason: "No build found (Stages 2-5 are stubs in Phase 2)"
       }
     }

     // Run pluginval with strictness level 5
     const result = bash(`pluginval --validate --strictness-level 5 "${vst3Path}" > logs/${pluginName}/pluginval_$(date +%Y%m%d_%H%M%S).log 2>&1`)

     // Parse results
     const log = readFile(`logs/${pluginName}/pluginval_latest.log`)
     const passed = log.includes("All tests passed")

     return { passed, log }
   }
   ```
4. Generate CHANGELOG.md (Keep a Changelog format):
   ```typescript
   function generateChangelog(pluginName) {
     const brief = readFile(`plugins/${pluginName}/.ideas/creative-brief.md`)
     const paramSpec = readFile(`plugins/${pluginName}/.ideas/parameter-spec.md`)
     const parameters = parseParameters(paramSpec)

     const changelog = `# Changelog

All notable changes to ${pluginName} will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - ${getCurrentDate()}

### Added
- Initial release
- ${extractFeatures(brief).join('\n- ')}
- ${parameters.length} parameters: ${parameters.map(p => p.name).join(', ')}
- Factory presets: Default, Subtle, Extreme

[1.0.0]: https://github.com/yourrepo/${pluginName}/releases/tag/v1.0.0
`

     writeFile(`plugins/${pluginName}/CHANGELOG.md`, changelog)
   }
   ```
5. Update PLUGINS.md to ✅ Working:
   ```typescript
   function finalizePlugin(pluginName) {
     // Update status
     updatePluginStatus(pluginName, "✅ Working")

     // Add validation results to entry
     const entry = getPluginEntry(pluginName)
     entry.addSection("Validation", [
       "✓ Factory presets created (3)",
       pluginvalPassed ? "✓ pluginval tests passed" : "⚠️ pluginval skipped (no build)",
       "✓ CHANGELOG.md generated"
     ])
     entry.setVersion("1.0.0")

     // Delete handoff file (plugin complete)
     deleteFile(`plugins/${pluginName}/.continue-here.md`)
   }
   ```
6. Invoke validator for Stage 6:
   ```typescript
   const validation = validateStage(pluginName, 6)

   if (validation.status === "FAIL") {
     presentValidationFailure(validation)
     // Decision menu: Fix issues / Continue anyway / Pause
   }
   ```
7. Git commit:
   ```bash
   git commit -m "feat: ${pluginName} Stage 6 - validation complete

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>"
   ```
8. Present final decision menu:
   ```
   ✓ Stage 6 complete: ${pluginName} ready for installation

   What's next?
   1. Install plugin to system folders (recommended)
   2. Test in DAW first
   3. Create another plugin
   4. Review complete plugin code
   5. Document this plugin
   6. Other
   ```

**Expected Output:**
- Presets/ directory with 3+ factory presets
- pluginval executed (or skipped if no build)
- CHANGELOG.md in Keep a Changelog format
- PLUGINS.md status → ✅ Working
- .continue-here.md deleted
- Validator validates Stage 6
- Git commit created
- Decision menu offers installation

**Verification:**
- Automated: `scripts/test-stage6.sh` - Validates presets, CHANGELOG format, PLUGINS.md status
- Manual: STOP AND ASK LEX: "Please verify Stage 6: (1) creates Presets/ with 3+ .preset files, (2) generates CHANGELOG.md following Keep a Changelog format with version 1.0.0, (3) updates PLUGINS.md to '✅ Working', (4) deletes .continue-here.md, (5) validator validates Stage 6 completion, and (6) decision menu offers 'Install plugin to system folders' as option 1"

---

#### Task 12: Implement Stages 2-5 Stubs

**Description**: Create placeholder implementations for code generation stages completed in Phase 3.

**Required Reading:**
- `procedures/skills/plugin-workflow.md` (lines 54-120) - Stages 2-5 specifications
- `architecture/07-communication-architecture.md` (lines 1-81) - Subagent dispatch pattern (reference)

**Dependencies**: Task 8 (stage dispatcher calls stubs)

**Implementation Steps:**
1. Create Stage 2 stub (Foundation):
   ```typescript
   function executeStage2FoundationStub(pluginName) {
     displayMessage(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Stage 2: Foundation (Build System Setup)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

TODO (Phase 3): Dispatch foundation-agent subagent

Expected implementation:
- Generate CMakeLists.txt with JUCE 8 configuration
- Create empty PluginProcessor.h/cpp
- Create empty PluginEditor.h/cpp
- Verify compilation with --no-install flag

For now, marking stage as stub and continuing...
     `)

     // Update state
     updatePluginStatus(pluginName, "🚧 Stage 2 (stub)")
     updateHandoff(pluginName, 2, ["Foundation stub executed"], [
       "Complete Phase 3 to implement foundation-agent",
       "foundation-agent will create build system and empty plugin files"
     ])

     // No git commit for stub (not real implementation)

     // Present stub menu
     return presentStubDecisionMenu(pluginName, 2, "Continue to Stage 3 (stub)")
   }
   ```
2. Create Stage 3 stub (Shell):
   ```typescript
   function executeStage3ShellStub(pluginName) {
     displayMessage(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Stage 3: Shell (Parameter Setup)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

TODO (Phase 3): Dispatch shell-agent subagent

Expected implementation:
- Create APVTS with all parameters from parameter-spec.md
- Implement basic processBlock stub
- Empty editor with placeholder text
- Verify plugin loads in DAW

For now, marking stage as stub and continuing...
     `)

     updatePluginStatus(pluginName, "🚧 Stage 3 (stub)")
     updateHandoff(pluginName, 3, ["Shell stub executed"], [
       "Complete Phase 3 to implement shell-agent",
       "shell-agent will implement parameters and basic structure"
     ])

     return presentStubDecisionMenu(pluginName, 3, "Continue to Stage 4 (stub)")
   }
   ```
3. Create Stage 4 stub (DSP):
   ```typescript
   function executeStage4DSPStub(pluginName) {
     const plan = readFile(`plugins/${pluginName}/.ideas/plan.md`)
     const phased = plan.includes("Stage 4.1") // Check if phased

     displayMessage(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Stage 4: DSP (Audio Processing)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

TODO (Phase 3): Dispatch dsp-agent subagent

Expected implementation:
- Implement processBlock with DSP algorithms from architecture.md
- ${phased ? 'Phased execution: Stage 4.1, 4.2, 4.3 with test criteria' : 'Single-pass implementation'}
- Real audio processing
- Automated stability tests after completion

For now, marking stage as stub and continuing...
     `)

     updatePluginStatus(pluginName, "🚧 Stage 4 (stub)")
     updateHandoff(pluginName, 4, ["DSP stub executed"], [
       "Complete Phase 3 to implement dsp-agent",
       "dsp-agent will implement audio processing from architecture.md"
     ])

     return presentStubDecisionMenu(pluginName, 4, "Continue to Stage 5 (stub)")
   }
   ```
4. Create Stage 5 stub (GUI):
   ```typescript
   function executeStage5GUIStub(pluginName) {
     const plan = readFile(`plugins/${pluginName}/.ideas/plan.md`)
     const phased = plan.includes("Stage 5.1") // Check if phased

     displayMessage(`
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Stage 5: GUI (User Interface)
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

TODO (Phase 3): Dispatch gui-agent subagent

Expected implementation:
- Create WebView UI with parameter bindings
- ${phased ? 'Phased execution: Stage 5.1, 5.2 with test criteria' : 'Single-pass implementation'}
- FlexBox/Grid layout (never manual setBounds)
- Automated stability tests after completion

For now, marking stage as stub and continuing...
     `)

     updatePluginStatus(pluginName, "🚧 Stage 5 (stub)")
     updateHandoff(pluginName, 5, ["GUI stub executed"], [
       "Complete Phase 3 to implement gui-agent",
       "gui-agent will create WebView UI with parameter bindings"
     ])

     return presentStubDecisionMenu(pluginName, 5, "Continue to Stage 6 (real implementation)")
   }
   ```
5. Create stub decision menu function:
   ```typescript
   function presentStubDecisionMenu(pluginName, stage, continueLabel) {
     return presentDecisionMenu({
       pluginName,
       stage,
       completionStatement: `Stage ${stage} stub executed (Phase 3 will implement)`,
       options: [
         { label: continueLabel, recommended: true },
         { label: "Review handoff file", action: "review_handoff" },
         { label: "Skip to Stage 6 (test stub workflow)", action: "skip_to_6" },
         { label: "Pause here", action: "pause" },
         { label: "Other", action: "other" }
       ]
     })
   }
   ```
6. Add Phase 3 preparation documentation in stubs:
   - Document subagent interface expected
   - List Task tool parameters needed
   - Reference architecture/07-communication-architecture.md for dispatch pattern
   - Note JSON report format subagents must return

**Expected Output:**
- Stages 2-5 execute as clear stubs with TODO messages
- Each stub updates PLUGINS.md status (🚧 Stage N stub)
- Each stub updates handoff file with Phase 3 note
- Stub menus allow continuing through workflow
- No git commits for stubs (not real implementation)
- Clear indication Phase 3 will implement these stages

**Verification:**
- Automated: `scripts/test-stub-workflow.sh` - Runs Stage 0 → 1 → 2-5 (stubs) → 6, validates reaches Stage 6
- Manual: STOP AND ASK LEX: "Please verify you can run a test plugin through the full workflow from Stage 0 to Stage 6, with Stages 2-5 showing clear 'TODO (Phase 3)' stub messages in formatted boxes, cleanly transitioning between stages without errors, and Stage 6 executing normally with real implementation (presets, CHANGELOG, etc.)"

---

#### Task 13: End-to-End Workflow Integration Testing

**Description**: Test complete workflow from /implement through Stage 6 with all Phase 2 components integrated.

**Required Reading:**
- `procedures/skills/plugin-workflow.md` (all) - Complete workflow specification
- `procedures/commands/implement.md` (all) - Entry point
- `procedures/commands/continue.md` (all) - Resumption

**Dependencies**: All previous tasks (complete Phase 2 implementation)

**Implementation Steps:**
1. Create comprehensive test plugin specification:
   - Name: "TestWorkflowPhase2"
   - Create `plugins/TestWorkflowPhase2/.ideas/creative-brief.md`:
     ```markdown
     # TestWorkflowPhase2 - Simple Gain Plugin

     ## Type
     Audio Effect

     ## Concept
     Basic gain control plugin for testing Phase 2 workflow.

     ## Features
     - Single gain parameter
     - Simple audio processing
     - Minimal complexity for testing
     ```
   - Create `plugins/TestWorkflowPhase2/.ideas/parameter-spec.md`:
     ```markdown
     # Parameter Specification

     ## GAIN
     - Type: Float
     - Range: -60.0 to 12.0 dB
     - Default: 0.0 dB
     - Skew: 1.0
     - UI Control: Rotary slider
     - DSP Usage: Applied to audio signal amplitude
     ```
   - Create `plugins/TestWorkflowPhase2/.ideas/architecture.md`:
     ```markdown
     # DSP Architecture

     ## Core Components
     - juce::dsp::Gain (amplitude control)

     ## Processing Chain
     Input → Gain → Output

     ## Parameter Mapping
     | Parameter | DSP Component | Usage |
     |-----------|---------------|-------|
     | GAIN | juce::dsp::Gain | setGainDecibels(GAIN) |
     ```
2. Execute full workflow test:
   - Run: `/implement TestWorkflowPhase2`
   - Verify Stage 0:
     - research.md created with Context7 JUCE references
     - Decision menu appears (6 options)
     - Choose "1. Continue to Stage 1"
   - Verify Stage 1:
     - **Contracts checked:** parameter-spec.md ✓, architecture.md ✓
     - Stage 1 proceeds (not blocked)
     - Complexity calculated: ~1.2 (simple plugin)
     - plan.md created with single-pass strategy
     - Decision menu appears
     - Choose "1. Continue to Stage 2"
   - Verify Stages 2-5 (stubs):
     - Each stage shows TODO message in formatted box
     - PLUGINS.md updated: 🚧 Stage N (stub)
     - Handoff updated with Phase 3 note
     - Decision menu allows continuing
     - Choose "1. Continue to next stage" for each
   - Verify Stage 6:
     - Presets/ created with 3 .preset files
     - CHANGELOG.md generated (Keep a Changelog format, v1.0.0)
     - PLUGINS.md updated: ✅ Working
     - .continue-here.md DELETED
     - Validator validates Stage 6
     - Decision menu appears with installation option
3. Verify state management throughout:
   - **PLUGINS.md transitions:**
     - Initial: 💡 Ideated
     - Stage 0: 🚧 Stage 0
     - Stage 1: 🚧 Stage 1
     - Stages 2-5: 🚧 Stage N (stub)
     - Stage 6: ✅ Working
   - **Handoff file lifecycle:**
     - Created after Stage 0 (with correct YAML)
     - Updated after Stages 1, 2, 3, 4, 5 (with complexity_score, phased=false)
     - Deleted after Stage 6
   - **Git commits:**
     - Stage 0: `feat: TestWorkflowPhase2 Stage 0 - research complete`
     - Stage 1: `feat: TestWorkflowPhase2 Stage 1 - planning complete`
     - No commits for Stages 2-5 (stubs)
     - Stage 6: `feat: TestWorkflowPhase2 Stage 6 - validation complete`
4. Test resumption workflow:
   - Start new plugin: `TestWorkflowPhase2Resume`
   - Complete Stage 0
   - Complete Stage 1
   - Choose "5. Pause here" at Stage 2 stub
   - Verify handoff file exists with stage=2
   - Exit conversation
   - Start new conversation
   - Run: `/continue TestWorkflowPhase2Resume`
   - Verify UserPromptSubmit hook auto-loads context
   - Verify context-resume presents resumption menu
   - Choose "1. Continue from Stage 2"
   - Verify workflow resumes at Stage 2 stub
   - Continue through Stage 6
5. Test contract enforcement:
   - Create plugin without contracts: `TestContractEnforcement`
   - Create only creative-brief.md
   - Run: `/implement TestContractEnforcement`
   - Complete Stage 0 (research)
   - Attempt Stage 1
   - **Verify BLOCKS with error:**
     ```
     ✗ Cannot proceed to Stage 1 - missing implementation contracts

     Required contracts:
     ✗ parameter-spec.md - MISSING (required)
     ✗ architecture.md - MISSING (required)

     Next step: Finalize UI mockup to generate parameter-spec.md
     ```
   - Add contracts
   - Retry Stage 1
   - Verify proceeds successfully
6. Test decision menus:
   - Verify inline numbered lists appear (NOT AskUserQuestion)
   - Test keyword shortcuts: type "continue", "pause", "review"
   - Verify "Pause here" creates clean checkpoint
   - Verify "Other" accepts free-form input and re-presents menu
7. Test validator integration:
   - Verify validator called after Stage 0 (validates research.md)
   - Verify validator called after Stage 1 (validates plan.md)
   - Verify validator called after Stage 6 (validates CHANGELOG, presets)
8. Test hook system:
   - Verify UserPromptSubmit loads context on /continue
   - Verify Stop hook blocks if stage not committed (manually test by skipping commit)
   - Verify PreCompact preserves contracts (trigger compaction if possible)
9. Verify final state:
   - PLUGINS.md shows ✅ Working status for TestWorkflowPhase2
   - All timeline entries present in PLUGINS.md
   - CHANGELOG.md exists with v1.0.0
   - Git log shows Stage 0, 1, 6 commits (not 2-5 stubs)
   - Presets/ directory has 3 files
   - .continue-here.md deleted (complete plugin)

**Expected Output:**
- Complete workflow executes from Stage 0 through Stage 6
- All state files updated correctly
- Contract enforcement blocks Stage 1 when specs missing
- Resumption works from any checkpoint
- Decision menus functional with keyword shortcuts
- Validator validates Stages 0, 1, 6
- Hooks inject context and enforce stage commits
- Test plugin reaches ✅ Working status

**Verification:**
- Automated: `scripts/test-workflow-e2e-phase2.sh` - Comprehensive end-to-end test
- Manual: STOP AND ASK LEX: "Please verify the complete Phase 2 workflow: (1) TestWorkflowPhase2 completes Stages 0,1,2-5 (stubs),6 successfully, (2) contract enforcement blocks Stage 1 without specs, (3) pausing at Stage 2 and resuming with /continue works with context auto-loaded, (4) decision menus appear at all checkpoints with inline numbered lists, (5) validator validates Stages 0,1,6, (6) PLUGINS.md transitions correctly through all states ending at ✅ Working, (7) handoff file deleted after Stage 6, and (8) git log shows commits for Stages 0,1,6 only (not stubs)"

---

## Comprehensive Verification Criteria

### Automated Tests

```bash
# Test 1: Handoff file format validation
./scripts/test-handoff-format.sh
# Verifies: YAML frontmatter valid, markdown structure correct

# Test 2: State machine transitions
./scripts/test-state-machine.sh
# Verifies: Legal transitions pass, illegal blocked, single 🚧 enforced

# Test 3: Git commit format
git log --grep="feat:.*Stage" --format="%s" | grep -E "feat: \w+ Stage [0-6]"
# Verifies: Commits follow standard format

# Test 4: Contract enforcement
./scripts/test-stage1-contracts.sh
# Verifies: Stage 1 blocks without parameter-spec.md and architecture.md

# Test 5: Complexity scoring
./scripts/test-complexity-scoring.sh
# Verifies: Complexity calculation correct, phasing logic valid

# Test 6: Stage 0 research
./scripts/test-stage0.sh
# Verifies: research.md structure, Context7 references, decision menu

# Test 7: Context resume parsing
./scripts/test-context-resume.sh
# Verifies: Handoff parsing, context loading, resumption

# Test 8: Decision menu format
./scripts/test-decision-menus.sh
# Verifies: Inline numbered lists, context-aware options, keyword shortcuts

# Test 9: Validator subagent
./scripts/test-validator.sh
# Verifies: Stage 0,1,6 validation, JSON report format

# Test 10: Hook system
./scripts/test-hooks.sh
# Verifies: UserPromptSubmit, Stop, PreCompact hooks functional

# Test 11: Stage 6 outputs
./scripts/test-stage6.sh
# Verifies: Presets created, CHANGELOG format, PLUGINS.md ✅

# Test 12: Stub workflow
./scripts/test-stub-workflow.sh
# Verifies: Stages 2-5 stubs allow workflow continuation

# Test 13: End-to-end workflow
./scripts/test-workflow-e2e-phase2.sh TestWorkflowPhase2
# Verifies: Complete workflow from Stage 0 through Stage 6

# Test 14: Stage dispatcher
./scripts/test-stage-dispatcher.sh
# Verifies: Routing, preconditions, stage loop
```

### Manual Verification Checklist

**Checkpoint System:**
- [ ] Handoff template matches checkpoint-system.md format exactly (YAML + markdown sections)
- [ ] Handoff file created after Stage 0 with all required fields
- [ ] Handoff file updated after each stage with correct data
- [ ] Handoff file deleted when plugin reaches ✅ Working
- [ ] Hard checkpoints (Stages 0,1,6) pause for user decision

**State Management:**
- [ ] PLUGINS.md state machine prevents illegal transitions
- [ ] PLUGINS.md prevents starting second plugin when one is 🚧
- [ ] PLUGINS.md entry includes all sections (status, timeline, etc.)
- [ ] Single 🚧 constraint enforced across all plugins

**Git Workflow:**
- [ ] Git commits created after Stages 0, 1, 6 (not stub stages 2-5)
- [ ] Commit format: `feat: [Plugin] Stage N - description`
- [ ] Commits are atomic (include Source/, PLUGINS.md, handoff in single commit)
- [ ] Commit hash displayed to user after each commit

**Decision Menus:**
- [ ] Inline numbered lists used (NOT AskUserQuestion tool)
- [ ] Context-aware options generated based on stage/status
- [ ] Keyword shortcuts work ("continue", "pause", "review")
- [ ] "Other" option accepts free-form input and re-presents menu
- [ ] Discovery markers visible (`← User discovers [feature]`)
- [ ] Recommendation markers visible (`(recommended)`)

**Stage 0 (Research):**
- [ ] Context7 JUCE documentation lookup executes
- [ ] research.md contains Context7 library references
- [ ] Professional plugin examples documented
- [ ] Technical feasibility assessment included
- [ ] Decision menu appears with 6 options
- [ ] Extended thinking enabled (Opus model)

**Stage 1 (Planning):**
- [ ] **Contract enforcement BLOCKS without parameter-spec.md**
- [ ] **Contract enforcement BLOCKS without architecture.md**
- [ ] Error message explains WHY blocked and HOW to unblock
- [ ] Complexity score calculated correctly with breakdown shown
- [ ] plan.md includes phase breakdown for complexity ≥3
- [ ] plan.md includes single-pass note for complexity ≤2
- [ ] Handoff includes complexity_score and phased_implementation fields
- [ ] Decision menu appears with 6 options

**Stage 6 (Validation):**
- [ ] Presets/ directory created with 3+ factory preset files
- [ ] pluginval executed (or skipped with clear message if no build)
- [ ] CHANGELOG.md generated in Keep a Changelog format
- [ ] CHANGELOG.md version 1.0.0 for initial release
- [ ] PLUGINS.md status updated: 🚧 Stage 6 → ✅ Working
- [ ] .continue-here.md deleted
- [ ] Decision menu offers installation as option 1

**Stages 2-5 (Stubs):**
- [ ] Each stub displays TODO message in formatted box
- [ ] Stub messages clearly state "Phase 3 will implement"
- [ ] Stubs update PLUGINS.md: 🚧 Stage N (stub)
- [ ] Stubs update handoff with Phase 3 note
- [ ] Stubs allow workflow to continue cleanly
- [ ] No git commits created for stubs

**Validator Integration:**
- [ ] Validator subagent file exists: `.claude/agents/validator.md`
- [ ] Validator called after Stage 0 (validates research.md)
- [ ] Validator called after Stage 1 (validates plan.md, contracts)
- [ ] Validator called after Stage 6 (validates CHANGELOG, presets)
- [ ] Validator returns JSON with status, checks, recommendation
- [ ] Validation failure presents 3-option menu (Fix/Re-run/Override)

**Hook System:**
- [ ] UserPromptSubmit hook auto-loads context on /continue
- [ ] UserPromptSubmit hook loads handoff file and contracts
- [ ] Stop hook verifies stage committed before session end
- [ ] Stop hook blocks if commit missing
- [ ] PreCompact hook preserves parameter-spec.md and architecture.md
- [ ] All hooks check relevance first (conditional execution)
- [ ] All hooks exit 0 when not relevant (graceful skip)

**context-resume Integration:**
- [ ] /continue command finds handoff file (with or without plugin name)
- [ ] Handoff YAML parsed correctly
- [ ] Handoff markdown sections parsed correctly
- [ ] Contracts loaded (parameter-spec, architecture, plan)
- [ ] Recent commits loaded for context
- [ ] Resumption menu presented with current stage
- [ ] Workflow resumes at exact checkpoint

**End-to-End Workflow:**
- [ ] TestWorkflowPhase2 completes Stage 0 → 1 → 2-5 (stubs) → 6
- [ ] Contract enforcement blocks Stage 1 without specs
- [ ] Pausing at Stage 2 and resuming with /continue works
- [ ] All state transitions correct (💡 → 🚧 → ✅)
- [ ] Final plugin status: ✅ Working in PLUGINS.md
- [ ] Git log shows commits for Stages 0, 1, 6 only
- [ ] CHANGELOG.md exists with v1.0.0
- [ ] Presets/ directory has 3+ files
- [ ] .continue-here.md deleted after completion

### File Existence Verification

```bash
# After complete workflow, verify all expected files:

# Handoff template
ls -la .claude/skills/plugin-workflow/assets/continue-here-template.md

# Test plugin outputs
ls -la plugins/TestWorkflowPhase2/.ideas/research.md
ls -la plugins/TestWorkflowPhase2/.ideas/plan.md
ls -la plugins/TestWorkflowPhase2/CHANGELOG.md
ls -la plugins/TestWorkflowPhase2/Presets/
ls plugins/TestWorkflowPhase2/Presets/ | wc -l  # Should be ≥3

# Handoff deleted after completion
test ! -f plugins/TestWorkflowPhase2/.continue-here.md && echo "✓ Handoff deleted" || echo "✗ Handoff still exists"

# Validator subagent
ls -la .claude/agents/validator.md

# Hook scripts
ls -la .claude/hooks/UserPromptSubmit.sh
ls -la .claude/hooks/Stop.sh
ls -la .claude/hooks/PreCompact.sh

# Hooks executable
test -x .claude/hooks/UserPromptSubmit.sh && echo "✓ UserPromptSubmit executable"
test -x .claude/hooks/Stop.sh && echo "✓ Stop executable"
test -x .claude/hooks/PreCompact.sh && echo "✓ PreCompact executable"

# PLUGINS.md status
grep "✅ Working" PLUGINS.md | grep "TestWorkflowPhase2"

# Test scripts exist
ls -la scripts/test-*.sh
```

### Integration Tests

**Checkpoint System Integration:**
1. Create plugin → Stage 0 → verify handoff created with YAML
2. Continue Stage 1 → verify handoff updated with complexity_score
3. Pause at Stage 2 stub → verify handoff preserved
4. Resume with /continue → verify context loaded
5. Complete Stage 6 → verify handoff deleted

**Contract Enforcement Integration:**
1. Create plugin with only creative-brief.md
2. Complete Stage 0 research
3. Attempt Stage 1 → verify BLOCKED
4. Add parameter-spec.md only → verify still BLOCKED
5. Add architecture.md → verify Stage 1 proceeds
6. Verify complexity scoring uses both contracts

**Decision Menu Integration:**
1. Complete Stage 0 → verify 6-option inline menu appears
2. Type "pause" (keyword) → verify creates checkpoint
3. Type "continue" (keyword) → verify proceeds to next stage
4. Choose "5. Pause here" → verify handoff updated and workflow paused
5. Choose "1. Continue" → verify proceeds automatically

**Validator Integration:**
1. Complete Stage 0 → verify validator validates research.md
2. Validator reports PASS → workflow continues
3. Complete Stage 1 → verify validator validates plan.md and contracts
4. Complete Stage 6 → verify validator validates CHANGELOG and presets
5. Validator reports FAIL → verify 3-option menu appears

**Hook System Integration:**
1. Complete Stage 0, pause
2. Start new conversation, run /continue → verify UserPromptSubmit loads context
3. Verify handoff content and contracts displayed
4. Skip git commit (manually) → verify Stop hook blocks
5. Trigger context compaction → verify PreCompact preserves contracts

### Success Criteria

Phase 2 is COMPLETE when:

1. ✅ Checkpoint system creates/updates/deletes handoff files correctly
2. ✅ Handoff file format matches checkpoint-system.md specification exactly
3. ✅ context-resume parses handoffs and resumes workflow at exact checkpoint
4. ✅ PLUGINS.md state machine transitions correctly (💡 → 🚧 → ✅)
5. ✅ State machine prevents illegal transitions and enforces single 🚧 constraint
6. ✅ Git commits created atomically for Stages 0, 1, 6 with standardized format
7. ✅ Interactive decision menus (inline numbered lists) appear at all checkpoints
8. ✅ Decision menus generate context-aware options based on stage/status
9. ✅ Keyword shortcuts work ("continue", "pause", "review")
10. ✅ Stage 0 research executes with Context7 JUCE docs lookup
11. ✅ Stage 1 **CONTRACT ENFORCEMENT BLOCKS** without parameter-spec.md and architecture.md
12. ✅ Stage 1 complexity scoring calculates correctly with phasing for score ≥3
13. ✅ Stage 6 creates factory presets, CHANGELOG.md, and updates to ✅ Working
14. ✅ Stages 2-5 stubs allow workflow continuation with clear Phase 3 notices
15. ✅ Validator subagent validates Stages 0, 1, 6 and returns JSON reports
16. ✅ UserPromptSubmit hook auto-loads context on /continue
17. ✅ Stop hook enforces stage commits before session end
18. ✅ PreCompact hook preserves contracts during context compaction
19. ✅ All hooks use conditional execution (exit 0 when not relevant)
20. ✅ TestWorkflowPhase2 completes full workflow successfully
21. ✅ All automated tests pass
22. ✅ All manual verification items checked

---

## Potential Issues & Mitigations

**Issue: Handoff format drift between plugin-workflow and context-resume**
- Mitigation: Use shared template file, validate YAML on write
- Recovery: context-resume shows parse error, suggests handoff format fix

**Issue: PLUGINS.md concurrent modification corruption**
- Mitigation: State machine enforces single 🚧 plugin
- Recovery: Manual PLUGINS.md edit, restore from git if corrupted

**Issue: Git commit failures (no git, permissions, conflicts)**
- Mitigation: Check git availability at workflow start
- Recovery: Display error, suggest manual commit, workflow continues

**Issue: Contract enforcement too strict (users want to "just try it")**
- Mitigation: Clear error explains WHY blocked, suggests unblock path
- User override: NOT PROVIDED - contract enforcement is non-negotiable per ROADMAP.md

**Issue: Complexity scoring inaccurate for novel plugin types**
- Mitigation: Display score breakdown, offer "Adjust complexity" in decision menu
- Recovery: User adjusts, plan.md regenerated

**Issue: Context7 lookup fails (network, service unavailable)**
- Mitigation: Fallback to web search for JUCE documentation
- Recovery: Display warning, continue with available information

**Issue: Validator subagent returns malformed JSON**
- Mitigation: Robust JSON parsing with fallback to text analysis
- Recovery: Log error, skip validation with warning, continue workflow

**Issue: Hook scripts fail (permissions, syntax errors)**
- Mitigation: All hooks exit 0 when not relevant (graceful skip)
- Recovery: Hook failure logs error but doesn't block workflow

**Issue: Decision menus feel repetitive across stages**
- Mitigation: Vary options based on context, discovery markers teach features
- Enhancement: Learn user preferences over time (future)

**Issue: Handoff file not found on /continue**
- Mitigation: Search both .ideas/ and plugin root, find most recent
- Recovery: context-resume shows "No handoff found", suggests create new plugin

**Issue: Stage 6 pluginval not found**
- Mitigation: Check pluginval availability before running
- Recovery: Skip validation with clear message, add manual test checklist

---

## Notes for Next Phase

**Phase 3 will implement:**
- foundation-agent subagent (replaces Stage 2 stub)
- shell-agent subagent (replaces Stage 3 stub)
- dsp-agent subagent (replaces Stage 4 stub)
- gui-agent subagent (replaces Stage 5 stub)
- Dispatcher integration via Task tool
- JSON report parsing from subagents
- SubagentStop hook for contract validation
- build-automation skill integration
- plugin-testing skill auto-invocation

**Phase 3 prerequisites from Phase 2:**
- Checkpoint system (handoff files) ✓
- State management (PLUGINS.md, git commits) ✓
- Stage dispatch pattern established ✓
- Decision menu system functional ✓
- Contract enforcement working ✓
- Validator subagent implemented ✓
- Hook system operational ✓
- Stage 6 validation complete ✓

**Interface requirements for Phase 3 subagents:**
- Input: pluginName, stage, contracts (parameter-spec, architecture, plan)
- Output: JSON report with structure:
  ```json
  {
    "agent": "agent-name",
    "status": "success" | "failure",
    "outputs": {...},
    "issues": [],
    "ready_for_next_stage": true
  }
  ```
- Pattern documented in: architecture/07-communication-architecture.md lines 82-101

**Testing approach for Phase 3:**
- Replace stubs one at a time
- Test integration with checkpoint system
- Verify JSON report parsing
- Test failure scenarios (build errors, validation failures)
- Ensure decision menus work with real implementation
- Test SubagentStop hook validation

**Critical Phase 2 → Phase 3 handoff:**
- Stage 2 stub expects: CMakeLists.txt, PluginProcessor.{h,cpp}, PluginEditor.{h,cpp}
- Stage 3 stub expects: APVTS with parameters, basic processBlock
- Stage 4 stub expects: DSP implementation, phased if complexity ≥3
- Stage 5 stub expects: WebView UI, parameter bindings
- All stubs have handoff notes explaining Phase 3 requirements

---

## Meta

**Document Version:** 2.0.0
**Phase:** 2 (Workflow Engine)
**Sub-Phases:** 3 (2a: Checkpoint & State, 2b: Stages 0 & 1, 2c: Stage 6 & Validator)
**Total Tasks:** 13
**Estimated Duration:** 10-12 hours
**Dependencies:** Phase 0, Phase 1
**Enables:** Phase 3 (Implementation Subagents)

**Key Files Created:**
- `.claude/skills/plugin-workflow/assets/continue-here-template.md` (handoff template)
- `.claude/agents/validator.md` (validator subagent)
- `.claude/hooks/UserPromptSubmit.sh` (context injection hook)
- `.claude/hooks/Stop.sh` (stage enforcement hook)
- `.claude/hooks/PreCompact.sh` (contract preservation hook)
- `plugins/[Name]/.ideas/research.md` (Stage 0 output)
- `plugins/[Name]/.ideas/plan.md` (Stage 1 output)
- `plugins/[Name]/CHANGELOG.md` (Stage 6 output)
- `plugins/[Name]/Presets/` (Stage 6 factory presets)
- Test scripts: 14 scripts in `scripts/`

**Key Files Modified:**
- `.claude/skills/plugin-workflow/SKILL.md` (complete Stages 0,1,6 implementation)
- `.claude/skills/context-resume/SKILL.md` (handoff parsing and resumption)
- `PLUGINS.md` (state machine updates throughout workflow)

**Architecture Documents Referenced:** 14 (00, 01, 02, 03, 04, 05, 06, 08, 09, 10, 13, 14, 16, 17)
**Procedure Files Referenced:** 20 (checkpoint-system, interactive-decision-system, plugin-workflow, context-resume, deep-research, build-automation, plugin-testing, plugin-lifecycle, juce-foundation, design-sync, troubleshooting-docs, implement, continue, dream, test, doc-fix, install-plugin, show-standalone, improve, troubleshooter)

**Created:** 2025-11-10
**Status:** Ready for implementation
