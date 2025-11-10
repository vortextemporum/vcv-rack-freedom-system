## Routing Architecture

**Core Philosophy:** Make every decision point an opportunity for discovery. Users learn capabilities by encountering contextual options, not by reading documentation.

### Entry Points: Slash Commands

**All workflows start with explicit slash commands.** No pattern matching, no guessing intent.

#### Command Structure

```bash
# Primary entry points (journeys start here)
/dream [concept]           # Ideation only, no implementation
/implement [plugin]        # Full workflow from concept to installed
/improve [plugin]          # Modify existing plugin
/continue [plugin?]        # Resume from checkpoint

# Supporting commands
/test [plugin]             # Run automated tests
/install [plugin]          # Install to system folders
/mockup [plugin]           # UI design workflow
/research [topic]          # Deep JUCE research

# Utility
/status                    # Show all plugins and states
/help                      # Quick reference
```

#### Request Flow

```
User types /command [args]
    │
    ▼
┌─────────────────────────┐
│ Command Parser          │
│  Extract: command, args │
└─────────┬───────────────┘
          │
          ▼
┌─────────────────────────┐
│ Validate Plugin State   │
│  - PLUGINS.md exists?   │
│  - Plugin status valid? │
└─────────┬───────────────┘
          │
          ▼
┌─────────────────────────┐
│ Check Preconditions     │
│  Command-specific rules │
└─────────┬───────────────┘
          │
          ▼
┌─────────────────────────┐
│ Invoke Skill            │
│  Direct 1:1 mapping     │
└─────────────────────────┘
```

#### Command → Skill Mapping

**Slash commands instruct Claude which skill to invoke** - this is instructed routing, not automatic dispatch.

```markdown
# .claude/commands/implement.md
---
name: implement
description: Create plugin from concept to installed
---

When user runs /implement [PluginName], invoke the plugin-workflow skill.

Preconditions:
- Plugin doesn't exist OR status == "💡"

# .claude/commands/improve.md
---
name: improve
description: Modify existing plugin
---

When user runs /improve [PluginName], invoke the plugin-improve skill.

Preconditions:
- Plugin exists
- status == "✅" OR "📦"

# .claude/commands/dream.md
---
name: dream
description: Explore plugin ideas without implementing
---

When user runs /dream, present ideation options, then invoke plugin-ideation skill based on user choice.

Preconditions: none

# .claude/commands/continue.md
---
name: continue
description: Resume from checkpoint
---

When user runs /continue [PluginName?], load .claude/handoff.md context and resume the active workflow.

Preconditions:
- handoff file exists

# .claude/commands/test.md
---
name: test
description: Run automated tests
---

When user runs /test [PluginName], invoke the plugin-testing skill.

Preconditions:
- Plugin exists
- status != "💡"
```

**How it works:**
1. Slash command expands into conversation as text prompt
2. Claude reads the instruction (e.g., "invoke the plugin-workflow skill")
3. Claude uses Skill tool to load `.claude/skills/plugin-workflow/SKILL.md`
4. Skill content loads progressively (not loaded until invoked)
5. Claude follows the detailed workflow in the skill

#### Why Slash Commands

**Discoverable:**
```
User types "/"
→ IDE shows autocomplete with ALL commands
→ User sees available workflows instantly
→ No docs needed
```

**Deterministic:**
```
/implement DelayPlugin → Always plugin-workflow
/improve ReverbPlugin  → Always plugin-improve
Same input, same route, every time
```

**Fast:**
```
No pattern matching
No intent parsing
No disambiguation
Direct to skill
```

**Extensible:**
```
New skill? Add new command.
That's it. No routing logic changes.
```

### Interactive Decision System

**Every checkpoint in the workflow presents contextual options.** This makes the system discoverable, gives users control, and enables Claude Code checkpointing exploration (Esc+Esc to try different paths).

#### When to Present Options

**ALWAYS present options when:**
- User request is ambiguous/vague
- Multiple valid next actions exist
- Workflow reaches a natural checkpoint (stage boundary, completion, error)
- User could benefit from discovering hidden capabilities

**NEVER present options when:**
- Request is explicit and unambiguous ("implement DelayPlugin")
- Only one valid path exists
- Mid-stage with no meaningful alternatives

#### Decision Point Categories

**1. Entry Routing (Ambiguous Requests)**

```
User: "Work on ReverbPlugin"

What would you like to do with ReverbPlugin?

1. **Continue development** - Resume from last checkpoint
2. **Test it** - Load in DAW and verify
3. **Improve it** - Add features or fix issues
4. **Review code** - Examine implementation
5. **Other** - Describe your approach

Reply with a number.
```

**2. Stage Boundaries (After Each Stage Completes)**

```
✓ Stage 3 complete: Shell implementation

What's next?

1. **Continue to Stage 4** - DSP implementation (recommended)
2. **Review Stage 3 code** - Look at what was just built
3. **Test current state** - Load plugin in DAW now
4. **Pause here** - Save progress, do something else
5. **Other**

Reply with a number.
```

**Option 4 ("Pause here") is critical:**
- Explicitly updates handoff file
- Commits current state
- Creates clean checkpoint
- User knows exactly where to resume

**3. UI Mockup Workflows**

**After mockup iteration:**
```
✓ Mockup v2 created (7 files generated)

What's next?

1. **Iterate on this design** - Refine layout/colors/controls
2. **Finalize and proceed** - Lock design for Stage 5
3. **Save aesthetic as template** - Add to UI aesthetic library
4. **Start fresh** - New mockup from scratch
5. **Other**
```

**Starting UI design:**
```
Ready to design the UI for ReverbPlugin.

How would you like to start?

1. **Brainstorm together** - I'll ask questions about your vision
2. **Apply an aesthetic** - Use saved visual style (3 available)
3. **Upload a sketch** - Start from your mockup image
4. **Describe it directly** - Tell me exactly what you want
5. **Other**
```

**After creative brief:**
```
✓ Creative brief complete for FuzzPlugin

What's next?

1. **Create UI mockup** - Design the interface (recommended)
2. **Start implementation** - Skip mockup, build now
3. **Improve the brief** - Refine concept further
4. **Research similar plugins** - See what exists first
5. **Other**
```

**4. Error/Failure Scenarios**

**Build failure:**
```
Build failed: linker error in PluginProcessor.cpp

What would you like to do?

1. **Investigate** - Research the error and find root cause
2. **Show me the code** - Review what was generated
3. **Show build output** - See full error log
4. **I'll fix it manually** - Pause automation here
5. **Other**
```

**Validation warnings (non-critical):**
```
⚠️ Stage 4 validation: 2 warnings, 0 errors

What's next?

1. **Continue anyway** - Warnings are acceptable
2. **Review warnings** - See what triggered them
3. **Fix warnings first** - Address before continuing
4. **Pause and investigate** - Stop to review manually
5. **Other**
```

**Test failures:**
```
❌ Automated tests failed (2/5 passed)

What would you like to do?

1. **Debug failures** - Investigate what went wrong
2. **Review test output** - See detailed results
3. **Manual inspection** - Test in DAW yourself
4. **Continue anyway** - Tests may be overly strict
5. **Other**
```

**5. Subagent Decisions**

**Before spawning autonomous subagent:**
```
Ready to implement Stage 4 (DSP).

How should I proceed?

1. **Fully autonomous** - Let subagent complete all phases (recommended)
2. **Phase-by-phase** - Review after each sub-phase
3. **Guided** - I'll make key decisions, subagent implements
4. **Manual** - You implement, I assist
5. **Other**
```

**6. Investigation Results**

**After troubleshooting:**
```
Root cause identified: Missing juce::dsp module link

What's next?

1. **Apply the fix** - I'll implement the solution
2. **Show me the code** - Review before applying
3. **Try alternative** - Different approach
4. **I'll do it manually** - You explained it, I'll fix
5. **Other**
```

#### Option Generation Logic

**Context-based selection algorithm:**

1. **Check plugin state** (PLUGINS.md: 💡/🚧/✅/📦)
2. **Check handoff file** (in-progress work? where stopped?)
3. **Check git history** (what was last done?)
4. **Generate options:**
   - Primary action (most likely intent, marked "recommended")
   - Alternative approaches (different angles)
   - Investigation/review (understand first)
   - Manual control (pause and take over)
   - Other (free text for unexpected needs)

**State-specific option templates:**

| Plugin State | Primary Options |
|--------------|-----------------|
| 💡 Ideated | Implement, Improve idea, Create mockup, Research, Delete |
| 🚧 In-progress | Continue, Review code, Test current, Start over, Pause here |
| ✅ Working | Test, Improve, Install, Create template, Archive |
| 📦 Installed | Use in DAW, Uninstall, Update, Distribute, Archive |

#### Implementation: Inline Numbered Menus

**Use inline numbered lists instead of AskUserQuestion tool:**

**Why inline menus:**
- No 4-option limit (AskUserQuestion restricted to 4)
- Supports dynamic lists (11 plugins, 8 aesthetics, unlimited templates)
- More readable and scannable
- Faster for users to process
- No artificial constraints

**Format:**

```
✓ Stage 3 complete

What's next?
1. Continue to Stage 4 (recommended)
2. Review Stage 3 code
3. Test current state
4. Pause here
5. Other

Choose (1-5): _
```

**Usage in skills:**

```typescript
// Detect checkpoint
if (stageComplete && !explicitNextAction) {
  // Present inline numbered menu
  output(`
✓ Stage 3 complete

What's next?
1. Continue to Stage 4 (recommended)
2. Review Stage 3 code
3. Test current state
4. Pause here
5. Other

Choose (1-5): _
  `)

  const choice = await getUserInput()

  // Route based on number or keyword
  if (choice === "1" || choice.toLowerCase().includes("continue")) {
    continueToStage4()
  } else if (choice === "4" || choice.toLowerCase().includes("pause")) {
    updateHandoffFile()
    commitCurrentState()
    return "Development paused. Resume anytime with /continue"
  }
  // ... etc
}
```

**Dynamic lists (unlimited options):**

```
Which plugin would you like to work on?

1. TapeDelay (Stage 4 - DSP in progress)
2. VintageComp (Stage 6 - ready to install)
3. SpringReverb (Stage 1 - planned)
4. 808Clap (✅ Installed)
5. FilterBank (💡 Ideated)
6. ModDelay (Stage 3 - shell complete)
7. ChorusEffect (Stage 5 - GUI in progress)
8. Create new plugin
9. Other

Choose (1-9): _
```

**AskUserQuestion should only be used for:**
- Simple binary choices (yes/no)
- Preference selections where 2-4 options is truly sufficient
- Non-dynamic lists

**Most decision menus should use inline numbered lists.**

#### Progressive Disclosure Pattern

**Features are discovered through use, not documentation:**

**Session 1:**
```
User sees: "3. Save as template - Add to UI template library"
User thinks: "What's that?"
User tries it: Saves mockup to templates/
User learns: "Oh, I can reuse designs!"
```

**Session 2:**
```
User sees: "2. Apply an aesthetic - Use saved visual style (3 available)"
User thinks: "Oh yeah, that warm vintage style!"
User applies: New layout generated with same aesthetic
User learns: "It adapted the style to my 8 parameters!"
```

**Session 3:**
```
User confidently: Builds plugin family with unified visual brand
System feels: Personal, generates appropriate layouts using saved aesthetics
```

**Hidden capabilities surfaced:**
- UI aesthetic library (save/reuse visual styles across plugins)
- Design sync (brief ↔ mockup)
- Improvement versioning (v1.0 → v1.1 with backups)
- Troubleshooting documentation
- Resume from any checkpoint
- Multiple mockup iterations
- Aesthetic adaptation (same style, different layouts)

#### Risk-Free Exploration with Checkpointing

**Every user prompt creates a Claude Code checkpoint.**

**Exploration pattern:**
```
User: "3" (Save as template)
→ Claude saves mockup to template library
User: "Hmm, actually don't want that"
User: Esc + Esc → Rewind menu
User: "Restore code only" → Template file deleted, back to v2 mockup
User: "2" (Finalize and proceed) → Different path
```

**Benefit:** Users can safely try options without fear of breaking things. Makes exploration **fun** instead of risky.

#### Vagueness Detection

**Request is vague when it lacks:**
1. Specific action verb ("fix", "add", "test")
2. Clear target component/feature
3. Success criteria

**Examples:**

| Request | Vague? | Why |
|---------|--------|-----|
| "Work on ReverbPlugin" | YES | No action specified |
| "Fix the filter" | YES | Which filter? What's wrong? |
| "Test DelayPlugin" | NO | Clear action + target |
| "Add reverb tail parameter" | NO | Specific feature to add |
| "Help with FuzzPlugin" | YES | "Help" is too broad |

**Handling:**

```python
if is_vague(request):
    present_contextual_options(plugin_state, recent_history)
else:
    route_directly(request)
```

### Precondition Verification

**Before invoking workflow:**

```typescript
interface PreconditionCheck {
  plugin_exists: boolean
  plugin_status: PluginStatus
  required_files_exist: string[]
  state_valid: boolean
}

function verifyPreconditions(
  workflow: string,
  plugin: string
): PrerequisiteResult {
  const checks = {
    plugin-workflow: {
      plugin_not_exists: !pluginExists(plugin),
      OR_plugin_status: getStatus(plugin) === "💡"
    },
    plugin-improve: {
      plugin_exists: pluginExists(plugin),
      plugin_complete: ["✅", "📦"].includes(getStatus(plugin))
    },
    plugin-workflow-stage1: {
      creative_brief_exists: fileExists(`${plugin}/.ideas/creative-brief.md`),
      parameter_spec_exists: fileExists(`${plugin}/.ideas/parameter-spec.md`),
      architecture_exists: fileExists(`${plugin}/.ideas/architecture.md`)
    }
  }

  if (!checks[workflow].allPassing()) {
    return {
      allowed: false,
      reason: checks[workflow].failureReason(),
      required_action: checks[workflow].suggestFix()
    }
  }

  return { allowed: true }
}
```

**Example enforcement:**

```
User: /implement ReverbPlugin

plugin-workflow checks Stage 1 preconditions:
  ✓ creative-brief.md exists
  ✗ parameter-spec.md missing
  ✗ architecture.md missing

plugin-workflow blocks with message:
"Cannot proceed to Stage 1 - missing implementation contracts:

Required before implementation:
✓ creative-brief.md exists
✗ parameter-spec.md (generated when mockup finalized)
✗ architecture.md (generated by Stage 0)

Next step: Finalize a UI mockup to generate parameter-spec.md

Would you like to create a mockup now?"
```

### Error Handling Paths

**Build failure:**
```
foundation-agent subagent reports: build_result = "failed"
    ↓
Validator catches failure
    ↓
plugin-workflow presents 4 options:
  1. **Investigate** - Deep research via troubleshooter
  2. **Show code** - Display files with errors
  3. **Show output** - Full build log
  4. **Wait** - Manual fix, then "resume automation"
    ↓
User chooses option
    ↓
Execute chosen action
    ↓
If fixed: Continue workflow
If not fixed: Stay paused, wait for next instruction
```

**Validation failure:**
```
Validator subagent returns: status = "FAIL"
    ↓
plugin-workflow pauses
    ↓
Presents validator findings to user
    ↓
User can:
  - Fix manually and say "re-validate"
  - Re-run stage with validator feedback
  - Override and continue (not recommended)
```

---

## Related Procedures

This routing architecture is implemented through:

- `procedures/commands/dream.md` - `/dream` entry routing
- `procedures/commands/implement.md` - `/implement` entry routing
- `procedures/commands/improve.md` - `/improve` entry routing
- `procedures/commands/continue.md` - `/continue` entry routing
- `procedures/core/interactive-decision-system.md` - Decision menu implementation patterns
- `procedures/skills/plugin-workflow.md` - Precondition verification and stage routing
- `procedures/skills/ui-mockup.md` - UI mockup workflow with decision menus
- `procedures/skills/ui-template-library.md` - Aesthetic library discovery pattern

---
