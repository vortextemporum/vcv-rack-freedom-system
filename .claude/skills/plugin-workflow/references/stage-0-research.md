# Stage 0: Research

**Context:** This file is part of the plugin-workflow skill.
**Invoked by:** Main workflow dispatcher when starting new plugin or resuming at Stage 0
**Purpose:** Understand what we're building before writing code through research and DSP architecture specification

---

**Goal:** Understand what we're building before writing code

**Duration:** 5-10 minutes

**Model Configuration:**
- Model: Opus (complex reasoning for algorithm comparison)
- Extended thinking: ENABLED
- Budget: 10000 tokens

## Actions

1. Read creative brief:

```bash
cat plugins/[PluginName]/.ideas/creative-brief.md
```

2. Identify plugin type and technical approach:

   - Audio effect, MIDI effect, synthesizer, or utility?
   - Input/output configuration (mono, stereo, sidechain)
   - Processing type (time-domain, frequency-domain, granular)

3. **Context7 JUCE Documentation Lookup** (CRITICAL):

   a. Resolve JUCE library ID:
   ```
   Use: mcp__context7__resolve-library-id("JUCE")
   Returns: Context7-compatible library ID (e.g., "/juce-framework/JUCE")
   ```

   b. Get JUCE DSP documentation:
   ```
   Use: mcp__context7__get-library-docs(libraryID, topic="dsp modules", tokens=5000)
   Extract: Relevant juce::dsp classes for identified plugin type
   ```

   c. Document JUCE modules found:
   - List specific juce::dsp classes (e.g., juce::dsp::Gain, juce::dsp::IIR::Filter)
   - Note Context7 library references
   - Identify any missing algorithms (need custom DSP)

4. Research professional plugin examples:

   - Search web for industry leaders (FabFilter, Waves, UAD, etc.)
   - Document 3-5 similar plugins
   - Note sonic characteristics
   - Extract typical parameter ranges used

5. Research parameter ranges:

   - Industry-standard ranges for plugin type
   - Typical defaults (reference professional plugins)
   - Skew factors for nonlinear ranges

6. Check design sync (if mockup exists):
   - Look for `plugins/[Name]/.ideas/mockups/v*-ui.yaml`
   - If exists: Compare mockup parameters with creative brief
   - If conflicts: Invoke design-sync skill
   - Document sync results

## Output

Create `plugins/[PluginName]/.ideas/architecture.md` (DSP specification)

**CRITICAL:** Use the DSP architecture contract template from `assets/architecture.md`.

**Required sections:**
1. Title: `# DSP Architecture: [PluginName]`
2. Contract header: `**CRITICAL CONTRACT:** This specification is immutable...`
3. `## Core Components` - List each DSP component with JUCE class, purpose, parameters affected, configuration
4. `## Processing Chain` - ASCII diagram showing signal flow and parameter connections
5. `## Parameter Mapping` - Table mapping every parameter ID to DSP component and usage
6. `## Algorithm Details` - Implementation approach for each algorithm
7. `## Special Considerations` - Thread safety, performance, denormal protection, sample rate handling
8. `## Research References` - Context7 docs, professional plugins, technical resources

**How to populate from research:**

- **Similar Plugins research** → `## Research References` (Professional Examples section)
- **JUCE modules identified** → `## Core Components` (one subsection per component with structured format)
- **Parameter research** → `## Parameter Mapping` (table linking parameter IDs to components)
- **Technical approach** → `## Processing Chain` (ASCII diagram) + `## Algorithm Details`
- **Technical feasibility** → `## Special Considerations`

**Example Core Component entry:**
```markdown
### File Manager
- **JUCE Class:** `juce::AudioFormatManager`
- **Purpose:** Detect audio file formats and create readers
- **Parameters Affected:** station
- **Configuration:** registerBasicFormats() for WAV/AIFF/MP3/FLAC/OGG
```

**Example Processing Chain:**
```
MIDI In → Trigger Detection → Set Playback Position
                                        ↓
File Manager ← station parameter → AudioFormatReader
      ↓
Resampler ← speed_pitch parameter
      ↓
Volume Control ← volume parameter
      ↓
Output
```

See `assets/architecture.md` for complete template structure.

## Create Handoff File

Create `plugins/[PluginName]/.continue-here.md`

**Format:**

```yaml
---
plugin: [PluginName]
stage: 0
status: in_progress
last_updated: [YYYY-MM-DD HH:MM:SS]
---

# Resume Point

## Current State: Stage 0 - Research

Research phase complete. Ready to proceed to planning.

## Completed So Far

**Stage 0:** ✓ Complete
- Plugin type defined
- Professional examples researched
- DSP feasibility verified
- Parameter ranges researched

## Next Steps

1. Stage 1: Planning (calculate complexity, create implementation plan)
2. Review research findings
3. Pause here

## Context to Preserve

**Plugin Type:** [Effect/Synth/Utility]
**Processing:** [Approach]
**JUCE Modules:** [List]

**Files Created:**
- plugins/[PluginName]/.ideas/architecture.md
```

## Update PLUGINS.md

1. Check if entry exists:
```bash
grep "^### ${PLUGIN_NAME}$" PLUGINS.md
```

2. If NOT found, create initial entry:

Use Edit tool to add to PLUGINS.md:
```markdown
### [PluginName]
**Status:** 💡 Ideated
**Type:** [Audio Effect | MIDI Instrument | Synth]
**Created:** [YYYY-MM-DD]

[Brief description from creative-brief.md]

**Lifecycle Timeline:**
- **[YYYY-MM-DD]:** Creative brief created

**Last Updated:** [YYYY-MM-DD]
```

3. Update status to Stage 0:

Use Edit tool to change:
```markdown
**Status:** 💡 Ideated → **Status:** 🚧 Stage 0
```

4. Add timeline entry:

Use Edit tool to append to Lifecycle Timeline:
```markdown
- **[YYYY-MM-DD] (Stage 0):** Research completed
```

5. Update Last Updated field

## Git Commit

```bash
git add plugins/[PluginName]/.ideas/architecture.md plugins/[PluginName]/.continue-here.md PLUGINS.md
git commit -m "$(cat <<'EOF'
feat: [PluginName] Stage 0 - research complete

🤖 Generated with Claude Code

Co-Authored-By: Claude <noreply@anthropic.com>
EOF
)"
```

Display commit hash:
```bash
git log -1 --format='✓ Committed: %h - Stage 0 complete'
```

## Decision Menu

Present inline numbered list (NOT AskUserQuestion):

```
✓ Stage 0 complete: research documented

What's next?
1. Continue to Stage 1 (recommended)
2. Review research findings
3. Improve creative brief based on research
4. Run deeper investigation (deep-research skill) ← Discover JUCE troubleshooting
5. Pause here
6. Other

Choose (1-6): _
```

Wait for user response. Handle:
- Number (1-6): Execute corresponding option
- "continue" keyword: Execute option 1
- "pause" keyword: Execute option 5
- "review" keyword: Execute option 2
- "other": Ask "What would you like to do?" then re-present menu

---

**Return to:** Main plugin-workflow orchestration in `SKILL.md`
