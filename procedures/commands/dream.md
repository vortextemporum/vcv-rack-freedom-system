# /dream

**Purpose:** Explore plugin ideas, improvements, UI mockups, or research problems through guided menus.

---

## Syntax

```bash
/dream                    # Interactive menu
/dream [PluginName]       # Explore specific plugin
```

## What It Does

Provides a guided exploration interface for creative work. Routes to appropriate skills based on your selection.

## Menu Options

When invoked without arguments, presents:

```
What would you like to explore?

1. New plugin idea
2. Improve existing plugin
3. Create UI mockup
4. Research problem
```

### Option 1: New Plugin Idea
Routes to `plugin-ideation` skill in new plugin mode.

**Use when:** You have a plugin concept to explore but haven't written a creative brief yet.

**Result:** Creates `plugins/[PluginName]/.ideas/creative-brief.md`

### Option 2: Improve Existing Plugin
Routes to `plugin-ideation` skill in improvement mode.

**Use when:** You want to explore enhancements before committing to implementation.

**Result:** Creates `plugins/[PluginName]/.ideas/improvements/[feature-name].md`

### Option 3: Create UI Mockup
Routes to `ui-mockup` skill.

**Use when:** You have a creative brief and want to visualize the interface.

**Result:** Creates 7 mockup files in `plugins/[PluginName]/.ideas/mockups/v[N]-*`

### Option 4: Research Problem
Routes to `deep-research` skill.

**Use when:** You need to investigate a technical problem, unknown API, or build error.

**Result:** Structured research findings with solutions and recommendations.

## With Plugin Name

```bash
/dream TestGainPlugin
```

Presents plugin-specific menu:

```
What would you like to do with TestGainPlugin?

1. Explore improvements
2. Create/update UI mockup
3. Research implementation approach
```

Skips "new plugin" option since the plugin already exists.

## Examples

**Exploring a new idea:**
```bash
/dream
→ Select "1. New plugin idea"
→ Interactive conversation captures vision
→ Creates creative brief
```

**Improving existing plugin:**
```bash
/dream MicroGlitch
→ Select "1. Explore improvements"
→ Brainstorm UI redesign ideas
→ Creates improvement proposal
```

**Creating mockup:**
```bash
/dream ReverbPlugin
→ Select "2. Create/update UI mockup"
→ Design interface visually
→ Generates HTML + C++ boilerplate
```

## When To Use

**Use `/dream` when:**
- You're in exploration mode (not ready to implement)
- You want to brainstorm before committing
- You need a visual mockup
- You want to research an approach

**Don't use `/dream` when:**
- You're ready to build (use `/implement`)
- You want to make specific changes (use `/improve`)
- You need to test (use `/test`)

## Related Commands

- `/implement` - Build plugin from creative brief
- `/improve` - Make changes to existing plugin
- `/continue` - Resume previous exploration session

## Output

All `/dream` operations create documentation in `.ideas/` subdirectories:
- Creative briefs
- Improvement proposals
- UI mockups
- Research findings

Nothing is implemented—this is purely exploratory.

## Tips

**Start broad:** Use `/dream` without arguments to see all options.

**Be specific:** Use `/dream [PluginName]` when you know which plugin to work on.

**Iterate freely:** Exploration is non-destructive—all versions preserved.

**Transition smoothly:** After exploration, use `/implement` or `/improve` to act on ideas.
