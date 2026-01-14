#include "plugin.hpp"

// PolyBLEP anti-aliasing function
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

struct GenesisPoly : Module {
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

    // DSP state variables (polyphonic - 16 voices)
    float phase[16] = {};
    float fmPhase[16] = {};
    dsp::SchmittTrigger syncTrigger[16];
    float heldSample[16] = {};
    int holdCounter[16] = {};

    GenesisPoly() {
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
        // --- Phase 2.1-2.3: Oscillator with PolyBLEP, FM & Sync - Polyphonic ---

        // Get channel count from V/Oct input
        int channels = std::max(1, inputs[VOCT_INPUT].getChannels());

        // Get global parameters (shared across all voices)
        float freqParam = params[FREQ_PARAM].getValue();
        float fineParam = params[FINE_PARAM].getValue() / 12.f; // Convert semitones
        int waveform = static_cast<int>(params[WAVE_PARAM].getValue());
        float pulseWidth = params[PULSE_WIDTH_PARAM].getValue();
        float level = params[LEVEL_PARAM].getValue();

        // FM parameters
        float fmAmountParam = params[FM_AMOUNT_PARAM].getValue();
        float fmRatio = params[FM_RATIO_PARAM].getValue();

        // Track max output for activity light
        float maxOutput = 0.f;

        // Process each voice
        for (int c = 0; c < channels; c++) {
            // Read pitch CV and calculate frequency for this channel
            float pitch = freqParam;
            pitch += inputs[VOCT_INPUT].getPolyVoltage(c);
            pitch += fineParam;
            float freq = dsp::FREQ_C4 * std::pow(2.f, pitch);

            // Calculate phase increment for PolyBLEP
            float dt = freq * args.sampleTime;

            // FM amount with CV modulation (per-voice)
            float fmAmount = fmAmountParam;
            fmAmount += inputs[FM_INPUT].getPolyVoltage(c) * 0.2f;
            fmAmount = clamp(fmAmount, 0.f, 1.f);

            // Hard sync detection (per-voice)
            if (syncTrigger[c].process(inputs[SYNC_INPUT].getPolyVoltage(c), 0.1f, 1.f)) {
                phase[c] = 0.f; // Reset carrier phase on rising edge
            }

            // Carrier phase accumulation for this channel
            phase[c] += dt;
            if (phase[c] >= 1.f) {
                phase[c] -= 1.f;
            }

            // FM modulator oscillator (ratio-based frequency)
            float fmFreq = freq * fmRatio;
            float fmDt = fmFreq * args.sampleTime;
            fmPhase[c] += fmDt;
            if (fmPhase[c] >= 1.f) {
                fmPhase[c] -= 1.f;
            }

            // Generate modulator output (uses same waveform type)
            float modulatorOutput = generateWaveform(fmPhase[c], waveform, pulseWidth, fmDt);

            // Apply phase modulation to carrier
            float modulatedPhase = phase[c] + modulatorOutput * fmAmount;
            // Wrap modulated phase to [0, 1)
            modulatedPhase = fmodf(modulatedPhase + 10.f, 1.f);

            // Generate carrier output with modulated phase
            float output = generateWaveform(modulatedPhase, waveform, pulseWidth, dt);

            // --- Phase 2.4: Bit Crushing & Sample Rate Reduction (per-voice) ---

            // Bit depth reduction
            float bitDepth = params[BIT_DEPTH_PARAM].getValue();
            bitDepth += inputs[BITS_INPUT].getVoltage() * 1.6f; // 0-10V CV maps to 0-16 bits
            bitDepth = clamp(bitDepth, 1.f, 16.f);

            // Quantize output (normalize to 0-1, quantize, denormalize)
            float levels = std::pow(2.f, bitDepth);
            float normalized = (output + 1.f) * 0.5f; // Map -1..1 to 0..1
            float quantized = std::floor(normalized * levels) / levels;
            output = quantized * 2.f - 1.f; // Map 0..1 back to -1..1

            // Sample rate reduction (per-voice)
            float sampleRateParam = params[SAMPLE_RATE_PARAM].getValue();
            float targetRate = 1000.f + (args.sampleRate - 1000.f) * sampleRateParam;
            int holdFrames = std::max(1, static_cast<int>(args.sampleRate / targetRate));

            holdCounter[c]++;
            if (holdCounter[c] >= holdFrames) {
                heldSample[c] = output;
                holdCounter[c] = 0;
            }
            output = heldSample[c];

            // Apply level control
            output *= level;

            // Scale to VCV Rack audio range (±5V)
            output *= 5.f;

            // Track max for activity light
            maxOutput = std::max(maxOutput, std::abs(output));

            // Output for this channel
            outputs[AUDIO_OUTPUT].setVoltage(output, c);
        }

        // Set output channels (CRITICAL: must call after loop)
        outputs[AUDIO_OUTPUT].setChannels(channels);

        // Activity light (based on max output amplitude)
        lights[ACTIVE_LIGHT].setBrightness(maxOutput > 0.1f ? 1.f : 0.f);
    }
};

struct GenesisPolyWidget : ModuleWidget {
    GenesisPolyWidget(GenesisPoly* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/GenesisPoly.svg")));

        // Add screws
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Left column - Parameters (positioned below labels)
        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(12.7, 24.0)), module, GenesisPoly::FREQ_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(12.7, 37.0)), module, GenesisPoly::FINE_PARAM));
        addParam(createParamCentered<RoundBlackSnapKnob>(mm2px(Vec(12.7, 50.0)), module, GenesisPoly::WAVE_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12.7, 63.0)), module, GenesisPoly::PULSE_WIDTH_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12.7, 76.0)), module, GenesisPoly::BIT_DEPTH_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12.7, 89.0)), module, GenesisPoly::SAMPLE_RATE_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(12.7, 102.0)), module, GenesisPoly::FM_AMOUNT_PARAM));
        addParam(createParamCentered<Trimpot>(mm2px(Vec(12.7, 115.0)), module, GenesisPoly::FM_RATIO_PARAM));

        // Right column - Level and Inputs
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(38.1, 24.0)), module, GenesisPoly::LEVEL_PARAM));

        // Add inputs (right column)
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.1, 37.0)), module, GenesisPoly::VOCT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.1, 50.0)), module, GenesisPoly::FM_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.1, 63.0)), module, GenesisPoly::SYNC_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38.1, 76.0)), module, GenesisPoly::BITS_INPUT));

        // Add output (right column bottom)
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.1, 115.0)), module, GenesisPoly::AUDIO_OUTPUT));

        // Add activity light (top center)
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(25.4, 11.0)), module, GenesisPoly::ACTIVE_LIGHT));
    }
};

// Create model at file scope
Model* modelGenesisPoly = createModel<GenesisPoly, GenesisPolyWidget>("GenesisPoly");
