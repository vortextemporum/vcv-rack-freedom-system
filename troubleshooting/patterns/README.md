# Pattern Library

This directory contains reusable patterns extracted from the troubleshooting knowledge base.

## Files

### vcv-rack-critical-patterns.md (REQUIRED READING)

**Purpose:** Non-negotiable VCV Rack 2 patterns that MUST be followed in every module implementation.

**Injection point:** All subagents (foundation-agent, dsp-agent, widget-agent) read this file BEFORE generating any code.

**Contains:**
- Makefile configuration requirements
- plugin.json structure and slug formatting
- SVG panel setup requirements
- Port/param configuration rules (inputs/outputs/params)
- Threading safety rules (audio thread constraints)
- Common Rack SDK API patterns

**When to update:** When troubleshooting-docs skill detects a critical pattern (severity=critical, affects multiple modules or foundational stages).

**Update process:**
1. troubleshooting-docs skill asks: "This looks critical. Should I add to Required Reading file?"
2. If yes: Extract pattern from solution and add to vcv-rack-critical-patterns.md
3. Format: WRONG vs CORRECT with code examples
4. Cross-reference the original documentation file

### common-solutions.md

**Purpose:** Common patterns seen across multiple modules (3+ occurrences).

**Contains:**
- General solution strategies
- Links to specific documentation
- Pattern recognition guidance

**When to update:** When troubleshooting-docs skill finds 3+ similar issues.

## Usage

### For Subagents
All implementation subagents (Stages 1-3) automatically read `vcv-rack-critical-patterns.md` before code generation. This is enforced in their prompts with a "CRITICAL: Required Reading" section.

### For Human Developers
Review `vcv-rack-critical-patterns.md` before:
- Creating new modules
- Modifying Makefile
- Adding SVG panel UI
- Debugging build/runtime issues

### For Pattern Addition
Use `/doc-fix` command after solving a problem. The troubleshooting-docs skill will:
1. Document the solution in appropriate category
2. Check if it represents a common pattern
3. Ask if it should be promoted to critical patterns
4. Update this library accordingly

## Pattern Lifecycle

### Path 1: Through Documentation (Recommended)
```
Problem Solved
     |
/doc-fix (or auto-detect "that worked")
     |
Document in troubleshooting/[category]/[file].md
     |
Decision Menu appears:
  1. Continue workflow
  2. Add to Required Reading <-- USER CHOOSES THIS
  3. Link related issues
  4. Update common patterns
  5. View documentation
  6. Other
     |
Claude extracts WRONG vs CORRECT pattern
     |
Appends to vcv-rack-critical-patterns.md
     |
Next module build: Subagent reads pattern -> Avoids mistake
```

### Path 2: Direct Promotion (Fast)
```
Problem Solved
     |
User: "add that to required reading"
     |
/add-critical-pattern auto-invokes
     |
Claude extracts pattern from conversation
     |
Appends to vcv-rack-critical-patterns.md
     |
Next module build: Subagent reads pattern -> Avoids mistake
```

### Path 3: Suggested by System (Proactive)
```
Problem Solved
     |
/doc-fix
     |
Claude detects: severity=critical, affects Stage 2
     |
Decision Menu with note:
  "This might be worth adding to Required Reading (Option 2)"
     |
User decides: Choose Option 2 or ignore
```

## References

See individual troubleshooting docs for full context:
- `troubleshooting/build-failures/`
- `troubleshooting/runtime-issues/`
- `troubleshooting/gui-issues/`
- `troubleshooting/api-usage/`
- `troubleshooting/dsp-issues/`
- `troubleshooting/parameter-issues/`
