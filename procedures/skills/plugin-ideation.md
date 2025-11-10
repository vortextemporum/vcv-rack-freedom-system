# plugin-ideation

**Purpose:** Interactive brainstorming and creative vision capture for new plugins and improvements to existing plugins.

**Version:** 3.0.0

---

## What It Does

Conducts an adaptive interview-style conversation to capture your creative vision before any code is written. Creates detailed specification documents that serve as the blueprint for implementation or improvement.

## When To Use

**For new plugins:**
- "I want to make a granular delay"
- "Thinking about building a compressor"
- "Brainstorm plugin ideas"

**For improvements:**
- "Explore improvements to ReverbPlugin"
- "Think about adding presets to MicroGlitch"
- "Brainstorm UI redesign for DelayPlugin"

## Two Modes

### New Plugin Mode
Used when the plugin doesn't exist yet. Explores:
- Plugin type (effect/synth/utility)
- Core concept and sonic goals
- Parameter ideas and ranges
- UI vision and layout preferences
- Use cases and target users
- Inspirations and references

### Improvement Mode
Used when enhancing existing plugins. Explores:
- What aspect to improve (DSP/Parameters/UI/Features)
- Current state and pain points
- Proposed changes
- Backward compatibility considerations
- Testing criteria

## How It Works

### Phase 1: Free-Form Collection
Starts with an open question: "What would you like to build?" (or "What would you like to improve?")

Extracts information from your response without forcing a rigid structure.

### Phase 2: Targeted Refinement
Asks **only** about gaps not covered in Phase 1. One question at a time, adaptively based on what you've already said.

### Phase 3: Checkpoints
Shows progress summary and offers to continue exploring or finalize. You control the depth—stop when ready or dive deeper into specific areas.

### Phase 4: Document Creation
Creates structured specification document with everything discussed.

### Phase 5: Session Handoff
Creates `.continue-here.md` with context for resuming later.

### Phase 6: Next Steps
Offers intelligent next actions:
- Create UI mockup (if UI was discussed)
- Start implementation (if ready to build)
- Save for later (if still exploring)

## What It Creates

**For new plugins:**
```
plugins/[PluginName]/.ideas/creative-brief.md
plugins/[PluginName]/.continue-here.md
```

**For improvements:**
```
plugins/[PluginName]/.ideas/improvements/[feature-name].md
plugins/[PluginName]/.continue-here.md
```

## Key Features

### Adaptive Questioning
- Skips redundant questions if you've already provided information
- Adjusts depth based on your level of detail
- Suggests deep-dive areas but lets you control exploration

### Grounded Feasibility
- Gently notes potential challenges without shutting down creativity
- Flags ambitious ideas for research phase
- Balances dreaming with practical implementation

### Continuous Iteration
- No fixed number of questions
- Can explore indefinitely until you say "finalize"
- Supports "ask me about [specific area]" requests

## Example Flow

**User:** "I want to make a tape delay with wow and flutter"

**System extracts:**
- Type: Effect ✓
- Core concept: Tape delay ✓
- Parameters: wow, flutter (implied)
- Sonic goal: analog warmth ✓

**System asks only:**
- "What parameters beyond wow and flutter?"
- "Visual style preferences?"
- (Skips asking about plugin type, core concept, sonic goal—already covered)

**Result:** Creative brief with complete vision captured efficiently.

## Integration

**Calls after completion:**
- Offers to invoke `ui-mockup` skill (if UI discussed)
- Offers to invoke `plugin-workflow` skill (for new plugins)
- Offers to invoke `plugin-improve` skill (for improvements)

**Called by:**
- Natural language detection ("I want to make...", "explore improvements...")
- `/dream` slash command

## Best Practices

**Be descriptive initially:** The more you share upfront, the fewer follow-up questions needed.

**Request specific focus:** Say "ask me about presets" to dive into one area.

**Iterate freely:** Say "finalize" when ready, but explore as long as you want.

**Reference inspirations:** Mention similar plugins or sonic references to speed up understanding.

## Output Quality

The creative brief becomes the reference document for:
- Planning phase (Stage 1)
- DSP implementation (Stage 4)
- GUI design (Stage 5)
- Preset creation (Stage 6)

Quality here determines quality of entire workflow downstream.
