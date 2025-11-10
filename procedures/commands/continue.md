# /continue

**Purpose:** Resume plugin development from where you left off.

---

## Syntax

```bash
/continue                    # Interactive menu - list all resumable plugins
/continue [PluginName]       # Resume specific plugin directly
```

## What It Does

Automatically loads context from `.continue-here.md` handoff files and resumes your workflow exactly where you paused. Works across the entire plugin lifecycle: ideation, mockup creation, implementation, and improvements.

## Universal Handoff System

Searches for handoff files in `plugins/[Name]/`:
- **Ideation/Mockup** → `plugins/[Name]/.ideas/.continue-here.md`
- **Implementation** → `plugins/[Name]/.continue-here.md`
- **Improvement** → `plugins/[Name]/.continue-here.md`

All handoff files use compatible formats and resume seamlessly.

## How It Works

### Without Plugin Name

Presents interactive menu of resumable work:

```
Which plugin would you like to resume?

1. MicroGlitch
   Stage 3 (DSP) • Active development • 2 hours ago

2. JustForKicks
   Mockup v1 ready • Ready to build • 1 day ago

3. Tom808
   v1.2.0 installed • Recently improved • 1 week ago
```

**Context indicators:**
- `Stage N (Name) • Active development` - Implementation in progress
- `Mockup vN ready • Ready to build` - Design phase complete
- `Creative brief complete • Not started` - Ideation done
- `vX.Y.Z installed • Recently improved` - Recent improvements

### With Plugin Name

Skips menu and loads context directly:

```bash
/continue MicroGlitch
```

Immediately loads handoff file and proposes next steps.

## What It Loads

**Context automatically loaded:**
- Handoff file content (current state, completed work, next steps)
- Recent commits for this plugin
- Source files mentioned in handoff
- Research notes (if Stage 0-1)
- UI mockups (if applicable)

**Skill output:**
```
Resuming MicroGlitch at Stage 4...

Summary of completed work:
- Foundation set up (Stage 2)
- Plugin loads in DAW (Stage 3)
- Started DSP architecture

Current status:
Working on Stage 4 (DSP implementation).
Core audio processing structure in place.

Next steps:
1. Implement glitch effect algorithms
2. Add parameter modulation system

Ready to continue?
```

## After Resuming

**If stage completes successfully:**
```
✓ Stage 4 complete: DSP implementation
✓ Committed: feat: MicroGlitch Stage 4 - DSP (abc1234)

Continue to Stage 5 (GUI)?
```

**If error occurs:**
```
Error at Stage 4: [description]

Options:
1. Investigate (triggers deep-research)
2. Show me the code
3. Show full output
4. I'll fix it manually

After manual fix, say "resume automation"
```

## When No Handoffs Exist

```
No resumable work found.

Handoff files are created throughout the plugin lifecycle:
- After ideation (creative brief complete)
- After mockup creation (design ready)
- During implementation (plugin-workflow stages)
- After improvements (version releases)

Start new work:
- /dream - Explore plugin ideas
- /implement - Build new plugin
```

## Plugin-Specific Errors

**Plugin doesn't have handoff:**
```
ClapPlugin808 doesn't have a handoff file.

Possible reasons:
- Plugin is already complete (Stage 6 done)
- Plugin hasn't been started yet
- Development finished and handoff removed

Check status: Look in PLUGINS.md
Modify complete plugin: /improve ClapPlugin808
Start new plugin: /implement ClapPlugin808
```

## Routes To

**Skill:** `context-resume`

The skill handles:
- Reading `.continue-here.md` files
- Parsing current stage and status
- Summarizing completed work
- Loading relevant context
- Proposing next steps
- Continuing workflow from checkpoint

## Examples

**Example 1: List and select**
```
User: /continue
Assistant: [Lists 2 plugins with resumable work]

          1. MicroGlitch - Stage 3, last updated 2 hours ago
          2. GranularGlitch - Stage 1, last updated yesterday

User: [Selects 1]
Assistant: Resuming MicroGlitch at Stage 3...
          [Shows summary and next steps]
```

**Example 2: Direct resume**
```
User: /continue MicroGlitch
Assistant: [Loads handoff immediately]

          Resuming MicroGlitch at Stage 3 (DSP)...
          Completed: Foundation, shell
          Next: Implement audio processing

          Ready to continue?
```

**Example 3: Resume after mockup**
```
User: /continue JustForKicks
Assistant: [Loads .ideas/.continue-here.md]

          Resuming JustForKicks from mockup phase...

          Design complete:
          - Creative brief written
          - UI mockup v1 created
          - 7 mockup artifacts generated

          Ready to start implementation with /implement?
```

**Example 4: Delegated from /implement**
```
User: /implement MicroGlitch
Assistant: MicroGlitch is in-progress.

          Delegating to /continue...

          [Loads handoff and continues from checkpoint]
```

## Natural Language Equivalent

These phrases work the same:
- "Continue MicroGlitch"
- "Resume MicroGlitch"
- "Pick up where we left off on MicroGlitch"
- "Keep working on MicroGlitch"

## Related Commands

- `/implement` - Start new plugin from scratch
- `/improve` - Modify completed plugin
- `/dream` - Explore ideas without implementing

## When To Use

**Use /continue when:**
- You paused mid-development
- Starting a new chat session
- Want to see status before deciding what to do
- Returning after hours/days away

**Don't use /continue when:**
- Starting brand new plugin (use `/implement`)
- Making changes to complete plugin (use `/improve`)
- Just exploring ideas (use `/dream`)

## Tips

**Quick resume:** Use `/continue [Plugin]` when you know which plugin.

**Browse options:** Use `/continue` alone to see what's resumable.

**Check before continuing:** Handoff shows what's done and what's next—review before proceeding.

**Fresh sessions:** Handoff files preserve context perfectly across sessions.

## Output

After successful continuation, work proceeds exactly as if you never paused. Stage boundaries still create commits, tests still run, and handoff updates automatically.
