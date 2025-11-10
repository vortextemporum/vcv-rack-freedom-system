# System Philosophy

**TÂCHES v2.0 is designed to be discovered through play, not learned through manuals.**

---

## Core Principle

**Every checkpoint is a "choose your own adventure" moment** where users:
- Discover capabilities organically
- Shape their own journey
- Learn by trying (risk-free via Claude Code checkpointing)
- Feel in control, not railroaded

This makes the system **fun and personal** rather than procedural.

---

## Progressive Disclosure Over Documentation

**Users learn features through two discovery mechanisms:**

### 1. Command Discovery (Entry Points)

```
User types "/"
→ IDE autocomplete shows all commands:
  /dream
  /implement
  /improve
  /continue
  /test
  /mockup
  /research
  /status
  /help
```

**Benefit:** User sees ALL workflows instantly. No guessing, no docs.

### 2. Feature Discovery (Within Workflows)

**Session 1:**
```
User: /mockup DelayPlugin (3 knobs, warm vintage design)
→ Mockup complete
System: "3. Save aesthetic as template"
User thinks: "What's that?"
User tries it → Saves aesthetic (colors, styling, vibe)
User learns: "Oh, I can reuse this visual style!"
```

**Session 2:**
```
User: /mockup ReverbPlugin (8 parameters, completely different)
System: "2. Apply an aesthetic (1 available)"
User thinks: "Oh yeah, that warm vintage style!"
User applies it → New layout, same aesthetic
User learns: "It adapted the style to my 8 parameters!"
```

**Session 3:**
```
User confidently builds plugin family with unified visual brand
System generates appropriate layouts using saved aesthetics
Each plugin looks cohesive but functionally appropriate
```

**No manual required. Commands are discovered via autocomplete, features via contextual options.**

---

## Risk-Free Exploration

**Claude Code checkpointing (Esc+Esc) enables experimentation:**

```
User: "3" (Save as template)
→ Template saved
User: "Hmm, don't want that actually"
User: Esc + Esc → Rewind
User: Restore code only → Template gone
User: "2" (Different option) → New path
```

**Benefit:** Users can safely try any option without fear. Makes exploration **fun** instead of risky.

---

## Hidden Features Surfaced Naturally

**Capabilities users wouldn't know exist:**

- UI aesthetic library (save/reuse visual styles across plugins)
- Design sync (brief ↔ mockup)
- Improvement versioning (v1.0 → v1.1 with backups)
- Troubleshooting documentation capture
- Resume from any checkpoint
- Multiple mockup iterations
- Subagent autonomy levels
- Aesthetic adaptation (same style, different layouts)

**All discovered through:**
> "Option 3 looks interesting... what's that?" → Tries it → "Oh shit, this is useful!"

---

## Every Checkpoint Is a Teaching Moment

**Stage boundaries:**
```
✓ Stage 3 complete

What's next?
1. Continue to Stage 4 (recommended)
2. Review Stage 3 code
3. Test current state
4. Pause here ← User discovers clean exit points
5. Other
```

**UI mockup complete:**
```
✓ Mockup v2 created

What's next?
1. Iterate on this design
2. Finalize and proceed
3. Save aesthetic as template ← User discovers aesthetic library
4. Start fresh
5. Other
```

**Starting UI design:**
```
Ready to design ReverbPlugin UI.

How would you like to start?
1. Brainstorm together
2. Apply an aesthetic (3 available) ← User discovers saved aesthetics
3. Upload a sketch
4. Describe it directly
5. Other
```

**Build failure:**
```
Build failed: linker error

What would you like to do?
1. Investigate ← User discovers troubleshooting
2. Show me the code
3. Show build output
4. I'll fix it manually
5. Other
```

---

## Minimal Learning Curve

**First-time user experience:**

1. User types "/" → Sees all commands via autocomplete
2. User: `/implement ReverbPlugin`
3. System: Presents options at each checkpoint
4. User: Discovers features by seeing them in context
5. User: Tries options (risk-free with checkpointing)
6. User: Learns capabilities without reading docs
7. Result: **Working plugin in one session** while learning the system

**Experienced user experience:**

1. Types slash commands without thinking
2. Confidently navigates using numbered shortcuts
3. Knows which options do what from prior exploration
4. System feels personalized to their workflow
5. Fast, fluid development

---

## Why This Matters

**Traditional approach:**
- User reads 50-page manual
- Tries to remember options
- Feels lost when stuck
- Gives up before mastering system

**TÂCHES approach:**
- User starts playing immediately
- System shows options in context
- User discovers capabilities organically
- Feels empowered, not overwhelmed

**Result:** System becomes **beloved** instead of just **useful**.

---

## Implementation Principles

1. **Present options at EVERY meaningful checkpoint**
   - Stage boundaries
   - Completion moments
   - Error scenarios
   - Starting new workflows
   - Ambiguous requests

2. **Make options contextual**
   - Show what's relevant NOW
   - Mark recommended path
   - Include "Pause here" for clean exits
   - Always include "Other" for flexibility

3. **Surface hidden features through options**
   - Aesthetic library (save visual styles, apply to any plugin)
   - Versioning (v1.0 → v1.1)
   - Troubleshooting docs
   - Multiple autonomy levels
   - Design sync

4. **Enable risk-free exploration**
   - Leverage Claude Code checkpointing
   - Make trying options safe
   - Users learn by doing, not fearing

5. **Make the journey feel personal**
   - Remember user preferences (saved aesthetics)
   - Offer discovered features naturally
   - Give control over workflow
   - Make it fun, not procedural

6. **Use inline numbered menus, not restrictive dialogs**
   - Inline numbered lists support unlimited options
   - Can show 11 plugins, not just 4
   - Dynamic lists (plugins, aesthetics, templates)
   - More readable and scannable
   - No artificial constraints from tool limitations

---

## Success Metrics

**Quantitative:**
- Users discover ≥3 features without reading docs
- 80%+ of users try "Other" exploration option at least once
- Users checkpoint and rewind to try alternatives

**Qualitative:**
- Users say "I didn't know it could do that!" (discovery)
- Users say "This feels like it knows me" (personalization)
- Users say "This is fun" (playfulness)
- Users recommend it to others (word of mouth)

---

## See Also

- `05-routing-architecture.md` - Interactive Decision System (complete implementation)
- `14-design-decisions.md` - Why interactive menus over yes/no prompts
- `01-executive-summary.md` - Core philosophy summary

---

## Related Procedures

This philosophy is implemented through:

- `procedures/core/interactive-decision-system.md` - Interactive decision menu implementation patterns
- `procedures/core/checkpoint-system.md` - Claude Code checkpointing usage patterns
- `procedures/skills/plugin-workflow.md` - Progressive disclosure in 7-stage workflow
- `procedures/skills/plugin-ideation.md` - Adaptive brainstorming implementation
- `procedures/skills/design-sync.md` - Brief ↔ mockup validation workflow
- `procedures/skills/ui-mockup.md` - UI design workflow with progressive disclosure
- `procedures/skills/ui-template-library.md` - Aesthetic library system for design reuse
