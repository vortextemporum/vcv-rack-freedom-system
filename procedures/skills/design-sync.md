# design-sync

**Purpose:** Reconcile differences between creative brief and UI mockup, updating documentation to match practical design decisions.

---

## What It Does

Compares the original creative vision (brief) against implemented mockup design, identifies where they diverged, and updates the brief to reflect actual decisions made during mockup creation.

## The Problem

**Common scenario:**

Creative brief says:
```
UI Vision: 3 rotary knobs in horizontal row, dark theme
```

Mockup v2 actually has:
```
4 knobs in 2x2 grid, plus LED indicators, gradient background
```

**Issue:** Brief and mockup are now out of sync. Future references to the brief will be misleading.

## When Invoked

**Automatically offered by:**
- `ui-mockup` skill (after creating/updating mockup)

**Manually triggered:**
- "Sync creative brief with mockup"
- "Update brief to match mockup v[N]"

## Sync Process

### Step 1: Load Both Documents

**Reads:**
```
plugins/[PluginName]/.ideas/creative-brief.md
plugins/[PluginName]/.ideas/mockups/v[N]-ui.yaml
plugins/[PluginName]/.ideas/mockups/v[N]-ui.html
```

### Step 2: Compare

**Identifies differences in:**

**Layout changes:**
- Brief: "Horizontal row of knobs"
- Mockup: "2x2 grid layout"

**Component differences:**
- Brief: "3 knobs for Low/Mid/High"
- Mockup: "3 vertical sliders + 4 rotary knobs"

**Visual style changes:**
- Brief: "Dark minimalist theme"
- Mockup: "Brushed aluminum with orange accents"

**Parameter additions/removals:**
- Brief: "Decay parameter for each tom"
- Mockup: "Added Tone, Tune, Drive, Volume global controls"

**Special elements:**
- Brief: "Simple controls only"
- Mockup: "Added LED indicators for voice activity"

### Step 3: Categorize Changes

**Design decisions** (intentional improvements):
- Layout changed for better ergonomics
- Added visual feedback elements
- Color scheme refined during iteration

**Discovered requirements** (missing from brief):
- Needed master volume control
- Drive parameter for saturation
- LED indicators for clarity

**Scope adjustments** (added features):
- Test buttons for each tom
- Visual grouping with sections
- Responsive sizing

### Step 4: Update Brief

Creates updated UI Vision section:

```markdown
## UI Vision (Updated from v2 Mockup)

### Window
- 700×450px fixed size (was: not specified)
- Brushed aluminum gradient background (was: "dark theme")

### Layout
- 3-column layout (LOW | MID | HIGH) (was: "horizontal row")
- Each column: label, LED, vertical slider (160px), TEST button
- Bottom row: 4 rotary knobs for master controls (was: not mentioned)

### Components

**Tom Controls (3 columns):**
- Vertical decay sliders (was: "3 rotary knobs")
- Range: 50-1000ms
- Orange track color (#FF8C00)

**LED Indicators (added):**
- 12×12px with glow effect
- Shows active voice state
- Orange when triggered

**Master Controls (added):**
- Tone, Tune, Drive, Volume
- 70px diameter rotary knobs
- Orange indicator line

### Visual Style
- TR-808 drum machine aesthetic (refined)
- Metallic gray (#C0C0C0 to #D3D3D3)
- Orange accents (#FF8C00)
- High contrast labels
```

### Step 5: Document Rationale

Adds section explaining changes:

```markdown
## Design Evolution

Changes from original brief to mockup v2:

**Vertical sliders instead of rotary knobs:**
Rationale: Decay time is primary control, vertical position
gives immediate visual feedback of setting.

**Added LED indicators:**
Rationale: Visual confirmation of which toms are currently
sounding improves usability during performance.

**Added master controls:**
Rationale: Tone/Tune/Drive/Volume needed for tonal shaping,
not just per-tom decay.

**Brushed aluminum aesthetic:**
Rationale: Better matches TR-808 hardware authenticity than
generic dark theme.
```

### Step 6: Preserve Original

Doesn't delete original vision—adds "Evolution" section:

```markdown
## Original Vision

[Original UI Vision section preserved here]

## Design Evolution

[Changes documented here]

## Current Design (v2)

[Updated specifications here]
```

Full lineage preserved.

## What Gets Updated

**Sections updated in creative brief:**
- UI Vision → Reflects current mockup
- Parameters & Controls → Matches implemented controls
- Visual Style → Matches mockup colors/layout

**Sections preserved:**
- Core Concept (unchanged)
- Sonic Goals (unchanged unless mockup revealed new requirements)
- Use Cases (unchanged)

**Sections added:**
- Design Evolution (new section documenting changes)

## Integration

**Called by:**
- `ui-mockup` skill (offers after mockup creation)

**Reads:**
- Creative brief
- Mockup YAML spec
- Mockup HTML (for visual style)

**Updates:**
- Creative brief (preserves original, adds evolution)

**Does NOT change:**
- Mockup files (source of truth remains unchanged)

## Example: Before and After

**Creative Brief (Original):**
```markdown
## UI Vision
Simple interface with 3 rotary knobs for decay control.
Dark theme with orange accents.
```

**Creative Brief (After design-sync):**
```markdown
## Original Vision
Simple interface with 3 rotary knobs for decay control.
Dark theme with orange accents.

## Design Evolution
v2 mockup refined the approach based on practical UX considerations:
- Changed to vertical sliders for better decay visualization
- Added LED indicators for voice activity feedback
- Added master control section (Tone/Tune/Drive/Volume)
- Refined to brushed aluminum aesthetic (TR-808 authenticity)

## Current Design (v2)
### Layout
3-column layout (LOW | MID | HIGH) with:
- Vertical decay sliders (160px tall)
- LED indicators (12×12px, orange glow)
- TEST buttons for auditioning

Master section:
- 4 rotary knobs (70px): Tone, Tune, Drive, Volume
- Horizontal row at bottom

### Visual Style
- Brushed aluminum gradient (#C0C0C0 to #D3D3D3)
- Orange accents (#FF8C00)
- TR-808 drum machine aesthetic
```

Now brief accurately reflects what was actually designed.

## When NOT to Use

**Don't sync if:**
- Mockup is experimental (v1, still iterating)
- Brief already matches mockup
- Changes are minor cosmetic tweaks

**DO sync if:**
- Mockup is approved for implementation
- Significant differences exist
- About to start Stage 5 (GUI implementation)

## Best Practices

**Sync before Stage 5:** Ensures GUI implementation references accurate specs

**Preserve lineage:** Always keep original vision, add evolution section

**Explain decisions:** Document WHY changes were made, not just WHAT changed

**Version references:** Clearly mark which mockup version the sync reflects

**One-time operation:** Sync once per mockup version, not continuously

## Output

```
✓ Synced creative brief with mockup v2

Changes documented:
- Layout: Horizontal row → 3-column grid
- Controls: Rotary knobs → Vertical sliders
- Added: LED indicators, master controls
- Visual: Dark theme → Brushed aluminum

Updated: plugins/Tom808/.ideas/creative-brief.md

Original vision preserved in "Original Vision" section.
Rationale documented in "Design Evolution" section.
```

Brief now matches implemented design.
