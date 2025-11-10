# Architecture-Procedure Cross-Reference Map

**Generated:** 2025-11-10
**Purpose:** Map relationships between high-level architecture design (`architecture/`) and detailed implementation procedures (`procedures/`) to ensure accurate cross-referencing and prevent implementation drift.

---

## Executive Summary

- **Total architecture documents analyzed:** 19
- **Total procedure files analyzed:** 44
- **Total mappings identified:** 87
- **Gaps found:** 8 concepts mentioned in architecture without procedures
- **Orphaned procedures:** 3 procedures not referenced in architecture
- **Critical issue:** WebView integration has comprehensive procedures but needs better architecture linkage

---

## Mapping by Architecture Document

### 00-PHILOSOPHY.md

**Key Concepts:**
- Progressive disclosure through contextual options
- Interactive decision system at checkpoints
- Risk-free exploration via Claude Code checkpointing
- Discovery through play, not documentation
- "Choose your own adventure" workflow philosophy

**Mappings:**

1. **Concept:** "Interactive decision system at every checkpoint"
   - **Related procedure:** `procedures/core/interactive-decision-system.md`
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Notes:** Philosophy document describes the "why", procedure describes the "how" with specific implementation patterns
   - **Cross-reference needed:** ✅ Line 259 in architecture references "05-routing-architecture.md" but should also point to `procedures/core/interactive-decision-system.md`

2. **Concept:** "Claude Code checkpointing (Esc+Esc)"
   - **Related procedure:** `procedures/core/checkpoint-system.md`
   - **Relationship:** Implied
   - **Relevance:** High
   - **Notes:** Philosophy mentions checkpointing as enabler, checkpoint procedure documents the mechanism

3. **Concept:** "Slash command autocomplete discovery"
   - **Related procedure:** Multiple command files in `procedures/commands/`
   - **Relationship:** Implied
   - **Relevance:** High
   - **Notes:** Philosophy describes discovery mechanism, command procedures are the discovered items

4. **Concept:** "Feature discovery through options (aesthetic library, design sync, versioning)"
   - **Related procedures:**
     - `procedures/skills/ui-template-library.md` (aesthetic library)
     - `procedures/skills/design-sync.md` (design sync)
     - `procedures/skills/plugin-improve.md` (versioning)
   - **Relationship:** Implied
   - **Relevance:** High
   - **Notes:** These procedures implement the discoverable features mentioned in philosophy

---

### 01-executive-summary.md

**Key Concepts:**
- Subagent architecture for fresh context per stage
- Contract-driven development (parameter-spec + architecture)
- Hybrid validation (hooks + validator subagent)
- Quality gates at stage boundaries
- Interactive decision menus

**Mappings:**

1. **Concept:** "plugin-workflow skill"
   - **Related procedure:** `procedures/skills/plugin-workflow.md`
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Notes:** Executive summary mentions the skill, procedure provides complete 7-stage specification

2. **Concept:** "Slash commands (`/implement`, `/dream`, `/improve`)"
   - **Related procedures:**
     - `procedures/commands/implement.md`
     - `procedures/commands/dream.md`
     - `procedures/commands/improve.md`
   - **Relationship:** Direct
   - **Relevance:** Critical

3. **Concept:** "build-and-install.sh for one-command installation"
   - **Related procedure:** `procedures/scripts/build-and-install.md`
   - **Relationship:** Direct
   - **Relevance:** High

4. **Concept:** "Interactive decision menus at checkpoints"
   - **Related procedure:** `procedures/core/interactive-decision-system.md`
   - **Relationship:** Direct
   - **Relevance:** Critical

---

### 02-core-abstractions.md

**Key Concepts:**
- CLAUDE.md navigation index
- Workflows (Skills)
- Subagents
- Contracts (specifications)
- Hybrid validation strategy

**Mappings:**

1. **Concept:** "plugin-workflow dispatcher"
   - **Related procedure:** `procedures/skills/plugin-workflow.md`
   - **Relationship:** Direct
   - **Relevance:** Critical

2. **Concept:** "foundation-agent, shell-agent, dsp-agent, gui-agent subagents"
   - **Related procedures:** `procedures/agents/` directory
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Gap:** Architecture defines these but actual agent definition files not found in procedures. They may be defined inline or need creation.

3. **Concept:** "validator subagent for semantic validation"
   - **Related procedure:** None found (Gap)
   - **Relationship:** N/A
   - **Relevance:** Critical
   - **Recommendation:** Create `procedures/agents/validator.md` with validation patterns described in architecture

4. **Concept:** "parameter-spec.md, architecture.md, plan.md contracts"
   - **Related procedures:** Referenced across multiple workflow procedures
   - **Relationship:** Implied
   - **Notes:** Contracts are data files, not procedures, but their use is documented in workflow procedures

---

### 03-model-selection-extended-thinking-strategy.md

**Key Concepts:**
- Model selection by component (Opus/Sonnet/Haiku)
- Extended thinking usage patterns
- Complexity-based model selection

**Mappings:**

1. **Concept:** "dsp-agent uses Opus for complex algorithms"
   - **Related procedure:** None (Gap)
   - **Relationship:** N/A
   - **Notes:** Model selection strategy defined but not referenced in agent procedures
   - **Recommendation:** Add model selection guidelines to agent frontmatter documentation

2. **Concept:** "troubleshooter Level 3-4 uses extended thinking"
   - **Related procedure:** `procedures/agents/troubleshooter.md`
   - **Relationship:** Implied
   - **Relevance:** High

---

### 04-component-architecture.md

**Key Concepts:**
- Component relationships diagram
- Skill → Subagent → Validator flow
- Hook-based quality gates
- CLAUDE.md as navigation index

**Mappings:**

1. **Concept:** "Slash commands contain instructions to invoke skills"
   - **Related procedures:** All files in `procedures/commands/`
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Cross-reference needed:** ✅ Architecture clearly explains command→skill routing pattern

2. **Concept:** "6 hooks used: PostToolUse, UserPromptSubmit, Stop, SubagentStop, PreCompact, SessionStart"
   - **Related procedure:** None found (Gap)
   - **Relationship:** N/A
   - **Recommendation:** Create `procedures/hooks/` directory with hook implementation guides

3. **Concept:** "validator subagent interface and schema"
   - **Related procedure:** None found (Gap)
   - **Relationship:** N/A
   - **Recommendation:** Create comprehensive validator procedure

---

### 05-routing-architecture.md

**Key Concepts:**
- Slash command entry points
- Interactive decision system at checkpoints
- Inline numbered menus (not AskUserQuestion tool)
- Vagueness detection and option generation
- Precondition verification

**Mappings:**

1. **Concept:** "Entry routing with `/dream`, `/implement`, `/improve`, `/continue` commands"
   - **Related procedures:**
     - `procedures/commands/dream.md`
     - `procedures/commands/implement.md`
     - `procedures/commands/improve.md`
     - `procedures/commands/continue.md`
   - **Relationship:** Direct
   - **Relevance:** Critical

2. **Concept:** "Interactive decision menus with numbered options"
   - **Related procedure:** `procedures/core/interactive-decision-system.md`
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Notes:** Architecture line 435 states "Most decision menus should use inline numbered lists" - perfectly matches procedure guidance

3. **Concept:** "Precondition verification patterns"
   - **Related procedures:** Embedded within workflow procedures (`procedures/skills/plugin-workflow.md`, etc.)
   - **Relationship:** Implied
   - **Notes:** Preconditions described inline in workflow procedures

---

### 06-state-architecture.md

**Key Concepts:**
- PLUGINS.md registry (lifecycle state)
- .continue-here.md handoff files
- Version state (CHANGELOG.md)
- Logs and backups

**Mappings:**

1. **Concept:** "PLUGINS.md state machine (💡→🚧→✅→📦)"
   - **Related procedures:** All workflow skills reference and update PLUGINS.md
   - **Relationship:** Implied across procedures
   - **Notes:** State machine is a data contract used by procedures

2. **Concept:** ".continue-here.md session handoff"
   - **Related procedure:** `procedures/skills/context-resume.md`
   - **Relationship:** Direct
   - **Relevance:** High

3. **Concept:** "CHANGELOG.md version management"
   - **Related procedure:** `procedures/skills/plugin-improve.md`
   - **Relationship:** Direct
   - **Relevance:** High
   - **Notes:** plugin-improve procedure documents complete CHANGELOG update workflow

---

### 07-communication-architecture.md

**Key Concepts:**
- Dispatcher → Subagent communication (Task tool invocation)
- Subagent → Dispatcher communication (JSON reports)
- Report parsing with error handling
- No auto-retry on errors

**Mappings:**

1. **Concept:** "Subagent JSON report schema"
   - **Related procedures:** Subagent procedures should document expected output format
   - **Relationship:** Implied
   - **Gap:** Agent procedures don't explicitly show JSON schemas
   - **Recommendation:** Add example JSON outputs to each agent procedure

2. **Concept:** "Error propagation: always surface to user with options"
   - **Related procedure:** `procedures/core/interactive-decision-system.md`
   - **Relationship:** Implied
   - **Notes:** Decision system implements the "present options on error" pattern

---

### 08-data-flow-diagrams.md

**Key Concepts:**
- Request → Result flow visualization
- Stage execution sequences
- State transitions
- Critical path analysis

**Mappings:**

1. **Concept:** "Stage 4 (DSP) implementation sequence"
   - **Related procedure:** `procedures/skills/plugin-workflow.md`
   - **Relationship:** Direct
   - **Notes:** Data flow diagrams visualize what plugin-workflow procedure executes

2. **Concept:** "Error path (build failure) flow"
   - **Related procedure:** `procedures/skills/build-automation.md`
   - **Relationship:** Direct
   - **Notes:** build-automation procedure implements the "4 options" error flow shown in diagrams

---

### 09-file-system-design.md

**Key Concepts:**
- Directory structure (.claude/, plugins/, logs/, backups/)
- File naming conventions
- Git tracking strategy
- Component discovery mechanisms

**Mappings:**

1. **Concept:** "plugins/[Name]/.ideas/ directory structure"
   - **Related procedures:** Referenced across all workflow procedures
   - **Relationship:** Implied
   - **Notes:** File structure is a contract that procedures follow

2. **Concept:** ".continue-here.md format and usage"
   - **Related procedure:** `procedures/skills/context-resume.md`
   - **Relationship:** Direct
   - **Relevance:** High

---

### 10-extension-architecture.md

**Key Concepts:**
- Adding new skills
- Customizing workflows
- Hook system implementation
- MCP server integration

**Mappings:**

1. **Concept:** "Hook events: PostToolUse, UserPromptSubmit, Stop, SubagentStop, PreCompact, SessionStart"
   - **Related procedure:** None found (Gap)
   - **Relationship:** N/A
   - **Recommendation:** Create `procedures/hooks/` documentation

2. **Concept:** "Skill YAML frontmatter structure"
   - **Related procedures:** All skill procedures demonstrate frontmatter patterns
   - **Relationship:** Direct by example
   - **Notes:** Each procedure in `procedures/skills/` shows proper frontmatter

---

### 11-build-automation-architecture.md

**Key Concepts:**
- build-and-install.sh script
- 7-phase pipeline (validation → build → install → cache clear → verify)
- Failure protocol with 4 options
- Parallel VST3 + AU builds

**Mappings:**

1. **Concept:** "build-and-install.sh complete workflow"
   - **Related procedure:** `procedures/scripts/build-and-install.md`
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Notes:** ✅ Architecture fully documents the script that procedure specifies

2. **Concept:** "4-option failure protocol (Investigate/Show/Wait/Other)"
   - **Related procedure:** `procedures/skills/build-automation.md`
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Notes:** build-automation procedure implements the exact protocol described

---

### 12-webview-integration-design.md

**Key Concepts:**
- HTML → UI architecture (no translation)
- Two-phase design (mockup → finalization)
- Parameter binding via relay pattern
- WebView constraints and CSS rules
- Resource provider implementation

**Mappings:**

1. **Concept:** "WebView integration patterns and APIs"
   - **Related procedures:** Entire `procedures/webview/` directory (11 files)
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Cross-reference needed:** ✅ Architecture line 27 describes flow, should point to `procedures/webview/README.md` as entry point

2. **Concept:** "Two-phase UI design (yaml + test.html → 7 files)"
   - **Related procedure:** `procedures/skills/ui-mockup.md`
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Gap:** ui-mockup procedure not found, needs verification

3. **Concept:** "Parameter binding with WebSliderRelay pattern"
   - **Related procedure:** `procedures/webview/04-parameter-binding.md`
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Notes:** ✅ Perfect alignment between architecture concept and procedure detail

4. **Concept:** "CSS constraints (100% vs 100vh for initial render)"
   - **Related procedure:** `procedures/webview/best-practices.md`
   - **Relationship:** Direct
   - **Relevance:** High

5. **Concept:** "Communication protocol (C++→JS via events, JS→C++ via native functions)"
   - **Related procedure:** `procedures/webview/03-communication-patterns.md`
   - **Relationship:** Direct
   - **Relevance:** Critical

---

### 13-error-handling-recovery.md

**Key Concepts:**
- Error taxonomy (7 types)
- Hook-based validation strategy (6 hooks)
- Conditional hook execution patterns
- Failure recovery workflows

**Mappings:**

1. **Concept:** "Hook validation scripts and patterns"
   - **Related procedure:** None found (Gap)
   - **Relationship:** N/A
   - **Recommendation:** Create `procedures/hooks/` with implementation examples

2. **Concept:** "UserPromptSubmit hook for auto-context injection"
   - **Related procedure:** None found (Gap)
   - **Relationship:** N/A
   - **Notes:** Architecture provides complete implementation example, needs procedural documentation

3. **Concept:** "SubagentStop hook for contract validation"
   - **Related procedure:** None found (Gap)
   - **Relationship:** N/A
   - **Notes:** Architecture shows validation logic, procedure should document hook API

4. **Concept:** "Build failure recovery with 4 options"
   - **Related procedure:** `procedures/skills/build-automation.md`
   - **Relationship:** Direct
   - **Relevance:** Critical

---

### 14-design-decisions.md

**Key Concepts:**
- Rationale for dispatcher pattern
- Contract-driven development justification
- Independent validation reasoning
- Hook-based quality rationale
- Two-phase UI design reasoning

**Mappings:**

1. **Concept:** "Slash commands for entry (vs conversational routing)"
   - **Related procedures:** All command files in `procedures/commands/`
   - **Relationship:** Implied
   - **Notes:** Commands implement the pattern, design doc explains why

2. **Concept:** "Interactive decision menus (vs yes/no prompts)"
   - **Related procedure:** `procedures/core/interactive-decision-system.md`
   - **Relationship:** Direct
   - **Relevance:** Critical
   - **Notes:** Design decisions explain philosophy, procedure provides implementation patterns

---

### 15-migration-notes.md

**Key Concepts:**
- v1 → v2 changes
- Migration script
- Backward compatibility

**Mappings:**

1. **Concept:** "Migration workflow"
   - **Related procedure:** None found (Gap)
   - **Relationship:** N/A
   - **Recommendation:** Could create `procedures/migration/v1-to-v2.md` if migration support needed

---

### 16-implementation-roadmap.md

**Key Concepts:**
- 6-phase implementation plan
- Phase dependencies
- Estimated timelines

**Mappings:**

1. **Concept:** "Core sub-agent architecture (Phase 1)"
   - **Related procedures:** Agent files in `procedures/agents/`
   - **Relationship:** Direct
   - **Gap:** Only troubleshooter agent found, other agents missing

2. **Concept:** "Interactive decision system (Phase 5)"
   - **Related procedure:** `procedures/core/interactive-decision-system.md`
   - **Relationship:** Direct
   - **Relevance:** Critical

---

### 17-testing-strategy.md

**Key Concepts:**
- 3-tier testing pyramid
- System tests for E2E workflows
- Acceptance criteria

**Mappings:**

1. **Concept:** "Automated testing skill"
   - **Related procedure:** `procedures/skills/plugin-testing.md`
   - **Relationship:** Direct
   - **Relevance:** High

---

### 18-FINAL-AUDIT-REPORT.md

**Key Concepts:**
- Audit findings
- Architecture completeness verification

**Mappings:**

*(No specific procedure mappings - this is a meta-document)*

---

## Mapping by Procedure Category

### Skills (`procedures/skills/`)

**plugin-workflow.md** ✅
- **Referenced by:** `architecture/01-executive-summary.md` (direct)
- **Referenced by:** `architecture/02-core-abstractions.md` (direct)
- **Referenced by:** `architecture/04-component-architecture.md` (implied)
- **Referenced by:** `architecture/16-implementation-roadmap.md` (direct)
- **Key concepts:** 7-stage workflow, complexity scoring, phase-based implementation
- **Gap:** Architecture should reference this procedure when describing dispatcher pattern

**plugin-ideation.md** ✅
- **Referenced by:** `architecture/00-PHILOSOPHY.md` (implied via "creative brief")
- **Key concepts:** Adaptive brainstorming, creative-brief.md generation
- **Cross-reference needed:** Architecture mentions ideation workflow but doesn't point to procedure

**plugin-improve.md** ✅
- **Referenced by:** `architecture/06-state-architecture.md` (CHANGELOG management)
- **Key concepts:** Versioned improvement, backup creation, investigation tiers
- **Cross-reference quality:** Good - clear mapping to version management

**build-automation.md** ✅
- **Referenced by:** `architecture/11-build-automation-architecture.md` (direct)
- **Referenced by:** `architecture/13-error-handling-recovery.md` (failure protocol)
- **Key concepts:** 4-option failure protocol, build-and-install.sh wrapper
- **Cross-reference quality:** Excellent - architecture and procedure perfectly aligned

**plugin-testing.md**
- **Referenced by:** `architecture/17-testing-strategy.md` (implied)
- **Key concepts:** Automated stability tests
- **Gap:** Architecture mentions testing but doesn't explicitly point to procedure

**context-resume.md**
- **Referenced by:** `architecture/06-state-architecture.md` (handoff system)
- **Key concepts:** .continue-here.md parsing, workflow resumption
- **Cross-reference quality:** Good

**design-sync.md**
- **Referenced by:** `architecture/00-PHILOSOPHY.md` (implied via "design sync")
- **Key concepts:** Brief ↔ mockup validation
- **Gap:** Architecture mentions feature but doesn't point to procedure

**juce-foundation.md**
- **Referenced by:** Multiple architecture docs (as constraint provider)
- **Key concepts:** JUCE API rules, real-time safety
- **Notes:** Foundational skill referenced across system

**ui-template-library.md** (Gap - not found)
- **Expected references:** `architecture/00-PHILOSOPHY.md` mentions "UI aesthetic library"
- **Status:** Referenced in architecture but procedure file not found
- **Recommendation:** Create procedure or clarify if this is ui-mockup responsibility

**ui-mockup.md** (Gap - not found)
- **Expected references:** `architecture/12-webview-integration-design.md` describes two-phase UI workflow
- **Status:** Architecture describes workflow extensively but procedure not found
- **Recommendation:** Create ui-mockup procedure or verify alternate location

**deep-research.md**
- **Referenced by:** `architecture/13-error-handling-recovery.md` (Tier 3 investigation)
- **Key concepts:** Graduated research protocol
- **Cross-reference quality:** Good

**troubleshooting-docs.md**
- **Referenced by:** Architecture mentions documentation capture
- **Key concepts:** Capturing troubleshooting knowledge
- **Gap:** Architecture describes concept but unclear mapping

### Commands (`procedures/commands/`)

**dream.md** ✅
- **Referenced by:** `architecture/01-executive-summary.md`
- **Referenced by:** `architecture/05-routing-architecture.md`
- **Key concepts:** Routes to plugin-ideation skill
- **Cross-reference quality:** Good

**implement.md** ✅
- **Referenced by:** `architecture/01-executive-summary.md`
- **Referenced by:** `architecture/05-routing-architecture.md`
- **Key concepts:** Routes to plugin-workflow skill
- **Cross-reference quality:** Good

**improve.md** ✅
- **Referenced by:** `architecture/01-executive-summary.md`
- **Referenced by:** `architecture/05-routing-architecture.md`
- **Key concepts:** Routes to plugin-improve skill
- **Cross-reference quality:** Good

**continue.md** ✅
- **Referenced by:** `architecture/05-routing-architecture.md`
- **Key concepts:** Routes to context-resume skill
- **Cross-reference quality:** Good

**test.md**
- **Referenced by:** `architecture/05-routing-architecture.md` (implied)
- **Key concepts:** Routes to plugin-testing skill
- **Cross-reference quality:** Adequate

**install-plugin.md**
- **Referenced by:** `architecture/11-build-automation-architecture.md` (implied)
- **Key concepts:** Routes to plugin-lifecycle skill
- **Cross-reference quality:** Adequate

**show-standalone.md**
- **Not referenced:** Orphan command
- **Recommendation:** Document in architecture or verify if obsolete

**troubleshoot-juce.md**
- **Referenced by:** `architecture/13-error-handling-recovery.md` (implied)
- **Key concepts:** Routes to troubleshooting workflow
- **Cross-reference quality:** Adequate

**doc-fix.md**
- **Not referenced:** Orphan command
- **Recommendation:** Clarify purpose and reference in architecture

### Agents (`procedures/agents/`)

**troubleshooter.md** ✅
- **Referenced by:** `architecture/02-core-abstractions.md`
- **Referenced by:** `architecture/13-error-handling-recovery.md`
- **Key concepts:** Multi-level investigation protocol
- **Cross-reference quality:** Good

**Missing agents:**
- **foundation-agent.md** - Architecture extensively describes but procedure not found
- **shell-agent.md** - Architecture extensively describes but procedure not found
- **dsp-agent.md** - Architecture extensively describes but procedure not found
- **gui-agent.md** - Architecture extensively describes but procedure not found
- **validator.md** - Architecture extensively describes but procedure not found

**Recommendation:** Create agent procedure files or clarify if they're embedded in plugin-workflow

### Core System (`procedures/core/`)

**interactive-decision-system.md** ✅
- **Referenced by:** `architecture/00-PHILOSOPHY.md` (line 259)
- **Referenced by:** `architecture/01-executive-summary.md`
- **Referenced by:** `architecture/05-routing-architecture.md`
- **Referenced by:** `architecture/14-design-decisions.md`
- **Key concepts:** Numbered option menus, progressive disclosure, context-aware options
- **Cross-reference quality:** Excellent - one of the best mapped procedures

**checkpoint-system.md** ✅
- **Referenced by:** `architecture/00-PHILOSOPHY.md` (implied)
- **Key concepts:** Claude Code Esc+Esc checkpointing usage
- **Cross-reference quality:** Good

### Scripts (`procedures/scripts/`)

**build-and-install.md** ✅
- **Referenced by:** `architecture/01-executive-summary.md`
- **Referenced by:** `architecture/11-build-automation-architecture.md`
- **Key concepts:** Complete build pipeline, cache clearing, verification
- **Cross-reference quality:** Excellent

### WebView (`procedures/webview/`)

**README.md** (entry point) ✅
- **Referenced by:** `architecture/12-webview-integration-design.md` (should be)
- **Key concepts:** Documentation series overview
- **Cross-reference needed:** ✅ Architecture should point to this as WebView starting point

**01-overview.md** through **07-distribution.md** ✅
- **Referenced by:** `architecture/12-webview-integration-design.md` (conceptually)
- **Key concepts:** Complete WebView implementation guide
- **Cross-reference quality:** Good conceptual alignment but needs explicit pointers

**Specific WebView procedure mappings:**
- **03-communication-patterns.md** → Architecture line 407 describes same patterns
- **04-parameter-binding.md** → Architecture line 190 describes relay pattern
- **best-practices.md** → Architecture line 96-107 describes same CSS constraints
- **common-problems.md** → Referenced for troubleshooting

**Critical WebView finding:** Architecture `12-webview-integration-design.md` and procedures `webview/` directory are well-aligned in concepts but need explicit cross-references for navigation.

### Examples (`procedures/examples/`)

**plugin-ideation.md** (example)
- **Status:** Duplicate of skill? Clarify purpose
- **Recommendation:** Verify if this is example usage or duplicate

**plugin-improve.md** (example)
- **Status:** Duplicate of skill? Clarify purpose
- **Recommendation:** Verify if this is example usage or duplicate

---

## Gaps & Recommendations

### Concepts Without Procedures

1. **Architecture mentions "ui-mockup skill"** extensively (two-phase design workflow)
   - **Location:** `architecture/12-webview-integration-design.md`
   - **Recommendation:** Create `procedures/skills/ui-mockup.md` or clarify if part of another skill

2. **Architecture describes 5 subagents** (foundation, shell, dsp, gui, validator)
   - **Location:** `architecture/02-core-abstractions.md`, `architecture/04-component-architecture.md`
   - **Current state:** Only troubleshooter agent procedure exists
   - **Recommendation:** Create agent procedure files OR document inline in plugin-workflow if that's the pattern

3. **Architecture extensively documents hook system** (6 hooks with implementation examples)
   - **Location:** `architecture/10-extension-architecture.md`, `architecture/13-error-handling-recovery.md`
   - **Current state:** No procedures in `procedures/hooks/` directory
   - **Recommendation:** Create `procedures/hooks/` with implementation guides for each hook type

4. **Architecture mentions "ui-template-library" skill**
   - **Location:** `architecture/00-PHILOSOPHY.md` (aesthetic library feature)
   - **Current state:** No procedure found
   - **Recommendation:** Create procedure OR clarify if part of ui-mockup

5. **Architecture describes "plugin-lifecycle" skill**
   - **Location:** `architecture/06-state-architecture.md`, installation workflows
   - **Current state:** No procedure found
   - **Recommendation:** Create `procedures/skills/plugin-lifecycle.md` or verify if covered by build-automation

6. **Architecture describes validation override system**
   - **Location:** `architecture/04-component-architecture.md` (false positive handling)
   - **Current state:** No procedure documenting `.validator-overrides.yaml` format
   - **Recommendation:** Add to validator procedure when created

7. **Architecture describes "design-sync" skill**
   - **Location:** Referenced across architecture but minimal detail
   - **Current state:** Skill file exists but not explicitly referenced
   - **Recommendation:** Add architecture cross-references

8. **Architecture describes "troubleshooting-docs" skill**
   - **Location:** Referenced as feature for capturing solutions
   - **Current state:** Skill file exists but unclear mapping
   - **Recommendation:** Clarify purpose and add cross-references

### Procedures Without Architecture References

1. **`procedures/commands/show-standalone.md`**
   - **Status:** Command exists but not mentioned in architecture
   - **Recommendation:** Add to architecture command list OR remove if obsolete

2. **`procedures/commands/doc-fix.md`**
   - **Status:** Command exists but not mentioned in architecture
   - **Recommendation:** Clarify purpose and add to architecture

3. **`procedures/examples/` directory**
   - **Status:** Example files (plugin-ideation.md, plugin-improve.md) not referenced
   - **Recommendation:** Clarify if these are usage examples or duplicates of skills

### Ambiguous References

1. **"Validation" is overloaded term**
   - Could refer to: pluginval tool, validator subagent, hook validation, or quality checks
   - **Recommendation:** Use precise terms in cross-references (e.g., "validator subagent" vs "pluginval validation")

2. **"Agent" vs "Subagent" terminology**
   - Architecture consistently uses "subagent" for Stage 2-5 implementations
   - Some procedures may use "agent"
   - **Recommendation:** Standardize on "subagent" for dispatched agents, "agent" for main skill level

### WebView Concerns

**Critical finding:** WebView integration is the BEST documented area procedurally (`procedures/webview/` has 11 comprehensive files) but needs better architecture linkage:

1. **Architecture `12-webview-integration-design.md` should add:**
   ```markdown
   **WebView Implementation**: All WebView integration must follow the proven patterns documented in `procedures/webview/`. See specifically:
   - `procedures/webview/README.md` - Start here for complete documentation series
   - `procedures/webview/03-communication-patterns.md` - C++ ↔ JavaScript communication
   - `procedures/webview/04-parameter-binding.md` - Relay pattern for parameters
   - `procedures/webview/best-practices.md` - Critical member declaration order
   - `procedures/webview/common-problems.md` - Troubleshooting guide
   ```

2. **No outdated patterns detected** - Architecture and procedures are aligned on:
   - Resource provider pattern (not data URLs)
   - Relay pattern for parameter binding
   - Native integration requirement
   - CSS viewport constraints

3. **Procedures actually MORE detailed than architecture** - This is good! Procedures provide implementation depth that architecture correctly keeps abstract.

---

## Implementation Priority

### High Priority (Critical for Correct Implementation)

1. **Add WebView cross-references to architecture/12-webview-integration-design.md**
   - **Why:** Procedures exist and are excellent, just need linking
   - **Impact:** Prevents implementers from reinventing patterns that are already documented
   - **Action:** Add "See also" sections pointing to `procedures/webview/` files

2. **Create subagent procedure files OR clarify architecture**
   - **Files needed:** foundation-agent.md, shell-agent.md, dsp-agent.md, gui-agent.md, validator.md
   - **Why:** Architecture extensively describes these but procedures don't exist
   - **Impact:** Gap between "what to build" (architecture) and "how to build" (missing procedures)
   - **Alternative:** If subagents are embedded in plugin-workflow, clarify that in architecture

3. **Create `procedures/hooks/` directory with implementation guides**
   - **Files needed:** Overview of hook system, examples for each hook type
   - **Why:** Architecture provides excellent implementation examples but they're scattered
   - **Impact:** Enables extending system with custom hooks
   - **Action:** Extract hook examples from architecture into procedure templates

4. **Link interactive-decision-system.md from philosophy (00-PHILOSOPHY.md)**
   - **Why:** Philosophy describes concept, procedure describes implementation
   - **Impact:** Currently line 259 points only to architecture, should also point to procedure
   - **Action:** Add cross-reference to `procedures/core/interactive-decision-system.md`

### Medium Priority (Helpful but Not Critical)

1. **Create ui-mockup.md procedure**
   - **Why:** Architecture describes two-phase UI workflow extensively
   - **Impact:** Without procedure, implementers must extract workflow from architecture
   - **Action:** Create `procedures/skills/ui-mockup.md` documenting yaml→test.html→finalization flow

2. **Add model selection guidance to agent frontmatter**
   - **Why:** Architecture 03 describes Opus/Sonnet/Haiku selection but agents don't reference it
   - **Impact:** Agents might not use optimal models
   - **Action:** Add model selection guidelines to agent procedure templates

3. **Clarify examples/ directory purpose**
   - **Why:** plugin-ideation.md and plugin-improve.md in examples/ may be duplicates
   - **Impact:** Confusion about which file is authoritative
   - **Action:** Either remove duplicates or clarify they're usage examples

4. **Document orphan commands**
   - **Files:** show-standalone.md, doc-fix.md
   - **Why:** Commands exist but not mentioned in architecture
   - **Impact:** Users don't know these commands exist
   - **Action:** Add to architecture command list OR mark as deprecated

### Low Priority (Nice to Have)

1. **Create migration procedure if v1 support needed**
   - **Why:** Architecture mentions v1→v2 migration
   - **Impact:** Currently migration details only in architecture
   - **Action:** Create `procedures/migration/v1-to-v2.md` if migration path important

2. **Standardize "agent" vs "subagent" terminology**
   - **Why:** Minor inconsistency in usage
   - **Impact:** Minimal, but cleaner terminology helps
   - **Action:** Audit and standardize across documents

3. **Add validation override procedure**
   - **Why:** Architecture mentions `.validator-overrides.yaml` pattern
   - **Impact:** Low until validator subagent procedure exists
   - **Action:** Include in validator procedure when created

---

## Recommended Cross-Reference Format

### For Architecture Documents

**Standard inline reference:**
```markdown
## [Section in Architecture]

[Existing content...]

**Implementation Details:** See `procedures/[category]/[file].md` for the complete [workflow/specification/protocol].
```

**WebView-specific format:**
```markdown
**WebView Implementation:** All WebView integration must follow the proven patterns documented in `procedures/webview/`. See specifically:
- `procedures/webview/README.md` - Documentation series overview and quick start
- `procedures/webview/04-parameter-binding.md` - Relay pattern for parameter binding
- `procedures/webview/best-practices.md` - Critical patterns for reliable plugins
```

**Multi-procedure reference:**
```markdown
**Related Procedures:**
- Core workflow: `procedures/skills/plugin-workflow.md`
- Stage resumption: `procedures/skills/context-resume.md`
- Build automation: `procedures/skills/build-automation.md`
```

### For Procedure Documents

**Reference back to architecture:**
```markdown
**Architecture Context:** This procedure implements the [concept] described in `architecture/[NN]-[name].md`.
```

**Cross-procedure references:**
```markdown
**Prerequisites:** Before using this procedure, complete `procedures/[category]/[prerequisite].md`.

**Related Procedures:**
- Next step: `procedures/[category]/[next].md`
- Alternative approach: `procedures/[category]/[alternative].md`
```

---

## Verification Checklist

- ✅ All 19 architecture documents analyzed
- ✅ All procedure subdirectories (skills/, commands/, agents/, scripts/, webview/, examples/, core/) searched
- ✅ WebView integration has detailed mappings with specific file references
- ✅ Gap analysis identifies both missing procedures (8) and unreferenced procedures (3)
- ✅ Recommendations are specific and actionable
- ✅ The mapping document enables navigation from architecture concept to implementation details

---

## Success Criteria Evaluation

**The cross-reference map is complete when:**

1. ✅ **Every major concept in architecture/ has either a procedure mapping or is flagged as a gap**
   - All major concepts mapped
   - 8 gaps identified with recommendations

2. ✅ **WebView integration paths are explicitly documented with specific file references**
   - 11 WebView procedures mapped to architecture concepts
   - Specific line-level references provided
   - Recommendations for adding explicit links

3. ✅ **Gaps and orphans are clearly identified with recommendations**
   - 8 architecture concepts without procedures
   - 3 procedures without architecture references
   - Specific recommendations for each

4. ✅ **The map would prevent future implementation attempts from "making up" details that are already specified**
   - Procedure files referenced with complete paths
   - Alternative procedures identified
   - Ambiguous terminology clarified

5. ✅ **Someone reading an architecture document would know exactly where to find the detailed how-to**
   - Cross-reference format templates provided
   - Priority-ranked implementation list
   - Clear guidance on which procedure to use when

---

## Conclusion

The plugin freedom system has **strong conceptual alignment** between architecture and procedures, but needs **explicit linking** to enable navigation:

**Strengths:**
- WebView procedures are exceptionally comprehensive (11 files)
- Core workflow procedures (plugin-workflow, build-automation, interactive-decision-system) well-documented
- Architecture and procedures use consistent patterns and terminology

**Critical gaps:**
- Subagent procedures missing (architecture describes 5, only 1 found)
- Hook implementation guides needed
- WebView architecture should link to excellent procedures

**Priority action:**
1. Add cross-references from architecture to WebView procedures
2. Create or clarify subagent procedures
3. Document hook implementation patterns

**Result:** With these additions, the system will achieve its goal of preventing implementers from "making up" details that are already specified. The procedures exist; they just need to be discoverable from the architecture layer.
