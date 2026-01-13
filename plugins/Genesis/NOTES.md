# Genesis Notes

## Status
- **Current Status:** 🚧 Stage 0
- **Version:** N/A
- **Type:** VCV Rack Oscillator Plugin

## Modules

| Module | Description | Tags |
|--------|-------------|------|
| GenesisMono | Monophonic 16-bit era oscillator with FM, bit crushing, and hard sync (8 HP) | Oscillator, VCO |
| GenesisPoly | Polyphonic 16-bit era oscillator with FM, bit crushing, and hard sync (10 HP, 16 voices) | Oscillator, VCO, Polyphonic |

## Lifecycle Timeline

- **2026-01-13 (Stage 0):** Research & Planning complete - Architecture and plan documented (Complexity 5.0)

## Known Issues

None

## Description

Genesis is a pair of digital oscillators that capture the distinctive sound of 16-bit era gaming consoles, particularly the Sega Genesis/Mega Drive's YM2612 FM chip. It delivers that crunchy, lo-fi digital character beloved by chiptune artists and retro gaming enthusiasts while offering modern modular synthesis capabilities.

**GenesisMono** is a compact, CPU-efficient monophonic version ideal for bass lines and leads.

**GenesisPoly** is the full polyphonic version (16 voices) for chords, pads, and complex patches.

## DSP Features

1. **Classic Waveforms:** Sine, Triangle, Saw, Square, Pulse (variable width), Noise
2. **Bit Crusher:** 16-bit down to 1-bit reduction for authentic digital degradation
3. **Sample Rate Reduction:** Full rate down to ~1kHz for extreme aliasing
4. **2-Operator FM Synthesis:** Carrier + Modulator architecture (YM2612-inspired)
5. **Hard Sync:** Phase reset on external trigger for aggressive timbres
6. **PolyBLEP Anti-Aliasing:** Band-limited waveforms (saw/square/pulse)
7. **V/Oct Pitch Tracking:** Standard 1V/octave with fine tuning
8. **Polyphonic Processing:** Up to 16 voices (GenesisPoly only)

## Parameters

**Controls:**
- FREQ: Frequency (-4 to +4 V, ±4 octaves from C4)
- FINE: Fine tuning (-0.5 to +0.5 semitones)
- WAVE: Waveform selection (Sine, Triangle, Saw, Square, Pulse, Noise)
- PW: Pulse width (0.1 to 0.9, when Pulse selected)
- BITS: Bit depth (1-16 bits)
- RATE: Sample rate reduction (0.0 = ~1kHz, 1.0 = full rate)
- FM AMT: FM modulation amount (0.0-1.0)
- FM RATIO: FM modulator:carrier ratio (0.5:1 to 8:1)
- LEVEL: Output level (0-100%)

**Inputs:**
- V/OCT: 1V/octave pitch CV (polyphonic)
- FM: FM amount CV modulation (polyphonic)
- SYNC: Hard sync trigger (polyphonic)
- BITS CV: Bit depth CV modulation

**Outputs:**
- OUT: Main audio output (±5V, polyphonic)

**Lights:**
- ACTIVE: Activity indicator (green LED)

## Panel Design

- **GenesisMono:** 8 HP width, compact layout
- **GenesisPoly:** 10 HP width, "POLY" label
- **Aesthetic:** Dark background with bright accent colors (Sega blue/red), retro pixel-art inspired graphics

## Implementation Plan

**Complexity Score:** 5.0 (capped from 9.8)
**Strategy:** Phased implementation

**GenesisMono (4 DSP Phases):**
1. Basic oscillator (sine/triangle, V/Oct tracking)
2. Anti-aliasing (saw/square/pulse/noise with PolyBLEP)
3. FM synthesis & hard sync
4. Bit crushing & sample rate reduction

**GenesisPoly (2 DSP Phases):**
1. Polyphonic voice management (16 channels)
2. Polyphonic testing & optimization

## Technical Details

**Architecture:**
- PolyBLEP anti-aliasing for saw/square/pulse waveforms
- Linear phase modulation (YM2612-style FM, not DX7-style)
- Sample-and-hold downsampling for sample rate reduction
- Uniform quantization for bit depth reduction
- Per-channel state arrays for polyphonic processing

**Performance (estimated):**
- Per-voice CPU: ~18-21% single core
- 16 voices: ~300-350% single core (VCV Rack handles multi-threading)

**References:**
- Bogaudio VCO (PolyBLEP reference)
- Venom VCO Lab (FM + sync reference)
- YM2612 datasheets (FM ratios and phase modulation)
- TAL-Bitcrusher (bit crushing parameters)

## Target Use Cases

1. **Chiptune/Retro:** Authentic 16-bit gaming sounds
2. **FM Bass:** Punchy, aggressive bass lines
3. **Metallic Leads:** Bell-like FM tones
4. **Lo-fi Textures:** Bit-crushed ambient pads
5. **Harsh Noise:** Extreme bit reduction for industrial sounds

## Reference Sounds

- Sega Genesis game soundtracks (Streets of Rage, Sonic)
- YM2612 FM chip characteristics
- Commodore 64 SID-style grit
- Modern chiptune artists (Chipzel, Disasterpeace)
