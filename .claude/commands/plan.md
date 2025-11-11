---
name: plan
description: Interactive research and planning for plugin (Stages 0-1)
---

# /plan

When user runs `/plan [PluginName?]`, invoke the plugin-planning skill to handle Stages 0-1 (Research and Planning).

## Preconditions

**Check PLUGINS.md status:**

1. Verify plugin exists in PLUGINS.md
2. Check current status:
   - If 💡 Ideated: OK to proceed (start fresh)
   - If 🚧 Stage 0: OK to proceed (resume research)
   - If 🚧 Stage 1: OK to proceed (resume planning)
   - If 🚧 Stage 2+: **BLOCK** - Plugin already in implementation

**If plugin is Stage 2 or beyond:**
```
[PluginName] is already in implementation (Stage [N]).

Stages 0-1 (planning) are complete. Use:
- /continue [PluginName] - Resume from current stage
- /implement [PluginName] - Review implementation workflow
```

**Check for creative brief:**
```
plugins/[PluginName]/.ideas/creative-brief.md
```

If missing, offer:
```
✗ No creative brief found for [PluginName]

Planning requires a creative brief to define plugin vision.

Would you like to:
1. Create one now (/dream [PluginName])
2. Skip planning (not recommended - leads to implementation drift)
```

## Behavior

**Without argument:**
List plugins eligible for planning:
- Status: 💡 Ideated
- Status: 🚧 Stage 0 (resume)
- Status: 🚧 Stage 1 (resume)

Present numbered menu of eligible plugins or offer to create new plugin.

**With plugin name:**
```bash
/plan [PluginName]
```

Verify preconditions, then invoke plugin-planning skill.

## The Planning Stages

The plugin-planning skill executes:

**Stage 0: Research (5-10 min)**
- Identify plugin technical approach
- Research JUCE DSP modules
- Research professional plugin examples
- Research parameter ranges
- Design sync (if mockup exists)
- Output: architecture.md (DSP specification)

**Stage 1: Planning (2-5 min)**
- Calculate complexity score
- Determine implementation strategy (single-pass vs phased)
- Create phase breakdown for complex plugins
- Output: plan.md (implementation strategy)

## Contract Enforcement

**Stage 0 (Research):**
- Requires: creative-brief.md
- Creates: architecture.md

**Stage 1 (Planning) BLOCKS if missing:**
- parameter-spec.md (from UI mockup finalization)
- architecture.md (from Stage 0)

If blocked at Stage 1:
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✗ BLOCKED: Cannot proceed to Stage 1
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Missing implementation contracts:
✓ creative-brief.md - exists
✗ parameter-spec.md - MISSING (required)
[✓/✗] architecture.md - [status]

HOW TO UNBLOCK:
1. parameter-spec.md: Complete ui-mockup workflow
   Run: /dream [PluginName] → Choose "Create UI mockup" → Finalize

2. architecture.md: Complete Stage 0 (Research)
   Run: /plan [PluginName] → Complete Stage 0 first

Once both contracts exist, Stage 1 will proceed.
```

## Handoff to Implementation

After Stage 1 completes, the skill creates handoff state:
- .continue-here.md updated with "ready_for_implementation: true"
- User runs `/implement [PluginName]` to begin Stage 2 (Foundation)

## Workflow Integration

Complete plugin development flow:
1. `/dream [PluginName]` - Create creative brief + UI mockup
2. `/plan [PluginName]` - Research and planning (Stages 0-1)
3. `/implement [PluginName]` - Build plugin (Stages 2-6)

## Output

By completion of planning, you have:
- ✅ architecture.md (DSP specification)
- ✅ plan.md (implementation strategy with complexity score)
- ✅ Updated PLUGINS.md status
- ✅ Git commits for both stages
- ✅ Ready for implementation handoff
