# deep-research

**Purpose:** Autonomous investigation of problems, errors, and unknowns using graduated research depth protocol.

---

## What It Does

Conducts thorough research on technical problems by consulting multiple knowledge sources in order of specificity. Prevents band-aid fixes by finding root causes.

## When Invoked

**Automatically triggered by:**
- Build failures (user chooses "Investigate" option)
- Unknown API usage
- Unexpected plugin behavior

**Manually triggered by:**
- "Research [topic]"
- "Investigate [problem]"
- "How does [X] work"
- `/dream` → "Research problem"

## Research Sources (Priority Order)

### 1. Local Troubleshooting Docs
**Location:** `docs/troubleshooting/*.md`

**What's there:**
- Previously solved problems
- Known JUCE issues
- Common error patterns
- Verified solutions

**When used:** Always checked first (fastest, most relevant)

### 2. Context7 JUCE Documentation
**Source:** Official JUCE documentation via Context7 MCP

**What's there:**
- JUCE API reference
- Class documentation
- Usage examples
- Best practices

**When used:** For JUCE-specific questions and API usage

### 3. Web Search
**Sources:** Current web documentation, forums, Stack Overflow

**What's there:**
- Recent solutions
- Community knowledge
- Third-party libraries
- Platform-specific issues

**When used:** When local docs and JUCE docs don't have answers

## Graduated Depth Protocol

### Tier 1: Quick Check (5-10 minutes)
**Scope:** Local docs + basic JUCE lookup

**Use for:**
- Common errors with known solutions
- Standard API usage questions
- Documented patterns

**Output:** Direct answer with reference

### Tier 2: Moderate Investigation (15-30 minutes)
**Scope:** Thorough JUCE docs + targeted web search

**Use for:**
- Uncommon errors
- Complex API interactions
- Integration issues

**Output:** Structured analysis with multiple solution options

### Tier 3: Deep Research (30-60 minutes)
**Scope:** Exhaustive multi-source investigation

**Use for:**
- Architectural problems
- Unknown error patterns
- Performance optimization
- Novel implementations

**Output:** Comprehensive report with:
- Problem analysis
- Multiple approaches evaluated
- Recommended solution with rationale
- Implementation notes
- Prevention strategies

## Research Process

### Step 1: Problem Definition
Extracts from context:
- Error message (exact text)
- File and line number
- What was being attempted
- Expected vs actual behavior
- Environmental factors

### Step 2: Local Knowledge Check
Searches `docs/troubleshooting/`:
```
grep -r "error message text" docs/troubleshooting/
```

If match found: Returns solution immediately

### Step 3: JUCE Documentation Query
Consults Context7 for JUCE APIs:
- Relevant class documentation
- Method signatures
- Usage examples
- Known limitations

### Step 4: Web Investigation (if needed)
Searches for:
- Error message + "JUCE"
- Problem description + version info
- Similar implementations
- Platform-specific solutions

### Step 5: Synthesis
Combines findings into structured report:

```markdown
# Research: [Problem Description]

## Problem Analysis
[Clear explanation of what's happening and why]

## Root Cause
[Technical explanation of underlying issue]

## Solutions

### Option 1: [Recommended]
**Approach:** ...
**Why:** ...
**Implementation:** ...
**Trade-offs:** ...

### Option 2: [Alternative]
**Approach:** ...
**Why:** ...
**Implementation:** ...
**Trade-offs:** ...

## Prevention
[How to avoid this in the future]

## References
- [Source 1]
- [Source 2]
```

## Integration with Troubleshooting Docs

After problem is solved, skill suggests:

```
This was a non-trivial problem. Should I document the solution?
```

If yes, invokes `troubleshooting-docs` skill to create permanent reference.

**Benefit:** Next time this happens, Tier 1 finds the answer instantly.

## Example Scenarios

### Build Error Investigation

**Input:**
```
Build failed with:
'juce::WebSliderRelay' does not name a type
File: PluginEditor.h:32
```

**Tier 1 Research:**
- Checks troubleshooting docs: No match
- Escalates to Tier 2

**Tier 2 Research:**
- Queries Context7 for WebSliderRelay
- Finds: Requires JUCE 8.0.9+
- Checks CMakeLists.txt: Using JUCE 8.0.3
- **Solution found:** Upgrade JUCE version

**Output:**
```
Root Cause: JUCE version too old

juce::WebSliderRelay was introduced in JUCE 8.0.9.
Your CMakeLists.txt specifies 8.0.3.

Solution: Update JUCE requirement
Change: set(JUCE_VERSION 8.0.3)
To:     set(JUCE_VERSION 8.0.9)

Rebuild after updating.
```

### API Usage Question

**Input:**
```
How do I implement pitch shifting in JUCE?
```

**Tier 2 Research:**
- Checks JUCE docs for pitch shifting classes
- Finds: No built-in pitch shifter
- Searches for windowed FFT approach
- Finds: juce::dsp::FFT class

**Output:**
```
JUCE doesn't have built-in pitch shifting.

Recommended approach:
1. Use juce::dsp::FFT for frequency domain processing
2. Implement phase vocoder technique
3. Or: Use external library (RubberBand, SoundTouch)

Implementation options:
[Detailed breakdown of each approach]

Reference implementation:
[Link to example code]
```

### Performance Problem

**Input:**
```
Plugin causing CPU spikes when parameters change
```

**Tier 3 Research:**
- Investigates parameter smoothing
- Checks for real-time violations
- Analyzes processBlock efficiency
- Reviews JUCE best practices

**Output:**
```
Problem: Parameter changes triggering expensive calculations in audio thread

Root Cause Analysis:
1. setParameter() called from UI thread
2. processBlock() recalculates filter coefficients every sample
3. No parameter smoothing implemented

Solution (3-part):
1. Implement juce::SmoothedValue for parameter changes
2. Precalculate coefficients, store in atomic variables
3. Update coefficients once per buffer, not per sample

[Detailed implementation guide]

Performance impact: CPU usage reduced from 40% to 5%
```

## Output Formats

### Quick Answer (Tier 1)
```
Error: Missing include

Add to PluginProcessor.h:
#include <juce_dsp/juce_dsp.h>

Reference: docs/troubleshooting/missing-juce-includes.md
```

### Analysis (Tier 2)
```
Problem: WebView not rendering

Possible causes:
1. Missing JUCE_WEB_BROWSER=1 flag (most likely)
2. ui/public/ directory empty
3. Resource provider not returning correct MIME types

Check these in order, report findings.
```

### Comprehensive Report (Tier 3)
```
[Full markdown document with:]
- Executive summary
- Problem breakdown
- Technical deep-dive
- Multiple solution options
- Implementation guidance
- Code examples
- Performance considerations
- Testing approach
- References
```

## Integration Points

**Triggered by:**
- `build-automation` (when build fails)
- `plugin-improve` Phase 0.5 (for complex issues)
- Manual user requests

**Creates:**
- Research findings (inline or markdown file)
- Suggests troubleshooting doc creation

**Calls:**
- `troubleshooting-docs` (if solution should be documented)

## Best Practices

**Don't guess:** If local docs don't have it, search properly

**Document findings:** Create troubleshooting docs for non-trivial solutions

**Escalate appropriately:** Don't spend 30 minutes on a 5-minute question

**Provide options:** Multiple solutions with trade-offs, not just one answer

**Verify sources:** Check JUCE version compatibility, platform requirements

## Success Criteria

Research is complete when:
- ✅ Root cause identified
- ✅ Solution provided with implementation steps
- ✅ Trade-offs explained
- ✅ References cited
- ✅ Prevention strategy suggested

## Failure Mode

If research exhausts all sources without finding solution:

```
Research exhausted. No definitive solution found.

Attempted:
✓ Local troubleshooting docs (0 matches)
✓ JUCE documentation (API exists but undocumented)
✓ Web search (no relevant results)

Recommendations:
1. Post to JUCE forum with detailed error
2. Try alternative approach: [suggestion]
3. Contact JUCE support if critical

I can help formulate the forum post if needed.
```

Honest about limitations.
