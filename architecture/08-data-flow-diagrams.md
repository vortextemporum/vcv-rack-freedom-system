## Data Flow Diagrams

### Request → Result Flow

```
┌─────────────┐
│ User Request│
└──────┬──────┘
       │
       ▼
┌─────────────────────┐
│ CLAUDE + CLAUDE.md  │
│  1. Parse request   │
│  2. Check state     │
│  3. Find workflow   │
│  4. Invoke skill    │
└──────┬──────────────┘
       │
       ▼
┌─────────────────────┐
│ WORKFLOW            │
│  1. Check precond   │
│  2. Dispatch subagnt│
│  3. Manage state    │
│  4. Commit changes  │
└──────┬──────────────┘
       │
       ├──────────────┐
       │              │
       ▼              ▼
┌───────────┐  ┌───────────┐
│Subagent 1 │  │Subagent 2 │
│(Stage 2)  │  │(Stage 3)  │
└─────┬─────┘  └─────┬─────┘
      │              │
      │  ┌───────────┘
      │  │
      ▼  ▼
┌───────────────┐
│   VALIDATOR   │
│  Verify claims│
└────┬──────────┘
     │
     ├─ PASS ───┐
     │          │
     ▼          ▼
┌─────────┐  ┌─────────┐
│Git Commit│  │Continue │
│State +   │  │to Next  │
│Code      │  │Stage    │
└──────────┘  └─────────┘
```

### Component Interaction Sequence

**Example: Stage 4 (DSP) Implementation**

```
User → Claude → plugin-workflow → dsp-agent subagent → Validator → Git
│                        │                  │           │         │
│ "continue to DSP"      │                  │           │         │
│───────────────────────>│                  │           │         │
│                        │                  │           │         │
│                        │ Check prereqs:   │           │         │
│                        │ - parameter-spec ✓│          │         │
│                        │ - architecture ✓  │          │         │
│                        │ - plan ✓          │          │         │
│                        │                  │           │         │
│                        │ Task(dsp-agent)  │           │         │
│                        │─────────────────>│           │         │
│                        │                  │           │         │
│                        │                  │ Execute:  │         │
│                        │                  │ - Read spec│        │
│                        │                  │ - Read arch│        │
│                        │                  │ - Impl DSP │        │
│                        │                  │ - Build    │        │
│                        │                  │ - Test     │        │
│                        │                  │           │         │
│                        │<─────────────────│           │         │
│                        │ Report JSON      │           │         │
│                        │                  │           │         │
│                        │ Task(validator)  │           │         │
│                        │──────────────────────────────>│         │
│                        │                  │           │         │
│                        │                  │           │ Verify: │
│                        │                  │           │ - Comps │
│                        │                  │           │ - Params│
│                        │                  │           │ - Chain │
│                        │                  │           │         │
│                        │<──────────────────────────────│         │
│                        │ Validation: PASS │           │         │
│                        │                  │           │         │
│                        │ git commit       │           │         │
│                        │──────────────────────────────────────>│
│                        │                  │           │         │
│                        │<──────────────────────────────────────│
│                        │ Committed        │           │         │
│                        │                  │           │         │
│<───────────────────────│                  │           │         │
│ "✓ Stage 4 complete"   │                  │           │         │
│ "Continue to Stage 5?" │                  │           │         │
```

### State Transitions

```
Plugin Creation Flow:

[Initial State]
    │
    │ /dream
    ▼
[💡 Ideated]
    │
    │ Finalize mockup
    ▼
[💡 Ideated + parameter-spec.md]
    │
    │ /implement
    ▼
[🚧 Stage 0] ──┐
    │          │ Research
    ▼          │
[🚧 Stage 0]◄──┘
    │
    │ architecture.md validated
    ▼
[🚧 Stage 1] ──┐
    │          │ Planning
    ▼          │
[🚧 Stage 1]◄──┘
    │
    │ plan.md validated
    ▼
[🚧 Stage 2] ──┐
    │          │ foundation-agent subagent
    ▼          │
[🚧 Stage 2]◄──┘
    │
    │ Build successful, validated
    ▼
[🚧 Stage 3] ──┐
    │          │ shell-agent subagent
    ▼          │
[🚧 Stage 3]◄──┘
    │
    │ Parameters validated
    ▼
[🚧 Stage 4] ──┐
    │          │ dsp-agent subagent (all phases)
    ▼          │
[🚧 Stage 4]◄──┘
    │
    │ DSP validated
    ▼
[🚧 Stage 5] ──┐
    │          │ gui-agent subagent
    ▼          │
[🚧 Stage 5]◄──┘
    │
    │ GUI validated
    ▼
[🚧 Stage 6] ──┐
    │          │ Validation
    ▼          │
[🚧 Stage 6]◄──┘
    │
    │ Pluginval passed
    ▼
[✅ Working]
    │
    │ /install-plugin
    ▼
[📦 Installed]
    │
    │ /improve
    ▼
[🚧 Improving] ──┐
    │            │ Versioned improvement
    ▼            │
[🚧 Improving]◄──┘
    │
    │ Improvement complete
    ▼
[📦 Installed v1.1.0]
```

### Critical Paths

**Happy path (simple plugin):**
```
User request
 → Claude (30s) [consults CLAUDE.md for workflow]
 → plugin-workflow Stage 0 (5min)
 → Validator (30s) → PASS
 → plugin-workflow Stage 1 (2min)
 → Validator (30s) → PASS
 → foundation-agent subagent (5min)
 → Validator (30s) → PASS
 → shell-agent subagent (3min)
 → Validator (30s) → PASS
 → dsp-agent subagent (15min)
 → Validator (1min) → PASS
 → gui-agent subagent (10min)
 → Validator (1min) → PASS
 → plugin-workflow Stage 6 (5min)
 → Validator (30s) → PASS
 → Result: Complete plugin (~50min, single session)
```

**Error path (build failure):**
```
foundation-agent subagent
 → build fails
 → Subagent reports failure
 → Validator detects failure
 → plugin-workflow pauses
 → Presents 4 options
 → User chooses "Investigate"
 → troubleshooter subagent researches
 → Returns findings
 → User confirms fix approach
 → foundation-agent subagent retries
 → Build succeeds
 → Validator passes
 → Continue to Stage 3
```

---
