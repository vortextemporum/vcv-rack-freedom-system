# Genesis - Parameter Specification

## Parameters

### FREQ_PARAM
- **Type:** Float
- **Range:** -4.0 to 4.0 (V/Oct, ±4 octaves from C4)
- **Default:** 0.0 (C4 = 261.6 Hz)
- **Display:** "Frequency"
- **Units:** " V"
- **Widget:** RoundBigBlackKnob
- **DSP Target:** Base oscillator frequency

### FINE_PARAM
- **Type:** Float
- **Range:** -0.5 to 0.5 (semitones)
- **Default:** 0.0
- **Display:** "Fine"
- **Units:** " st"
- **Widget:** Trimpot
- **DSP Target:** Fine pitch adjustment

### WAVE_PARAM
- **Type:** Choice
- **Options:** ["Sine", "Triangle", "Saw", "Square", "Pulse", "Noise"]
- **Default:** 0 (Sine)
- **Display:** "Waveform"
- **Widget:** RoundBlackSnapKnob
- **DSP Target:** Waveform selection

### PULSE_WIDTH_PARAM
- **Type:** Float
- **Range:** 0.1 to 0.9
- **Default:** 0.5
- **Display:** "PW"
- **Units:** ""
- **Widget:** RoundSmallBlackKnob
- **DSP Target:** Pulse width (when Pulse waveform selected)

### BIT_DEPTH_PARAM
- **Type:** Float
- **Range:** 1.0 to 16.0
- **Default:** 16.0
- **Display:** "Bits"
- **Units:** ""
- **Widget:** RoundBlackKnob
- **DSP Target:** Bit depth reduction (quantization levels)

### SAMPLE_RATE_PARAM
- **Type:** Float
- **Range:** 0.0 to 1.0 (mapped to ~1kHz to full sample rate)
- **Default:** 1.0 (full rate)
- **Display:** "Rate"
- **Units:** ""
- **Widget:** RoundBlackKnob
- **DSP Target:** Sample rate reduction

### FM_AMOUNT_PARAM
- **Type:** Float
- **Range:** 0.0 to 1.0
- **Default:** 0.0
- **Display:** "FM Amt"
- **Units:** ""
- **Widget:** RoundBlackKnob
- **DSP Target:** FM modulation depth

### FM_RATIO_PARAM
- **Type:** Float
- **Range:** 0.5 to 8.0
- **Default:** 2.0
- **Display:** "FM Ratio"
- **Units:** ":1"
- **Widget:** RoundSmallBlackKnob
- **DSP Target:** Modulator/carrier frequency ratio

### LEVEL_PARAM
- **Type:** Float
- **Range:** 0.0 to 1.0
- **Default:** 0.8
- **Display:** "Level"
- **Units:** ""
- **Widget:** RoundSmallBlackKnob
- **DSP Target:** Output amplitude

---

## Inputs

### VOCT_INPUT
- **Type:** Polyphonic CV
- **Range:** ±10V (1V/Oct standard)
- **Display:** "V/Oct"
- **Widget:** PJ301MPort
- **DSP Target:** Pitch CV input (adds to FREQ_PARAM)

### FM_INPUT
- **Type:** Polyphonic CV
- **Range:** ±5V
- **Display:** "FM"
- **Widget:** PJ301MPort
- **DSP Target:** FM amount CV modulation

### SYNC_INPUT
- **Type:** Polyphonic Gate/Trigger
- **Range:** 0V/+10V
- **Display:** "Sync"
- **Widget:** PJ301MPort
- **DSP Target:** Hard sync - resets oscillator phase on rising edge

### BITS_INPUT
- **Type:** CV
- **Range:** 0V to +10V
- **Display:** "Bits CV"
- **Widget:** PJ301MPort
- **DSP Target:** Bit depth CV modulation

---

## Outputs

### AUDIO_OUTPUT
- **Type:** Polyphonic Audio
- **Range:** ±5V (10Vpp standard)
- **Display:** "Out"
- **Widget:** PJ301MPort
- **DSP Target:** Main audio output

---

## Lights

### ACTIVE_LIGHT
- **Type:** LED
- **Color:** Green
- **Display:** Activity indicator
- **Widget:** SmallLight<GreenLight>
- **DSP Target:** Lights when audio is being generated

---

## Summary

| Category | Count |
|----------|-------|
| Parameters | 9 |
| Inputs | 4 |
| Outputs | 1 |
| Lights | 1 |
| **Total Components** | **15** |

## Polyphony

- **Polyphonic:** Yes
- **Max Channels:** 16
- **Primary Input:** VOCT_INPUT (determines channel count)
- **State Arrays:** phases[16], fmPhases[16]
