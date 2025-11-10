## Design Decisions

**Guiding Philosophy:** The system should be fun, personal, and discoverable. Users learn by playing, not by reading manuals. Every decision point is an opportunity for discovery.

### Why These Abstractions?

**1. Dispatcher Pattern (vs. Monolithic Skills)**

**Alternative:** Keep implementation in main skill context (v1 approach)

**Chosen:** Dispatcher spawns subagents for implementation

**Rationale:**
- **Context management:** Subagents run in fresh contexts, preventing accumulation
- **Single-session creation:** No manual `/continue` needed
- **Isolation:** Stages don't pollute each other's context
- **Parallel potential:** Future optimization could run independent agents in parallel
- **Debugging:** Easier to isolate which stage failed

**Trade-offs accepted:**
- More complex architecture
- More agent definitions to maintain
- Slightly slower (agent spawn overhead)
- Benefit outweighs cost for plugins that take >2 hours

**2. Contract-Driven Development (vs. Incremental)**

**Alternative:** Generate specifications as you go (v1 approach)

**Chosen:** parameter-spec.md + architecture.md required before Stage 1

**Rationale:**
- **Zero drift:** UI, DSP, plan all built from same contracts
- **Early validation:** User approves specs before any C++ code written
- **Better planning:** Stage 1 references concrete contracts, not assumptions
- **Catch issues early:** Parameter count mismatch found at planning, not GUI stage
- **Single source of truth:** Contracts are canonical, not derived

**Trade-offs accepted:**
- More upfront work (must finalize mockup before implementation)
- Less flexibility (can't easily change parameters mid-implementation)
- Benefit: Eliminates entire class of drift bugs

**3. Independent Validation (vs. Built-in Checks)**

**Alternative:** Stages self-validate (v1 approach)

**Chosen:** Separate validator agent verifies stage claims

**Rationale:**
- **Catches lies:** Stage claims "all parameters implemented" but validator checks it's true
- **Independent perspective:** Fresh eyes catch issues the implementer missed
- **Audit trail:** Validation results recorded separately
- **Consistency:** Same validation logic across all stages
- **Confidence:** User knows each stage was verified, not just claimed complete

**Trade-offs accepted:**
- Adds validation time (~30s per stage)
- More complexity (validator agent + stage agent)
- Benefit: Eliminates "thought it was done but wasn't" bugs

**4. Hook-Based Quality (vs. Manual Checks)**

**Alternative:** Remind user to check for issues (v1 approach)

**Chosen:** Hooks automatically validate code quality

**Rationale:**
- **Deterministic:** Hook always runs, never forgotten
- **Fast feedback:** Errors caught before build (seconds vs. minutes)
- **Enforcement:** Can't bypass quality gates (exit code 2 blocks)
- **Education:** Error messages teach best practices
- **Consistency:** Same rules enforced across all plugins

**Trade-offs accepted:**
- Setup overhead (write hook scripts)
- Maintenance (keep rules updated)
- Potential false positives (need escape hatch)
- Benefit: Eliminates entire class of quality issues

**5. Two-Phase UI Design (vs. Generate-All-Files)**

**Alternative:** Generate all 7 files for every mockup iteration (v1 approach)

**Chosen:** Design phase (yaml + test.html) → Finalization (7 files)

**Rationale:**
- **Fast iteration:** Browser preview in seconds, no build needed
- **Clean workspace:** Implementation files only when design locked
- **Clear state:** Design is exploratory until finalized
- **Reduced clutter:** Old iterations don't have unused C++ templates
- **User control:** Explicit finalization step (not accidental)

**Trade-offs accepted:**
- Two-step process (design, then finalize)
- Extra decision point (when to finalize)
- Benefit: Faster design iteration, cleaner file organization

**6. Interactive Decision Menus (vs. Implicit Yes/No)**

**Alternative:** Binary prompts at checkpoints: "Continue to Stage 4? [y/n]"

**Chosen:** Contextual numbered options at every decision point

**Rationale:**
- **Discovery:** Users learn features by encountering them, not reading docs
- **Control:** User shapes their journey, doesn't feel railroaded
- **Fun:** "Choose your own adventure" feels playful, not procedural
- **Safe exploration:** Claude Code checkpointing (Esc+Esc) makes trying options risk-free
- **Progressive disclosure:** Hidden capabilities (templates, versioning, troubleshooting docs) surface naturally
- **Reduces friction:** "4" to pause is faster than explaining "I want to stop here and save my progress"
- **Makes exits explicit:** "Pause here" option creates clean checkpoints mid-workflow

**Trade-offs accepted:**
- More options to present (design effort)
- Slightly longer prompts (5 options vs yes/no)
- Need to generate context-aware options
- Benefit: System becomes discoverable, fun, and personal—users learn by playing

**Example transformation:**

v1 (implicit):
```
Stage 3 complete. Continue? [y/n]
```

v2 (explicit discovery):
```
✓ Stage 3 complete

What's next?
1. Continue to Stage 4 (recommended)
2. Review Stage 3 code
3. Test current state
4. Pause here
5. Other
```

User discovers they can pause mid-workflow, test early, or review code—without reading documentation.

**7. Slash Commands for Entry (vs. Conversational Routing)**

**Alternative:** Pattern matching on natural language ("make me a", "build", "create")

**Chosen:** Explicit slash commands as entry points (`/implement`, `/dream`, `/improve`)

**Rationale:**
- **Discoverable:** Type "/" → IDE autocomplete shows ALL workflows instantly
- **Deterministic:** Same command always routes to same skill, no guessing
- **Fast:** No pattern matching, no intent parsing, direct invocation
- **Extensible:** New skill = new command, no routing logic changes needed
- **Reduces friction:** `/implement` faster than explaining "I want to create a new plugin"
- **Fits philosophy:** Commands discovered via autocomplete, features via contextual menus

**Trade-offs accepted:**
- Users must type "/" instead of natural language
- One extra keystroke
- Benefit: Instant discoverability, zero ambiguity, zero documentation needed

**Example:**

Conversational (fragile):
```
"make me a reverb" ✓ Works
"build me a reverb" ✓ Works
"I want a reverb" ??? Maybe works
"reverb plugin" ??? Might miss
→ User has to guess what phrases work
```

Slash command (deterministic):
```
/implement → Always works, always goes to plugin-workflow
Type "/" → See all 8 commands in autocomplete
→ User knows exactly what's available
```

### Why This Routing Approach?

**Slash commands → Skills (direct mapping) vs. Natural language pattern matching**

**Slash command approach:**
```bash
/implement → plugin-workflow (always)
/dream → plugin-ideation (always)
/improve → plugin-improve (always)
```

Simple, direct, maintainable.

**Old conversational approach (complex):**
```python
# Pattern matching on user input
triggers = extract_triggers("make me a reverb plugin")
# Returns: ["make", "reverb", "plugin"]

for skill in registered_skills:
    if any(trigger in skill.triggers for trigger in triggers):
        if check_preconditions(skill):
            matched_skills.append(skill)

if len(matched_skills) > 1:
    present_disambiguation_menu()  # Extra step!
```

Complex, fragile, requires disambiguation.

**Benefits:**
- Routing code: ~20 lines (command parser) vs ~200 lines (pattern matching)
- Zero false positives from ambiguous phrasing
- New skills integrate instantly (add command file)
- Users see capabilities immediately (autocomplete)

### Why This State Model?

**Single state machine (PLUGINS.md) vs. Multiple state files**

**Chosen:** Centralized lifecycle state in PLUGINS.md, with supporting state in specific locations

**Rationale:**
- **Single source of truth:** One place to check plugin status
- **Atomic transitions:** Status + code change in same commit
- **Easy querying:** Grep PLUGINS.md to see all plugin states
- **Clear ownership:** Skills manage lifecycle via explicit instructions to Claude

**Supporting state:**
- `.continue-here.md` - Session-specific, deleted when stage complete
- `CHANGELOG.md` - Version-specific, permanent
- Logs - Build-specific, temporary

**Alternative considered:** State in plugin directory only (e.g., `plugins/[Name]/STATUS`)

**Rejected because:**
- Harder to get overview of all plugins
- No single file to grep
- Status could drift if not updated atomically

### Configuration Defaults

**Extended Thinking Budget**

**Units:** Tokens (not messages or time)

**Default:** 10,000 tokens per agent invocation

**Rationale:**
- 10k tokens ≈ 7,500 words of chain-of-thought reasoning
- Sufficient for most plugin debugging/research tasks
- Prevents runaway costs on infinite reasoning loops
- Balances thoroughness vs cost

**Per-agent tuning:**
- troubleshooter: 15,000 (complex investigations)
- validator: 5,000 (pattern matching, less reasoning)
- ui-mockup: 8,000 (moderate creative reasoning)
- plugin-workflow: 10,000 (standard)

**When budget exceeded:**
- Agent stops thinking, returns best answer based on thinking so far
- Warning logged for review
- User can retry with higher budget if needed

**Agent Timeouts**

**Default:** 45 minutes (2700 seconds)

**Configuration:**
- Agent frontmatter (primary): `timeout: 2700`
- Settings override: `.claude/settings.json`

**Per-agent recommendations:**
- troubleshooter: 3600s (1h) - deep research takes time
- validator: 300s (5m) - pattern matching should be fast
- ui-mockup: 1800s (30m) - design iteration needs time
- plugin-workflow: 2700s (45m) - standard default
- build-automation: 600s (10m) - builds should be fast

**When timeout occurs:**
- Save partial work if available
- Log timeout with context
- Offer retry with longer timeout or manual continuation

---
