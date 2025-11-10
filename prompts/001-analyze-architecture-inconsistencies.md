<objective>
Thoroughly analyze the architecture documentation files to identify inconsistencies, contradictions, and potential misalignments that could cause implementation problems.
This analysis will help prevent architectural debt and ensure the system can be built cohesively according to the documented design.
</objective>

<context>
The architecture folder contains the complete system design documentation for a plugin freedom system.
These documents should form a coherent, consistent blueprint for implementation.
@architecture/
</context>

<analysis_requirements>
1. Cross-reference all architecture documents for:
   - Contradictory statements or conflicting design decisions
   - Terminology inconsistencies (same concept, different names)
   - Interface mismatches between components
   - Dependency conflicts or circular dependencies
   - Missing connections between documented components
   - Version or technology conflicts
   - Incompatible data flow patterns

2. Identify alignment issues:
   - Components referenced but never defined
   - Defined components never referenced elsewhere
   - State management conflicts across documents
   - Communication patterns that don't match component boundaries
   - Build processes incompatible with component architecture
   - Testing strategies that don't align with component structure

3. Evaluate implementation feasibility:
   - Over-engineered solutions for stated goals
   - Under-specified critical components
   - Performance implications of architectural choices
   - Security gaps in the design
   - Scalability bottlenecks in the proposed architecture

4. Check for completeness:
   - Missing error handling strategies for critical paths
   - Undefined migration paths between stated versions
   - Gaps in the implementation roadmap sequence
   - Unaddressed edge cases in core workflows
</analysis_requirements>

<output_format>
Create a comprehensive analysis report saved to: `./analyses/architecture-inconsistencies.md`

Structure the report as:
# Architecture Consistency Analysis

## Executive Summary
[High-level findings and critical issues]

## Critical Issues
[Issues that would block or significantly complicate implementation]

## Inconsistencies Found
### Cross-Document Conflicts
[Specific contradictions with file:line references]

### Terminology Misalignments
[Same concepts with different names across documents]

### Component Mismatches
[Interface or dependency conflicts]

## Implementation Risks
[Technical debt likely to emerge from current design]

## Missing Specifications
[Critical details not covered in any document]

## Recommendations
[Prioritized list of fixes needed before implementation]

## Reference Matrix
[Table showing which components are defined/referenced where]
</output_format>

<verification>
Before completing, verify:
- Every document in architecture/ has been examined
- All cross-references between documents have been validated
- Each identified issue includes specific file references
- Recommendations are actionable and prioritized
- The analysis provides clear guidance for fixing issues
</verification>

<success_criteria>
The analysis successfully identifies all architectural inconsistencies that would:
- Cause implementation conflicts or confusion
- Result in system integration failures
- Create maintenance or scalability problems
- Lead to security vulnerabilities
- Generate technical debt
</success_criteria>