# Analyze Architecture-Procedure Cross-Reference Mapping

<objective>
Create a comprehensive mapping document that identifies all relationships between the high-level architecture design (in `architecture/`) and the detailed implementation procedures (in `procedures/`). This mapping will be used to update architecture documents with accurate cross-references, ensuring that future implementation prompts follow the specific, tested patterns already documented rather than making assumptions or using outdated approaches.

This is critical for the WebView integration specifically - the proven patterns in `procedures/webview/` must be correctly referenced from the architecture to prevent using outdated or incorrect implementation approaches.
</objective>

<context>
This project (TÂCHES v2.0) is a plugin development system built on JUCE with WebView-based UIs. The documentation is split into two layers:

**Architecture layer** (`architecture/`):
- High-level design decisions
- System structure and component relationships
- WHAT needs to be built and WHY it's designed that way
- 18 architecture documents covering everything from philosophy to testing strategy

**Procedure layer** (`procedures/`):
- Detailed step-by-step workflows
- Executable specifications for skills, commands, and agents
- HOW to build it and WHEN to do what
- Subdirectories: skills/, commands/, agents/, scripts/, webview/, examples/, core/
- Note: `core/` contains system-level procedures like interactive decision menus and checkpointing

Currently these layers are disconnected. Architecture mentions concepts like "plugin-workflow skill", "build automation", "validation system", "WebView integration" but doesn't point to the specific procedure files where these are fully specified.

Read the complete directory structure first:
- `!find architecture/ -name "*.md" -type f | sort`
- `!find procedures/ -name "*.md" -type f | sort`
</context>

<analysis_requirements>
For each architecture document, thoroughly analyze:

1. **Concept Identification**: What systems, components, workflows, or processes does it describe?
   - Skills mentioned (e.g., "plugin-workflow", "build-automation")
   - Commands referenced (e.g., "/implement", "/dream")
   - Agents discussed (e.g., "foundation agent", "validator agent")
   - Workflows described (e.g., "7-stage workflow", "failure protocol")
   - WebView patterns and APIs

2. **Procedure Mapping**: For each identified concept, find the corresponding procedure file(s)
   - Search procedures/skills/ for skill definitions
   - Search procedures/commands/ for command specifications
   - Search procedures/agents/ for agent definitions
   - Search procedures/scripts/ for script documentation
   - Search procedures/webview/ for WebView implementation details
   - Search procedures/examples/ for usage examples
   - Search procedures/core/ for system-level procedures (decision menus, checkpointing, etc.)

3. **Gap Analysis**:
   - Concepts mentioned in architecture but no procedure exists
   - Procedures that exist but architecture doesn't reference
   - Ambiguous references that could map to multiple procedures

4. **WebView Special Attention**:
   - `architecture/12-webview-integration-design.md` must map to specific `procedures/webview/` files
   - Identify which WebView patterns, APIs, and workflows are referenced in architecture
   - Ensure all WebView implementation details have clear mappings
   - Flag any outdated patterns mentioned in architecture that conflict with procedures/webview/

5. **Relationship Strength**: Categorize each mapping as:
   - **Direct** - Architecture explicitly names the procedure
   - **Implied** - Architecture describes concept that clearly relates to procedure
   - **Tangential** - Weak relationship, may or may not be relevant
</analysis_requirements>

<methodology>
Use parallel tool calls to maximize efficiency:

1. Read multiple architecture files simultaneously
2. Read multiple procedure files simultaneously
3. Use grep to find specific term mentions across all files
4. Build the mapping incrementally, document by document

For each architecture document:
1. Read the full content
2. Extract key concepts, component names, workflow names
3. Search procedures/ for matching files
4. Document the relationships with specific line/section references
</methodology>

<output_format>
Create `./cross-reference-map.md` with this structure:

```markdown
# Architecture-Procedure Cross-Reference Map

Generated: [timestamp]

## Executive Summary
- Total architecture documents analyzed: [N]
- Total procedure files analyzed: [N]
- Total mappings identified: [N]
- Gaps found: [N]
- Orphaned procedures: [N]

---

## Mapping by Architecture Document

### 00-PHILOSOPHY.md
**Key Concepts**: [list]

**Mappings**:
- Concept: "progressive disclosure"
  - Related procedure: `procedures/skills/plugin-workflow.md`
  - Relationship: Implied
  - Relevance: High
  - Notes: Philosophy of discovery-through-use is implemented in plugin-workflow's stage boundary protocol

[Continue for all concepts in this document]

---

### 01-executive-summary.md
[Same structure]

---

[Continue for all 18 architecture documents]

---

## Mapping by Procedure Category

### Skills (procedures/skills/)

**plugin-workflow.md**
- Referenced by: `architecture/04-component-architecture.md` (implied)
- Referenced by: `architecture/16-implementation-roadmap.md` (direct)
- Key concepts: 7-stage workflow, complexity scoring, phase-based implementation
- Gap: Architecture mentions "plugin-workflow skill" but doesn't explain where to find detailed stages

[Continue for all skills]

### Commands (procedures/commands/)
[Same structure]

### Agents (procedures/agents/)
[Same structure]

### WebView (procedures/webview/)

**Critical Mappings**:
- `architecture/12-webview-integration-design.md` → `procedures/webview/` relationships
- List each WebView architecture concept and its specific procedure file
- Flag any conflicts between architecture and proven procedures

[Detailed WebView mapping]

---

## Gaps & Recommendations

### Concepts Without Procedures
1. [Architecture doc] mentions [concept] - no matching procedure found
   - Recommendation: Create procedure file OR clarify in architecture this is design-only

### Procedures Without Architecture References
1. `procedures/[category]/[file].md` - not referenced in any architecture document
   - Recommendation: Add reference in [suggested architecture doc]

### Ambiguous References
1. [Architecture doc] mentions [concept] - could refer to multiple procedures
   - Candidates: [list]
   - Recommendation: Clarify which is correct

### WebView Concerns
- Any outdated patterns in architecture that conflict with procedures/webview/
- Missing cross-references for WebView implementation

---

## Implementation Priority

Rank the cross-references to add by importance:

**High Priority** (Critical for correct implementation):
1. [Architecture doc] → [Procedure file] - [Why it's critical]

**Medium Priority** (Helpful but not critical):
[List]

**Low Priority** (Nice to have):
[List]

---

## Recommended Cross-Reference Format

For architecture documents, suggest this format for inline references:

```markdown
## [Section in Architecture]

[Existing content...]

**Implementation Details**: See `procedures/[category]/[file].md` for the complete [workflow/specification/protocol].
```

For WebView specifically:
```markdown
**WebView Implementation**: All WebView integration must follow the proven patterns documented in `procedures/webview/`. See specifically:
- `procedures/webview/03-communication-patterns.md` for [specific aspect]
- `procedures/webview/04-parameter-binding.md` for [specific aspect]
```
```
</output_format>

<verification>
Before declaring complete, verify:
- [ ] All 18 architecture documents have been analyzed
- [ ] All procedure subdirectories (skills/, commands/, agents/, scripts/, webview/, examples/) have been searched
- [ ] WebView integration has special attention with detailed mappings
- [ ] Gap analysis identifies both missing procedures and unreferenced procedures
- [ ] Recommendations are specific and actionable
- [ ] The mapping document would enable someone to navigate from architecture concept to implementation details
</verification>

<success_criteria>
The cross-reference map is complete when:
1. Every major concept in architecture/ has either a procedure mapping or is flagged as a gap
2. WebView integration paths are explicitly documented with specific file references
3. Gaps and orphans are clearly identified with recommendations
4. The map would prevent future implementation attempts from "making up" details that are already specified
5. Someone reading an architecture document would know exactly where to find the detailed how-to
</success_criteria>
