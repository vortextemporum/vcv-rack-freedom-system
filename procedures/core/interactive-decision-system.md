# interactive-decision-system

**Purpose:** Present contextual, numbered option menus at every checkpoint to enable progressive feature discovery and user-directed workflows.

---

## Core Principle

**Every checkpoint is a "choose your own adventure" moment** where users discover capabilities organically rather than reading documentation. The system presents relevant options based on current context, and users learn by trying options risk-free (via Claude Code checkpointing).

## When To Use

**At every workflow boundary:**
- After completing a stage/phase
- Before starting a new workflow
- When encountering errors or failures
- After creating/modifying files
- When multiple paths are possible

**NOT for:**
- Simple yes/no confirmations (just ask inline)
- Single-path workflows with no alternatives
- Error messages that require immediate fixes

## Decision Menu Format

### Standard Inline Format

Use numbered lists with clear descriptions (NOT AskUserQuestion tool):

```
✓ [Completion statement]

What's next?
1. [Primary action] (recommended)
2. [Secondary action]
3. [Discovery feature] ← User discovers new capability
4. [Alternative path]
5. Other

Choose (1-5): _
```

**Why inline instead of AskUserQuestion:**
- Can show more than 4 options (AskUserQuestion limited to 4)
- Dynamic lists (plugins, templates, etc. may exceed 4)
- More readable with descriptions
- Faster to scan
- No artificial constraints

### Key Elements

**Completion Statement:**
```
✓ Stage 3 complete: shell loaded in DAW
✓ Mockup v2 created
✓ Build successful
✓ Template saved: "Vintage Tape (3 Knobs)"
```

Shows what was just accomplished.

**Option Structure:**
```
[Number]. [Action verb] [object] [(optional: context/benefit)]
```

Examples:
- "1. Continue to Stage 4 (DSP implementation)"
- "2. Review Stage 3 code"
- "3. Save as template ← User discovers template library"
- "4. Pause here"

**Recommendation Markers:**
- `(recommended)` - Suggested next step for typical workflow
- `← User discovers [feature]` - Comments highlighting hidden features
- `(required)` - Must be done before proceeding

**"Other" Option:**
Always include as last option for free-form requests.

## Context-Aware Option Generation

### Plugin State → Relevant Options

**After Stage 3 (Shell):**
```
✓ Stage 3 complete: shell loaded in DAW

What's next?
1. Continue to Stage 4 (recommended)
2. Review Stage 3 code
3. Test loading in different DAW
4. Pause here
5. Other
```

**After Stage 6 (Validation):**
```
✓ Stage 6 complete: validation passed

What's next?
1. Install plugin to system folders (recommended)
2. Create another plugin
3. Document this plugin
4. Share plugin (export build)
5. Other
```

**After Mockup Creation:**
```
✓ Mockup v2 created

What's next?
1. Iterate on this design
2. Finalize and proceed to implementation
3. Save as template ← Add to UI template library
4. Start fresh (new v3)
5. Test in browser (open v2-ui-test.html)
6. Other
```

### Build Failure → Investigation Options

```
✗ Build failed: linker error in PluginProcessor.cpp:45

What would you like to do?
1. Investigate ← Run deep-research to find root cause
2. Show me the code
3. Show me the build output
4. I'll fix it manually (resume with "resume automation")
5. Other
```

### Starting Mockup → Design Options

```
Ready to design DelayPlugin UI.

How would you like to start?
1. Brainstorm together (conversational)
2. Apply a template (3 available) ← Saved designs
3. Upload a sketch
4. Describe it directly
5. Start from creative brief
6. Other
```

### Multiple Plugins → Dynamic List

```
Which plugin would you like to work on?

1. TapeDelay (Stage 4 - DSP in progress)
2. VintageComp (Stage 6 - ready to install)
3. SpringReverb (Stage 1 - planned)
4. 808Clap (✓ Installed)
5. FilterBank (💡 Ideated)
6. ModDelay (Stage 3 - shell complete)
7. ChorusEffect (Stage 5 - GUI in progress)
8. Create new plugin
9. Other

Choose (1-9): _
```

**Handle any number of items** - not limited to 4 options.

## Progressive Disclosure Pattern

### Session 1: First Encounter

```
User: /mockup DelayPlugin
→ Mockup created

System:
✓ Mockup v1 created

What's next?
1. Finalize and proceed
2. Iterate on this design
3. Save as template ← Add to UI template library
4. Other

User sees: "What's that? (option 3)"
User tries: 3
→ Template saved
User learns: "Oh, I can save designs!"
```

### Session 2: Recognition

```
User: /mockup ReverbPlugin
→ Starting mockup

System:
How would you like to start?
1. Brainstorm together
2. Apply a template (1 available) ← Saved designs
3. Describe it directly
4. Other

User thinks: "Oh yeah, that design I saved!"
User tries: 2
→ Template applied
User learns: Templates can be reused
```

### Session 3: Mastery

```
User: /mockup ChorusPlugin
System: "How would you like to start?"
User: 2 (without reading - knows templates exist)
→ Instantly applies template
→ Fast workflow
```

**No manual reading required.** Features discovered through use.

## Hidden Feature Surfacing

**Features users wouldn't know exist without menus:**

- UI template library (save/apply)
- Design sync (brief ↔ mockup validation)
- Improvement versioning (v1.0 → v1.1)
- Troubleshooting documentation capture
- Context resume from checkpoints
- Multiple mockup iterations
- Pause points for clean exits
- Plugin status inspection

**All discovered via:**
> "Option 3 looks interesting... what's that?" → Tries it → "Useful!"

## Option Ordering Strategy

**1. Primary action (recommended path):**
Most users should do this next.

**2. Secondary common actions:**
Frequently used alternatives.

**3. Discovery features (with markers):**
Hidden capabilities that teach the system.
Use `← User discovers [feature]` comments.

**4. Alternative paths:**
Less common but valid options.

**5. Escape hatches:**
"Pause here", "I'll do it manually", "Other"

## Implementation Guidelines

### Generate Options Dynamically

Based on:
- Current workflow state
- Plugin status (which stage)
- Available resources (templates, plugins, mockups)
- Error context (if failure occurred)
- User history (if known preferences)

### Keep Descriptions Concise

**Good:**
- "1. Continue to Stage 4 (DSP implementation)"
- "2. Save as template"
- "3. Test in browser"

**Too verbose:**
- "1. Continue to the next stage of the workflow which is Stage 4 where you will implement the DSP processing algorithms"

### Mark Discovery Moments

Add inline comments for features users might not know:

```
3. Save as template ← Add to UI template library
```

User thinks: "Template library? Didn't know that existed. Let me try..."

### Always Include "Other"

Allows free-form requests that don't fit options:

```
5. Other

User: "Actually, can you explain what Stage 4 involves?"
→ System provides explanation
```

### No Artificial Limits

If there are 11 plugins, show all 11. Don't paginate or restrict to 4.

## Integration Points

### Skills That Present Decision Menus

**plugin-workflow:**
- After each stage completion
- Before each stage start
- On build failures
- On test failures

**ui-mockup:**
- Before starting mockup (how to start)
- After mockup creation (what's next)
- After iteration (finalize or continue)

**build-automation:**
- On build failures (4 investigation options)
- After successful build (install or test)

**plugin-improve:**
- Before implementing changes (confirm scope)
- After changes complete (test or continue)

**plugin-ideation:**
- After creative brief (proceed to mockup?)
- When conflicts detected (sync or override)

### When NOT To Use Menus

**Simple confirmations:**
```
Plugin has uncommitted changes. Continue anyway? (y/n): _
```

Don't need a menu for binary choice.

**Immediate errors:**
```
Error: JUCE not found at /path/to/juce
Fix JUCE_PATH in CMakeLists.txt and try again.
```

No options - must fix error first.

**Sequential workflows with no alternatives:**
```
Copying JUCE library files...
Creating build directory...
Running CMake...
```

No decision points - just execute.

## Example Decision Trees

### After Stage Completion

```
Stage complete
│
├─ Continue to next stage (primary)
├─ Review current code
├─ Test current state
├─ Pause here (checkpoint)
└─ Other
```

### Build Failure

```
Build failed
│
├─ Investigate (deep-research)
├─ Show code
├─ Show output
├─ Manual fix (wait for "resume automation")
└─ Other
```

### Starting Mockup

```
Start mockup design
│
├─ Brainstorm together
├─ Apply template (if any exist)
│   └─ [List templates inline]
├─ Upload sketch
├─ Describe directly
└─ Other
```

### Plugin Selection

```
Which plugin?
│
├─ [Plugin 1] (status)
├─ [Plugin 2] (status)
├─ [Plugin 3] (status)
├─ ... (show all)
├─ Create new plugin
└─ Other
```

## Response Handling

### Number Input

```
User: 3
→ System executes option 3
```

### Out of Range

```
User: 12
System: Please choose 1-5 (or Other for custom request)
```

### "Other" Handling

```
User: Other
System: What would you like to do?
User: "Explain Stage 4 in detail"
→ System provides explanation
→ After explanation, re-present menu
```

### Keyword Shortcuts

Allow keyword responses:
```
User: "continue"
→ Same as choosing option 1 (if it's "Continue...")

User: "pause"
→ Same as "Pause here" option

User: "template"
→ Same as "Save as template" option
```

## Risk-Free Exploration

**Claude Code checkpointing (Esc+Esc) enables safe experimentation:**

```
User: 3 (Save as template)
→ Template saved
→ User reviews result
User: "Actually, don't want this"
User: Esc + Esc → Rewind checkpoint
User: Restore code only → Template gone
User: 2 (Different option) → New path explored
```

**Benefit:** Users can safely try any option without fear of breaking things. Makes exploration **fun** instead of risky.

## Teaching Moments

Every decision menu is a teaching opportunity:

**Teaches workflow structure:**
- User sees stages in sequence
- Understands optional vs required steps
- Learns checkpointing exists

**Teaches feature existence:**
- Discovers template library
- Learns about design sync
- Finds troubleshooting tools

**Teaches system flexibility:**
- Multiple valid paths
- Can pause and resume
- Manual overrides available

## Best Practices

**Present options immediately after completions:**
Don't make user ask "what's next?"

**Mark recommended paths:**
Guide new users, don't require expertise.

**Include discovery features:**
Surface hidden capabilities contextually.

**Handle unlimited options:**
Don't restrict to 4 - use inline numbered lists.

**Allow keyword shortcuts:**
"continue", "pause", "template", etc.

**Re-present after "Other":**
After handling custom request, show menu again.

**Keep descriptions scannable:**
One line per option, clear action verbs.

**Comment discovery moments:**
Use `← User discovers [feature]` markers.

## Success Criteria

Decision system is working when:
1. Users discover features without reading docs
2. Every checkpoint offers relevant, contextual options
3. No artificial 4-option limit
4. Users confidently navigate by number
5. "What's next?" is never needed (system asks first)
6. Risk-free exploration via checkpointing
7. System feels personal and discoverable
