# troubleshooter

**Location:** `.claude/agents/troubleshooter.md`

**Purpose:** Deep research agent for JUCE plugin development issues using graduated depth protocol.

---

## What It Is

A specialized autonomous agent configured for comprehensive problem investigation. Uses a 4-level graduated research protocol to find authoritative solutions efficiently.

**Not a skill, not a command—an agent configuration.**

## When It Runs

**Auto-invoked by:**
- `deep-research` skill when investigating complex problems
- Build failure protocol (Option 1: Investigate)
- Natural language detection ("research this", "investigate this")

**Model:** Sonnet (not Opus)—balances thoroughness with speed

**Tools available:**
- Bash (for local file searches)
- Read (for reading docs and code)
- WebSearch (for web research)
- WebFetch (for fetching documentation pages)
- Context7 (for official JUCE documentation)

## Research Protocol (Graduated Depth)

The agent stops as soon as it finds a confident answer—doesn't over-research simple problems.

### Level 0: Quick Assessment (30 seconds)

**First check:** Is this immediately obvious?

**Obvious problems (skip all research):**
- Simple syntax errors (missing semicolon, bracket)
- Misspelled variable/function names
- Common beginner mistakes
- Error message clearly states the issue

**Action:** Answer directly with fix, no research needed

**Example:**
```
Error: missing semicolon on line 42
→ Skip research, fix obvious syntax error
```

### Level 1: Local Troubleshooting Docs (1-2 minutes)

**Why:** Pre-validated solutions that already worked in this workspace

**Process:**
1. **Classify problem** (problem_type, component, severity)
2. **Hierarchical search:**
   - Pass 1: Exact match (problem_type + component)
   - Pass 2: Fuzzy match (symptoms/tags)
   - Pass 3: Full-text search (keywords)
3. **Filter results** (check YAML front matter first)
4. **Read full doc** only if front matter matches

**Stops here if:** High confidence match found in local docs

**Example:**
```
Problem: Rotary sliders overlapping in FlexBox
→ Search docs/troubleshooting/ for "flexbox" + "layout" + "slider"
→ Found: flexbox-layout-overlapping-components.md
→ JUCE version matches, symptoms match
→ Use that solution, STOP
```

### Level 2: Context7 Official Docs (3-5 minutes)

**Why:** Official JUCE documentation is authoritative

**Process:**
1. Query Context7 with JUCE library ID
2. Search for relevant component (FlexBox, Slider, AudioProcessor, etc.)
3. Look for API docs, migration guides, best practices

**Stops here if:** Clear answer in official documentation

**Example:**
```
Problem: Unknown JUCE API method
→ Query Context7 for juce::dsp::Compressor
→ Found official API documentation
→ Method requires prepare() call first
→ Use official guidance, STOP
```

### Level 3: Web Research (10-15 minutes)

**Why:** Complex/unusual problems requiring multiple sources

**Only reaches this level if:**
- Not in local troubleshooting docs
- Not clearly answered in official docs
- Genuinely complex or unusual problem

**Process:**
1. Multiple search query variations
2. Evaluate source credibility:
   - ✅ Official JUCE forum
   - ✅ JUCE GitHub issues
   - ✅ Stack Overflow (high votes, recent)
   - ⚠️ Blog posts (verify)
   - ❌ Old pre-JUCE 6 tutorials
3. Cross-reference 2-3 sources
4. Check version compatibility

**Example:**
```
Problem: Obscure build error, not in docs or Context7
→ Search JUCE forum, GitHub issues, Stack Overflow
→ Find 3 sources agreeing on solution
→ Verify version compatibility
→ Present findings with confidence assessment
```

## Analysis Requirements

After research, agent analyzes:

1. **Root Cause** - Why problem exists fundamentally
2. **Version Relevance** - Applies to our JUCE version?
3. **Solution Quality** - Proper fix or workaround?
4. **Confidence Level** - HIGH / MEDIUM / LOW

## Report Format

Agent presents findings in structured markdown:

```markdown
## Research Report: [Problem Description]

### Problem Identified
- Error/Issue: [exact error]
- Context: [JUCE version, component]
- Root Cause: [technical explanation]

### Research Findings

#### Local Troubleshooting Docs
- Classification: [problem_type, component, severity]
- Search strategy: [passes used]
- Results: [matched doc or none found]

#### Context7 Documentation
[API documentation found or not found]

#### Web Research Sources
1. [Source] - [credibility] - [key finding]
2. [Source] - [credibility] - [key finding]

### Confidence Assessment
- Confidence Level: HIGH/MEDIUM/LOW
- Reasoning: [why confident or not]

### Recommended Solution

[HIGH confidence]:
- Proposed Fix: [exact code]
- Why This Works: [technical reasoning]
- Implementation Steps: [1, 2, 3]

[MEDIUM confidence]:
- Possible Solution: [likely approach]
- Uncertainties: [what's unclear]
- Suggested Validation: [how to test]

[LOW confidence]:
- Unable to Find Definitive Answer
- What I searched: [terms used]
- What's unclear: [specific unknowns]
- Suggested next steps: [alternatives]
```

## Research Rules

**STOP as soon as confident answer found:**
- Don't research obvious problems (Level 0)
- Don't continue past Level 1 if local docs have answer
- Don't continue past Level 2 if official docs are clear
- Only reach Level 3 for complex/unusual problems

**Always explain WHY:**
- Not just what to change
- Technical reasoning behind solution
- How it addresses root cause

**Never propose uncertain solutions as confident:**
- Be honest about confidence level
- Acknowledge limitations
- Note conflicting information

**Version numbers matter:**
- Always check solution applies to our JUCE version
- Note version compatibility in report

## Why This Works

**Graduated depth prevents over-research:**
- Simple problems solved in seconds (Level 0)
- Common problems solved in minutes (Level 1)
- Most problems solved in 3-5 minutes (Level 2)
- Only complex problems reach Level 3

**Local docs are fastest:**
- Pre-validated solutions
- Already worked in this workspace
- Same JUCE version
- Searchable by symptoms/tags

**Official docs are authoritative:**
- Context7 provides up-to-date JUCE documentation
- API changes, migration guides, best practices
- Stops most problems here

**Web research is comprehensive:**
- Multiple credible sources
- Cross-referencing for consensus
- Version verification
- Credibility assessment

## Example Scenarios

**Level 0 (obvious):**
```
Error: expected ';' before '}' token
→ Missing semicolon
→ Fix directly, no research
→ 30 seconds
```

**Level 1 (local docs):**
```
Error: FlexBox layout broken, sliders overlap
→ Search local troubleshooting docs
→ Found flexbox-layout-overlapping-components.md
→ Solution: Add .withFlex(1) constraints
→ 2 minutes
```

**Level 2 (Context7):**
```
Error: How to use juce::dsp::Compressor?
→ Query Context7 for juce::dsp::Compressor
→ Official API docs show prepare() + process()
→ Use official guidance
→ 4 minutes
```

**Level 3 (web research):**
```
Error: Obscure linker error on M1 Mac
→ Not in local docs
→ Not in Context7
→ Search JUCE forum, GitHub, Stack Overflow
→ Find 3 sources with same solution
→ Cross-reference, verify version
→ 12 minutes
```

## Configuration Details

**From `.claude/agents/troubleshooter.md` frontmatter:**

```yaml
name: troubleshooter
description: Deep research agent for JUCE plugin development issues
model: sonnet
tools:
  - Bash
  - Read
  - WebSearch
  - WebFetch
  - mcp__context7__resolve-library-id
  - mcp__context7__get-library-docs
```

**Model choice (Sonnet):**
- Balances thoroughness with cost/speed
- Sufficient for research tasks
- Faster than Opus for web searches

**Tool restrictions:**
- No Write/Edit (research only, no code changes)
- No Task (focused agent, doesn't spawn sub-agents)
- Has web access for deep research

## Integration

**Invoked by:**
- `deep-research` skill (primary)
- Build failure protocol investigations
- Natural language triggers

**Feeds into:**
- `troubleshooting-docs` skill (solutions get documented)
- User decision-making (presents findings, waits for approval)

**Does NOT:**
- Make code changes (research only)
- Commit changes
- Build or test
- Invoke other skills

## Output Quality

After investigation:
- ✅ Root cause identified
- ✅ Solution quality assessed
- ✅ Confidence level stated
- ✅ Version compatibility checked
- ✅ Multiple sources cross-referenced (if Level 3)
- ✅ Technical reasoning explained
- ✅ Implementation steps provided (if confident)

## Comparison: Agent vs Skill

**Agent (troubleshooter.md):**
- Configuration file
- Defines model, tools, behavior
- Autonomous execution
- Invoked by skills

**Skill (deep-research):**
- Orchestration layer
- Decides when to invoke agent
- Handles user interaction
- Manages workflow

**Analogy:**
- Skill = Manager (decides what research to do)
- Agent = Researcher (executes research protocol)

## Related

- `deep-research` skill - Invokes this agent
- `troubleshooting-docs` skill - Documents solutions agent finds
- Build failure protocol - Offers investigation option
- Context7 MCP - Provides JUCE documentation access

## Tips

**Trust the graduated protocol:**
- Don't manually force deeper research
- Let agent stop when confident
- Most problems solved at Level 1-2

**Local docs are your friend:**
- Populate with solved problems
- Faster than web research
- Pre-validated for your workspace

**Confidence levels matter:**
- HIGH = implement confidently
- MEDIUM = validate before implementing
- LOW = investigate alternatives

**Version compatibility is critical:**
- JUCE API changes between versions
- Agent always checks version relevance
- Note version mismatches in report

## Summary

The troubleshooter agent is a specialized research configuration that efficiently finds solutions using a 4-level graduated protocol. Stops as soon as confident answer is found, preventing over-research. Prioritizes local docs (fastest), then official docs (authoritative), then web (comprehensive). Always honest about confidence level and explains technical reasoning behind solutions.
