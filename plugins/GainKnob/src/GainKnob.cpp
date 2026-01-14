#include "plugin.hpp"

// Simple biquad filter implementation
struct BiquadFilter {
    float a0 = 1.f, a1 = 0.f, a2 = 0.f;
    float b1 = 0.f, b2 = 0.f;
    float z1 = 0.f, z2 = 0.f;

    void reset() {
        z1 = z2 = 0.f;
    }

    void setLowPass(float sampleRate, float cutoff, float Q) {
        float w0 = 2.f * M_PI * cutoff / sampleRate;
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.f * Q);

        float b0 = (1.f - cosw0) / 2.f;
        float b1_coef = 1.f - cosw0;
        float b2_coef = (1.f - cosw0) / 2.f;
        float a0_coef = 1.f + alpha;
        float a1_coef = -2.f * cosw0;
        float a2_coef = 1.f - alpha;

        // Normalize coefficients
        a0 = b0 / a0_coef;
        a1 = b1_coef / a0_coef;
        a2 = b2_coef / a0_coef;
        b1 = a1_coef / a0_coef;
        b2 = a2_coef / a0_coef;
    }

    void setHighPass(float sampleRate, float cutoff, float Q) {
        float w0 = 2.f * M_PI * cutoff / sampleRate;
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.f * Q);

        float b0 = (1.f + cosw0) / 2.f;
        float b1_coef = -(1.f + cosw0);
        float b2_coef = (1.f + cosw0) / 2.f;
        float a0_coef = 1.f + alpha;
        float a1_coef = -2.f * cosw0;
        float a2_coef = 1.f - alpha;

        // Normalize coefficients
        a0 = b0 / a0_coef;
        a1 = b1_coef / a0_coef;
        a2 = b2_coef / a0_coef;
        b1 = a1_coef / a0_coef;
        b2 = a2_coef / a0_coef;
    }

    float process(float input) {
        float output = a0 * input + a1 * z1 + a2 * z2 - b1 * z1 - b2 * z2;
        // Direct Form II Transposed
        float temp = input - b1 * z1 - b2 * z2;
        output = a0 * temp + a1 * z1 + a2 * z2;
        z2 = z1;
        z1 = temp;
        return output;
    }
};

struct GainKnob : Module {
    enum ParamId {
        GAIN_PARAM,
        PAN_PARAM,
        FILTER_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        LEFT_INPUT,
        RIGHT_INPUT,
        GAIN_CV_INPUT,
        PAN_CV_INPUT,
        FILTER_CV_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        LIGHTS_LEN
    };

    // Per-channel filter state
    BiquadFilter filterL;
    BiquadFilter filterR;
    bool previousWasLowPass = false;
    float lastFilterPercent = 0.f;

    GainKnob() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        // Gain: -60dB to 0dB (default 0dB)
        configParam(GAIN_PARAM, -60.f, 0.f, 0.f, "Gain", " dB");

        // Pan: -100% to +100% (default center)
        configParam(PAN_PARAM, -100.f, 100.f, 0.f, "Pan", "%");

        // Filter: -100% (low-pass) to +100% (high-pass), 0 = bypass
        configParam(FILTER_PARAM, -100.f, 100.f, 0.f, "Filter", "%");

        // Configure inputs
        configInput(LEFT_INPUT, "Left audio");
        configInput(RIGHT_INPUT, "Right audio");
        configInput(GAIN_CV_INPUT, "Gain CV");
        configInput(PAN_CV_INPUT, "Pan CV");
        configInput(FILTER_CV_INPUT, "Filter CV");

        // Configure outputs
        configOutput(LEFT_OUTPUT, "Left audio");
        configOutput(RIGHT_OUTPUT, "Right audio");

        // Bypass routing
        configBypass(LEFT_INPUT, LEFT_OUTPUT);
        configBypass(RIGHT_INPUT, RIGHT_OUTPUT);
    }

    void process(const ProcessArgs& args) override {
        // Read parameters with CV modulation
        float gainDb = params[GAIN_PARAM].getValue();
        if (inputs[GAIN_CV_INPUT].isConnected()) {
            // ±5V CV adds ±30dB
            gainDb += inputs[GAIN_CV_INPUT].getVoltage() * 6.f;
            gainDb = clamp(gainDb, -60.f, 0.f);
        }

        float panPercent = params[PAN_PARAM].getValue();
        if (inputs[PAN_CV_INPUT].isConnected()) {
            // ±5V CV adds ±100%
            panPercent += inputs[PAN_CV_INPUT].getVoltage() * 20.f;
            panPercent = clamp(panPercent, -100.f, 100.f);
        }

        float filterPercent = params[FILTER_PARAM].getValue();
        if (inputs[FILTER_CV_INPUT].isConnected()) {
            // ±5V CV adds ±100%
            filterPercent += inputs[FILTER_CV_INPUT].getVoltage() * 20.f;
            filterPercent = clamp(filterPercent, -100.f, 100.f);
        }

        // Get input signals
        float inputL = inputs[LEFT_INPUT].getVoltage();
        float inputR = inputs[RIGHT_INPUT].isConnected() ?
                       inputs[RIGHT_INPUT].getVoltage() : inputL;

        // Apply DJ-style filter if not at center
        float outputL = inputL;
        float outputR = inputR;

        if (std::abs(filterPercent) > 0.5f) {
            bool isLowPass = (filterPercent < 0.f);

            // Reset filter state when switching between low-pass and high-pass
            if (isLowPass != previousWasLowPass) {
                filterL.reset();
                filterR.reset();
            }
            previousWasLowPass = isLowPass;

            // Update filter coefficients
            if (isLowPass) {
                // Low-pass: -100% = 200Hz, 0% = 20kHz
                float normalizedValue = std::abs(filterPercent) / 100.f;
                float cutoffHz = 20000.f * std::pow(10.f, -normalizedValue * std::log10(20000.f / 200.f));
                cutoffHz = clamp(cutoffHz, 200.f, 20000.f);
                filterL.setLowPass(args.sampleRate, cutoffHz, 0.707f);
                filterR.setLowPass(args.sampleRate, cutoffHz, 0.707f);
            } else {
                // High-pass: 0% = 20Hz, +100% = 10kHz
                float normalizedValue = filterPercent / 100.f;
                float cutoffHz = 20.f * std::pow(10.f, normalizedValue * std::log10(10000.f / 20.f));
                cutoffHz = clamp(cutoffHz, 20.f, 10000.f);
                filterL.setHighPass(args.sampleRate, cutoffHz, 0.707f);
                filterR.setHighPass(args.sampleRate, cutoffHz, 0.707f);
            }

            outputL = filterL.process(inputL);
            outputR = filterR.process(inputR);
        } else {
            // Reset filter when entering bypass zone
            if (std::abs(lastFilterPercent) > 0.5f) {
                filterL.reset();
                filterR.reset();
                previousWasLowPass = false;
            }
        }
        lastFilterPercent = filterPercent;

        // Convert dB to linear gain
        float gainLinear;
        if (gainDb <= -59.9f) {
            gainLinear = 0.f;
        } else {
            gainLinear = std::pow(10.f, gainDb / 20.f);
        }

        // Calculate constant-power pan coefficients
        float panNormalized = panPercent / 100.f; // -1.0 to +1.0
        float panRadians = (panNormalized * 0.25f + 0.25f) * M_PI;

        float leftGain = std::cos(panRadians) * gainLinear;
        float rightGain = std::sin(panRadians) * gainLinear;

        // Apply gain and pan
        outputs[LEFT_OUTPUT].setVoltage(outputL * leftGain);
        outputs[RIGHT_OUTPUT].setVoltage(outputR * rightGain);
    }
};

struct GainKnobWidget : ModuleWidget {
    GainKnobWidget(GainKnob* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/GainKnob.svg")));

        // Add screws
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Main knobs (centered horizontally at 15.24mm for 6HP module)
        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 28)), module, GainKnob::GAIN_PARAM));
        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 52)), module, GainKnob::PAN_PARAM));
        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 76)), module, GainKnob::FILTER_PARAM));

        // CV inputs (left side)
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 95)), module, GainKnob::GAIN_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 107)), module, GainKnob::PAN_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.86, 95)), module, GainKnob::FILTER_CV_INPUT));

        // Audio inputs (bottom left)
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 119)), module, GainKnob::LEFT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 119)), module, GainKnob::RIGHT_INPUT));

        // Audio outputs (bottom right)
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 107)), module, GainKnob::LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.86, 119)), module, GainKnob::RIGHT_OUTPUT));
    }
};

Model* modelGainKnob = createModel<GainKnob, GainKnobWidget>("GainKnob");
