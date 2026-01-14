# Genesis - Creative Brief

## Overview

**Plugin Name:** Genesis
**Plugin Slug:** Genesis

### Modules

| Module | Type | HP Width |
|--------|------|----------|
| GenesisMono | Monophonic Oscillator | 8 HP |
| GenesisPoly | Polyphonic Oscillator | 10 HP |

**Module Count:** 2

## Vision

Genesis is a pair of digital oscillators that capture the distinctive sound of 16-bit era gaming consoles, particularly the Sega Genesis/Mega Drive's YM2612 FM chip. It delivers that crunchy, lo-fi digital character beloved by chiptune artists and retro gaming enthusiasts while offering modern modular synthesis capabilities.

**GenesisMono** is a compact, CPU-efficient monophonic version ideal for bass lines and leads.

**GenesisPoly** is the full polyphonic version (16 voices) for chords, pads, and complex patches.

## User Story

As a musician who loves retro gaming soundtracks and chiptune aesthetics, I want an oscillator that authentically recreates the sound of 16-bit era consoles so that I can create nostalgic, characterful sounds in my modular patches - from aggressive FM bass to shimmering bit-crushed pads.

## Sonic Character

- **Crunchy**: Intentional digital artifacts from bit reduction and sample rate limiting
- **Aggressive**: FM synthesis capable of metallic, bell-like, and growling tones
- **Nostalgic**: Immediately evokes 90s gaming soundtracks
- **Versatile**: From pure retro chip sounds to modern hybrid textures

## Key Features

### 1. Classic Waveforms
- Sine, Triangle, Saw, Square, Pulse (variable width)
- Noise generator (white/pink)
- All waveforms can be bit-crushed

### 2. Bit Crusher Section
- **Bit Depth**: 16-bit down to 1-bit reduction
- **Sample Rate**: Full rate down to ~1kHz for extreme aliasing
- Authentic digital degradation, not just filtering

### 3. 2-Operator FM Synthesis
- Carrier + Modulator architecture (like YM2612)
- FM Amount control with CV input
- Modulator ratio control (harmonic relationships)
- Classic FM bell and bass tones

### 4. Full Polyphony
- Up to 16 polyphonic voices
- Per-voice tracking of all parameters
- Works with polyphonic sequencers and keyboards

## Target Use Cases

1. **Chiptune/Retro**: Authentic 16-bit gaming sounds
2. **FM Bass**: Punchy, aggressive bass lines
3. **Metallic Leads**: Bell-like FM tones
4. **Lo-fi Textures**: Bit-crushed ambient pads
5. **Harsh Noise**: Extreme bit reduction for industrial sounds

## Reference Sounds

- Sega Genesis game soundtracks (Streets of Rage, Sonic)
- YM2612 FM chip characteristics
- Commodore 64 SID-style grit
- Modern chiptune artists (Chipzel, Disasterpeace)

## Panel Aesthetic

- Dark background with bright accent colors (Sega blue/red)
- Retro pixel-art inspired graphics (optional)
- Clear labeling with vintage digital display aesthetic
- Compact but readable layout

## Technical Notes

- V/Oct tracking for all waveforms
- Hard sync input for aggressive timbres
- Linear FM input for classic FM sounds
- Gate/trigger for potential envelope integration
