#include "plugin.hpp"

// PolyBLEP anti-aliasing function
// t is phase position relative to discontinuity (t = phase / dt or (phase - 1) / dt)
// dt is phase increment (freq * sampleTime)
inline float polyBlep(float t, float dt) {
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.f;
    } else if (t > 1.f - dt) {
        t = (t - 1.f) / dt;
        return t * t + t + t + 1.f;
    }
    return 0.f;
}

struct GenesisMono : Module {
    enum ParamId {
        FREQ_PARAM,
        FINE_PARAM,
        WAVE_PARAM,
        PULSE_WIDTH_PARAM,
        BIT_DEPTH_PARAM,
        SAMPLE_RATE_PARAM,
        FM_AMOUNT_PARAM,
        FM_RATIO_PARAM,
        LEVEL_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        VOCT_INPUT,
        FM_INPUT,
        SYNC_INPUT,
        BITS_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        AUDIO_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        ACTIVE_LIGHT,
        LIGHTS_LEN
    };

    // DSP state variables
    float phase = 0.f;
    float fmPhase = 0.f;
    dsp::SchmittTrigger syncTrigger;
    float heldSample = 0.f;
    int holdCounter = 0;

    GenesisMono() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        // Configure parameters
        configParam(FREQ_PARAM, -4.f, 4.f, 0.f, "Frequency", " Hz", 2.f, dsp::FREQ_C4);
        configParam(FINE_PARAM, -0.5f, 0.5f, 0.f, "Fine", " st");
        configParam(WAVE_PARAM, 0.f, 5.f, 0.f, "Waveform");
        configParam(PULSE_WIDTH_PARAM, 0.1f, 0.9f, 0.5f, "Pulse Width");
        configParam(BIT_DEPTH_PARAM, 1.f, 16.f, 16.f, "Bit Depth");
        configParam(SAMPLE_RATE_PARAM, 0.f, 1.f, 1.f, "Sample Rate");
        configParam(FM_AMOUNT_PARAM, 0.f, 1.f, 0.f, "FM Amount");
        configParam(FM_RATIO_PARAM, 0.5f, 8.f, 2.f, "FM Ratio");
        configParam(LEVEL_PARAM, 0.f, 1.f, 0.8f, "Level", "%", 0.f, 100.f);

        // Configure inputs
        configInput(VOCT_INPUT, "1V/octave pitch");
        configInput(FM_INPUT, "FM modulation");
        configInput(SYNC_INPUT, "Hard sync");
        configInput(BITS_INPUT, "Bit depth CV");

        // Configure output
        configOutput(AUDIO_OUTPUT, "Audio");
    }

    // Helper function to generate waveform from phase
    float generateWaveform(float p, int waveform, float pulseWidth, float dt) {
        float output = 0.f;
        switch (waveform) {
            case 0: // Sine
                output = std::sin(2.f * M_PI * p);
                break;
            case 1: // Triangle
                output = 4.f * std::abs(p - 0.5f) - 1.f;
                break;
            case 2: // Saw (with PolyBLEP)
                output = 2.f * p - 1.f;
                output -= polyBlep(p, dt);
                break;
            case 3: // Square (with PolyBLEP)
                output = p < 0.5f ? 1.f : -1.f;
                output += polyBlep(p, dt);
                output -= polyBlep(fmodf(p + 0.5f, 1.f), dt);
                break;
            case 4: // Pulse (with PolyBLEP)
                output = p < pulseWidth ? 1.f : -1.f;
                output += polyBlep(p, dt);
                output -= polyBlep(fmodf(p + (1.f - pulseWidth), 1.f), dt);
                break;
            case 5: // Noise (white noise)
                output = 2.f * random::uniform() - 1.f;
                break;
            default:
                output = std::sin(2.f * M_PI * p);
                break;
        }
        return output;
    }

    void process(const ProcessArgs& args) override {
        // --- Phase 2.1-2.3: Oscillator with PolyBLEP, FM Synthesis & Hard Sync ---

        // Read pitch CV and calculate frequency
        float pitch = params[FREQ_PARAM].getValue();
        pitch += inputs[VOCT_INPUT].getVoltage();
        pitch += params[FINE_PARAM].getValue() / 12.f; // Fine tune in semitones
        float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);

        // Calculate phase increment for PolyBLEP
        float dt = freq * args.sampleTime;

        // Get waveform selection and pulse width
        int waveform = static_cast<int>(params[WAVE_PARAM].getValue());
        float pulseWidth = params[PULSE_WIDTH_PARAM].getValue();

        // Get FM parameters
        float fmAmount = params[FM_AMOUNT_PARAM].getValue();
        fmAmount += inputs[FM_INPUT].getVoltage() * 0.2f; // CV adds ±1.0 to amount
        fmAmount = clamp(fmAmount, 0.f, 1.f);
        float fmRatio = params[FM_RATIO_PARAM].getValue();

        // Hard sync detection
        if (syncTrigger.process(inputs[SYNC_INPUT].getVoltage(), 0.1f, 1.f)) {
            phase = 0.f; // Reset carrier phase on rising edge
        }

        // Carrier phase accumulation
        phase += dt;
        if (phase >= 1.f) {
            phase -= 1.f;
        }

        // FM modulator oscillator (ratio-based frequency)
        float fmFreq = freq * fmRatio;
        float fmDt = fmFreq * args.sampleTime;
        fmPhase += fmDt;
        if (fmPhase >= 1.f) {
            fmPhase -= 1.f;
        }

        // Generate modulator output (uses same waveform type)
        float modulatorOutput = generateWaveform(fmPhase, waveform, pulseWidth, fmDt);

        // Apply phase modulation to carrier
        float modulatedPhase = phase + modulatorOutput * fmAmount;
        // Wrap modulated phase to [0, 1)
        modulatedPhase = fmodf(modulatedPhase + 10.f, 1.f); // +10 ensures positive before fmod

        // Generate carrier output with modulated phase
        float output = generateWaveform(modulatedPhase, waveform, pulseWidth, dt);

        // --- Phase 2.4: Bit Crushing & Sample Rate Reduction ---

        // Bit depth reduction
        float bitDepth = params[BIT_DEPTH_PARAM].getValue();
        bitDepth += inputs[BITS_INPUT].getVoltage() * 1.6f; // 0-10V CV maps to 0-16 bits
        bitDepth = clamp(bitDepth, 1.f, 16.f);

        // Quantize output (normalize to 0-1, quantize, denormalize)
        float levels = std::pow(2.f, bitDepth);
        float normalized = (output + 1.f) * 0.5f; // Map -1..1 to 0..1
        float quantized = std::floor(normalized * levels) / levels;
        output = quantized * 2.f - 1.f; // Map 0..1 back to -1..1

        // Sample rate reduction
        float sampleRateParam = params[SAMPLE_RATE_PARAM].getValue();
        // Map 0.0-1.0 to ~1kHz to full sample rate
        float targetRate = 1000.f + (args.sampleRate - 1000.f) * sampleRateParam;
        int holdFrames = std::max(1, static_cast<int>(args.sampleRate / targetRate));

        holdCounter++;
        if (holdCounter >= holdFrames) {
            heldSample = output;
            holdCounter = 0;
        }
        output = heldSample;

        // Apply level control
        float level = params[LEVEL_PARAM].getValue();
        output *= level;

        // Scale to VCV Rack audio range (±5V)
        output *= 5.f;

        // Output
        outputs[AUDIO_OUTPUT].setVoltage(output);
        outputs[AUDIO_OUTPUT].setChannels(1);

        // Activity light (based on output amplitude)
        lights[ACTIVE_LIGHT].setBrightness(std::abs(output) > 0.1f ? 1.f : 0.f);
    }
};

struct GenesisMonoWidget : ModuleWidget {
    GenesisMonoWidget(GenesisMono* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/GenesisMono.svg")));

        // Add screws
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Left column - Parameters (positioned below labels)
        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(10.16, 24.0)), module, GenesisMono::FREQ_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(10.16, 37.0)), module, GenesisMono::FINE_PARAM));
        addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(10.16, 50.0)), module, GenesisMono::WAVE_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10.16, 63.0)), module, GenesisMono::PULSE_WIDTH_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10.16, 76.0)), module, GenesisMono::BIT_DEPTH_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10.16, 89.0)), module, GenesisMono::SAMPLE_RATE_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(10.16, 102.0)), module, GenesisMono::FM_AMOUNT_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(10.16, 115.0)), module, GenesisMono::FM_RATIO_PARAM));

        // Right column - Level and Inputs
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(30.48, 24.0)), module, GenesisMono::LEVEL_PARAM));

        // Add inputs (right column)
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 37.0)), module, GenesisMono::VOCT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 50.0)), module, GenesisMono::FM_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 63.0)), module, GenesisMono::SYNC_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.48, 76.0)), module, GenesisMono::BITS_INPUT));

        // Add output (right column bottom)
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.48, 115.0)), module, GenesisMono::AUDIO_OUTPUT));

        // Add activity light (top center)
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(20.32, 11.0)), module, GenesisMono::ACTIVE_LIGHT));
    }
};

// Create model at file scope
Model* modelGenesisMono = createModel<GenesisMono, GenesisMonoWidget>("GenesisMono");
