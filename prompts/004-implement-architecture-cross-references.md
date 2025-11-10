# Implement Architecture Cross-References

<objective>
Update all architecture documents with inline cross-references to their corresponding procedure files, based on the mappings identified in `cross-reference-map.md`. This ensures that anyone reading the architecture will know exactly where to find detailed implementation specifications, preventing assumptions and outdated patterns (especially for WebView integration).
</objective>

<context>
You have a completed cross-reference map at `./cross-reference-map.md` that identifies all relationships between:
- Architecture documents (high-level design in `architecture/`)
- Procedure files (detailed implementations in `procedures/`)

This project is a JUCE plugin development system with WebView-based UIs. The WebView integration is particularly critical - the architecture must reference the proven, working patterns in `procedures/webview/` to prevent using outdated or incorrect approaches.

The architecture documents should remain focused on design decisions and system structure, but now include clear signposts to where the implementation details live.
</context>

<input>
Read and parse: `@cross-reference-map.md`

Extract:
1. All high-priority mappings (these must be implemented)
2. All WebView-related mappings (critical for correctness)
3. Medium-priority mappings (implement if clearly beneficial)
4. Recommended cross-reference format from the map
</input>

<requirements>
For each mapping in the cross-reference map:

1. **Locate the insertion point**: Find the relevant section in the architecture document where the cross-reference should be added

2. **Add contextual references**: Insert cross-references that feel natural and helpful, not forced. Use formats like:
   - "See `procedures/skills/plugin-workflow.md` for the complete 7-stage implementation workflow"
   - "This design is implemented in `procedures/skills/build-automation.md` (sections: Failure Protocol, Build Options)"
   - "**Implementation**: The detailed procedure is specified in `procedures/commands/implement.md`"

3. **WebView special handling**: For `architecture/12-webview-integration-design.md`, ensure:
   - Clear references to ALL relevant `procedures/webview/*.md` files
   - Specific section references (not just file-level pointers)
   - Warnings about following proven patterns: "All WebView integration must follow the patterns in `procedures/webview/`. Do not use generic JUCE WebView examples as they may be outdated."

4. **Preserve architecture content**: Don't change the existing design content, only add cross-references

5. **Use consistent formatting**: Follow the recommended format from cross-reference-map.md

6. **Add a "Procedures Reference" section**: At the end of each architecture document that has mappings, add:
   ```markdown
   ---

   ## Related Procedures

   This architecture is implemented through:
   - `procedures/[category]/[file].md` - [Brief description]
   - `procedures/[category]/[file].md` - [Brief description]
   ```
</requirements>

<implementation>
Process architecture documents in this order:

**Phase 1: Critical WebView** (must be done correctly)
1. `architecture/12-webview-integration-design.md`
   - Add all `procedures/webview/` cross-references
   - Emphasize following proven patterns
   - Reference specific sections, not just files

**Phase 2: High-Priority Core System** (referenced in implementation roadmap)
2. `architecture/04-component-architecture.md` → skills/commands/agents
3. `architecture/11-build-automation-architecture.md` → `procedures/skills/build-automation.md`
4. `architecture/16-implementation-roadmap.md` → all relevant procedures
5. `architecture/17-testing-strategy.md` → `procedures/skills/plugin-testing.md`

**Phase 3: Workflow & Process** (referenced during development)
6. `architecture/06-state-architecture.md`
7. `architecture/07-communication-architecture.md`
8. `architecture/13-error-handling-recovery.md` → failure protocols
9. `architecture/09-file-system-design.md` → scripts/procedures

**Phase 4: Remaining Documents** (as identified in cross-reference-map.md)
10. All other architecture documents with mappings

For each file:
1. Read the current content
2. Identify insertion points from cross-reference-map.md
3. Add inline cross-references naturally
4. Add "Related Procedures" section at end
5. Verify the changes don't disrupt existing content
</implementation>

<constraints>
- Do NOT change any existing architecture content (design decisions, diagrams, explanations)
- Do NOT add procedures to the architecture documents (keep them separate)
- Do NOT create new procedure files (only add references to existing ones)
- Do NOT add cross-references where the relationship is weak or tangential
- Do NOT make the architecture documents feel cluttered with too many references
- DO ensure WebView references are comprehensive and specific
- DO prioritize clarity and usefulness over completeness
</constraints>

<output>
For each modified architecture document:

1. Show a summary of changes:
   ```
   Modified: architecture/12-webview-integration-design.md
   Added cross-references:
   - Line 45: Reference to procedures/webview/03-communication-patterns.md
   - Line 78: Reference to procedures/webview/04-parameter-binding.md
   - Line 120: Warning about following proven patterns
   - End: Related Procedures section with 6 webview procedure files
   ```

2. Update the file using the Edit tool (not Write - preserve existing content)

3. After all files are updated, create a summary: `./architecture-updates-summary.md`
   ```markdown
   # Architecture Cross-Reference Updates

   Completed: [timestamp]
   Based on: cross-reference-map.md

   ## Files Modified
   - architecture/[file].md - [N] cross-references added

   ## Cross-References Added
   - Total inline references: [N]
   - Total "Related Procedures" sections: [N]
   - WebView-specific references: [N]

   ## High-Priority Mappings Implemented
   [List all high-priority mappings from the map]

   ## Notes
   [Any relevant observations or recommendations]
   ```
</output>

<verification>
Before declaring complete, verify:
- [ ] All high-priority mappings from cross-reference-map.md have been implemented
- [ ] WebView integration (`architecture/12-webview-integration-design.md`) has comprehensive, specific references
- [ ] No existing architecture content was removed or significantly altered
- [ ] Cross-references are natural and helpful, not forced
- [ ] "Related Procedures" sections are consistently formatted
- [ ] All modified files still have valid markdown structure
- [ ] The summary document accurately reflects all changes
</verification>

<success_criteria>
The cross-references are successfully implemented when:
1. Someone reading an architecture document encounters clear pointers to implementation details at relevant moments
2. `architecture/12-webview-integration-design.md` specifically references all relevant `procedures/webview/*.md` files with section-level detail
3. Future implementation prompts will naturally discover the procedure files by reading the architecture
4. The architecture remains design-focused, not cluttered with implementation details
5. No "guessing" is required to find where a concept is actually specified
</success_criteria>
