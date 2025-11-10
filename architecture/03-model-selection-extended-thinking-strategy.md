## Model Selection & Extended Thinking Strategy

### Claude Code Max Plan Optimization

**Context:** With Max Plan, all Claude models (Haiku, Sonnet, Opus) and extended thinking are **unlimited and free**. Architecture optimizes for **quality and success rate** rather than token cost.

### Model Selection by Component

**Opus (Most Powerful):**
```yaml
# Use for most complex reasoning tasks
components:
  - dsp-agent: Complex DSP algorithm design, novel implementations
  - troubleshooter (Level 3-4): Deep root cause investigation
  - Stage 0 Research: Algorithm comparison, architecture exploration
```

**Rationale:** Complex DSP (wavetable synthesis, physical modeling, advanced effects) benefits from deepest reasoning. No cost downside with Max Plan.

**Sonnet (Standard):**
```yaml
# Default for most components
components:
  - foundation-agent: CMake + project structure (straightforward)
  - shell-agent: Parameter boilerplate (template-based)
  - gui-agent: WebView binding (well-defined patterns)
  - validator: Semantic contract validation (quality gate)
  - Stage 1 Planning: Phase breakdown, risk assessment
```

**Rationale:** Balances quality and speed. Sufficient for well-defined tasks with established patterns.

**Haiku (Fast):**
```yaml
# Use only for trivial operations
components:
  - file-existence-checker: Simple bash commands
  - log-parser: Pattern extraction from build logs
  - quick-status-check: Read PLUGINS.md status
```

**Rationale:** Reserved for operations where reasoning depth doesn't matter (file I/O, simple parsing).

### Extended Thinking Usage

**Always Enable For:**

**Stage 0 (Research):**
- Algorithm comparison (multiple DSP approaches)
- JUCE API deep-dive (finding optimal patterns)
- Complex architecture design (5+ DSP components)
- Novel implementation strategies

**Example:**
```markdown
User: "Create shimmer reverb with granular texture"
→ Extended thinking explores:
  - Reverb algorithms (Freeverb, Dattorro, plate)
  - Granular synthesis approaches
  - Integration strategies
  - Trade-offs (CPU vs quality)
```

**Stage 1 (Planning):**
- Phase breakdown for complexity 4-5 plugins
- Risk assessment and mitigation
- Alternative implementation strategies
- Dependency analysis

**Validator:**
- Deep semantic contract analysis
- Multi-step verification chains
- Edge case consideration
- Cross-contract consistency checks

**Example:**
```markdown
Validator checks parameter-spec.md against architecture.md:
→ Extended thinking:
  - Do parameter ranges make sense for DSP algorithm?
  - Are modulation targets semantically valid?
  - Does UI layout support parameter relationships?
```

**Troubleshooter (Level 3-4):**
- Root cause investigation (not obvious errors)
- Multi-hypothesis testing
- Complex error pattern matching
- System-wide interaction analysis

**Conditionally Enable For:**

**Stage 4 (DSP):**
- Complexity 4-5 plugins: Enable for novel algorithms
- Complexity 1-3 plugins: Disable for standard effects

**Example:**
```markdown
Complexity 5 (wavetable synth): Extended thinking ON
→ Explores wavetable interpolation, anti-aliasing, modulation routing

Complexity 2 (simple delay): Extended thinking OFF
→ Straightforward circular buffer implementation
```

**Never Enable For:**

- Stage 2 (Foundation): Template-based CMake setup
- Stage 3 (Shell): Boilerplate parameter declarations
- File operations: Simple read/write/copy
- Build log parsing: Pattern matching only

### Implementation

**Agent frontmatter:**
```yaml
---
name: dsp-agent
model: opus
extended_thinking: true
extended_thinking_budget: 10000  # Max thinking tokens
---
```

**Conditional extended thinking:**
```yaml
---
name: dsp-agent
model: opus
extended_thinking: |
  {{ complexity >= 4 }}  # Enable only for complex plugins
---
```

### Performance Impact

**Extended thinking overhead:**
- Research: +2-5 minutes (high value - catches architectural issues)
- Planning: +1-3 minutes (high value - prevents rework)
- DSP: +3-8 minutes (high value for complex algorithms)
- Validation: +1-2 minutes (high value - quality gate)

**Success rate improvement (estimated):**
- First-attempt success: 70% → 85% (with extended thinking)
- Rework time saved: 15-30 minutes per caught issue
- Net time savings: Positive for complexity 3+

### Key Principles

1. **Quality > Speed** - Max Plan removes cost constraints
2. **Extended thinking for novel problems** - Not templates/boilerplate
3. **Opus for complex reasoning** - DSP algorithms, deep investigation
4. **Sonnet as default** - Good balance for most tasks
5. **Haiku rarely** - Only trivial file operations

---
