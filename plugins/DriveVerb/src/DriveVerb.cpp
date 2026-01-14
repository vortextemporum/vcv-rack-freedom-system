#include "plugin.hpp"

// Simple comb filter for reverb
struct CombFilter {
    static constexpr int MAX_SIZE = 8192;
    float buffer[MAX_SIZE] = {};
    int size = 1000;
    int writePos = 0;
    float feedback = 0.5f;
    float damp = 0.5f;
    float filterStore = 0.f;

    void setSize(int newSize) {
        size = clamp(newSize, 1, MAX_SIZE - 1);
    }

    float process(float input) {
        int readPos = (writePos - size + MAX_SIZE) % size;
        float output = buffer[readPos];

        // One-pole lowpass filter in feedback loop
        filterStore = (output * (1.f - damp)) + (filterStore * damp);

        buffer[writePos] = input + (filterStore * feedback);
        writePos = (writePos + 1) % size;

        return output;
    }

    void clear() {
        std::fill(std::begin(buffer), std::end(buffer), 0.f);
        filterStore = 0.f;
    }
};

// Simple allpass filter for reverb
struct AllpassFilter {
    static constexpr int MAX_SIZE = 4096;
    float buffer[MAX_SIZE] = {};
    int size = 500;
    int writePos = 0;
    float feedback = 0.5f;

    void setSize(int newSize) {
        size = clamp(newSize, 1, MAX_SIZE - 1);
    }

    float process(float input) {
        int readPos = (writePos - size + MAX_SIZE) % size;
        float bufOut = buffer[readPos];
        float output = -input + bufOut;
        buffer[writePos] = input + (bufOut * feedback);
        writePos = (writePos + 1) % size;
        return output;
    }

    void clear() {
        std::fill(std::begin(buffer), std::end(buffer), 0.f);
    }
};

// Biquad filter for DJ-style filtering
struct BiquadFilter {
    float a0 = 1.f, a1 = 0.f, a2 = 0.f;
    float b1 = 0.f, b2 = 0.f;
    float z1 = 0.f, z2 = 0.f;

    void reset() { z1 = z2 = 0.f; }

    void setLowPass(float sampleRate, float cutoff, float Q) {
        float w0 = 2.f * M_PI * cutoff / sampleRate;
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.f * Q);

        float b0 = (1.f - cosw0) / 2.f;
        float b1_coef = 1.f - cosw0;
        float b2_coef = (1.f - cosw0) / 2.f;
        float a0_coef = 1.f + alpha;

        a0 = b0 / a0_coef;
        a1 = b1_coef / a0_coef;
        a2 = b2_coef / a0_coef;
        b1 = (-2.f * cosw0) / a0_coef;
        b2 = (1.f - alpha) / a0_coef;
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

        a0 = b0 / a0_coef;
        a1 = b1_coef / a0_coef;
        a2 = b2_coef / a0_coef;
        b1 = (-2.f * cosw0) / a0_coef;
        b2 = (1.f - alpha) / a0_coef;
    }

    float process(float input) {
        float output = a0 * input + a1 * z1 + a2 * z2 - b1 * z1 - b2 * z2;
        float temp = input - b1 * z1 - b2 * z2;
        output = a0 * temp + a1 * z1 + a2 * z2;
        z2 = z1;
        z1 = temp;
        return output;
    }
};

struct DriveVerb : Module {
    enum ParamId {
        SIZE_PARAM,
        DECAY_PARAM,
        MIX_PARAM,
        DRIVE_PARAM,
        FILTER_PARAM,
        FILTER_POS_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        LEFT_INPUT,
        RIGHT_INPUT,
        MIX_CV_INPUT,
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

    // Freeverb-style reverb: 8 parallel comb filters + 4 series allpass filters (per channel)
    CombFilter combL[8];
    CombFilter combR[8];
    AllpassFilter allpassL[4];
    AllpassFilter allpassR[4];

    // Comb filter delay times (scaled for ~44.1kHz)
    const int combTunings[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
    const int allpassTunings[4] = {556, 441, 341, 225};
    const int stereoSpread = 23;

    // DJ-style filter
    BiquadFilter filterL;
    BiquadFilter filterR;
    bool previousWasLowPass = false;

    DriveVerb() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(SIZE_PARAM, 0.f, 100.f, 40.f, "Size", "%");
        configParam(DECAY_PARAM, 0.5f, 10.f, 2.f, "Decay", " s");
        configParam(MIX_PARAM, 0.f, 100.f, 30.f, "Dry/Wet", "%");
        configParam(DRIVE_PARAM, 0.f, 24.f, 6.f, "Drive", " dB");
        configParam(FILTER_PARAM, -100.f, 100.f, 0.f, "Filter", "%");
        configSwitch(FILTER_POS_PARAM, 0.f, 1.f, 1.f, "Filter Position", {"Pre", "Post"});

        configInput(LEFT_INPUT, "Left audio");
        configInput(RIGHT_INPUT, "Right audio");
        configInput(MIX_CV_INPUT, "Mix CV");

        configOutput(LEFT_OUTPUT, "Left audio");
        configOutput(RIGHT_OUTPUT, "Right audio");

        configBypass(LEFT_INPUT, LEFT_OUTPUT);
        configBypass(RIGHT_INPUT, RIGHT_OUTPUT);

        // Initialize comb and allpass filter sizes
        for (int i = 0; i < 8; i++) {
            combL[i].setSize(combTunings[i]);
            combR[i].setSize(combTunings[i] + stereoSpread);
        }
        for (int i = 0; i < 4; i++) {
            allpassL[i].setSize(allpassTunings[i]);
            allpassR[i].setSize(allpassTunings[i] + stereoSpread);
            allpassL[i].feedback = 0.5f;
            allpassR[i].feedback = 0.5f;
        }
    }

    void onSampleRateChange() override {
        float ratio = APP->engine->getSampleRate() / 44100.f;
        for (int i = 0; i < 8; i++) {
            combL[i].setSize(static_cast<int>(combTunings[i] * ratio));
            combR[i].setSize(static_cast<int>((combTunings[i] + stereoSpread) * ratio));
            combL[i].clear();
            combR[i].clear();
        }
        for (int i = 0; i < 4; i++) {
            allpassL[i].setSize(static_cast<int>(allpassTunings[i] * ratio));
            allpassR[i].setSize(static_cast<int>((allpassTunings[i] + stereoSpread) * ratio));
            allpassL[i].clear();
            allpassR[i].clear();
        }
    }

    void process(const ProcessArgs& args) override {
        // Read parameters
        float size = params[SIZE_PARAM].getValue() / 100.f;
        float decay = params[DECAY_PARAM].getValue();
        float mix = params[MIX_PARAM].getValue();
        if (inputs[MIX_CV_INPUT].isConnected()) {
            mix += inputs[MIX_CV_INPUT].getVoltage() * 10.f;
            mix = clamp(mix, 0.f, 100.f);
        }
        mix /= 100.f;

        float driveDbs = params[DRIVE_PARAM].getValue();
        float driveGain = std::pow(10.f, driveDbs / 20.f);
        float filterValue = params[FILTER_PARAM].getValue();
        bool isPostMode = params[FILTER_POS_PARAM].getValue() > 0.5f;

        // Calculate reverb parameters
        float feedback = 0.5f + (decay / 20.f);  // Map decay to feedback
        feedback = clamp(feedback, 0.5f, 0.98f);
        float damp = 0.5f - (size * 0.3f);  // More size = less damping
        damp = clamp(damp, 0.1f, 0.7f);

        // Update comb filters
        for (int i = 0; i < 8; i++) {
            combL[i].feedback = feedback;
            combR[i].feedback = feedback;
            combL[i].damp = damp;
            combR[i].damp = damp;
        }

        // Get inputs (normalize to ±1 for processing)
        float inputL = inputs[LEFT_INPUT].getVoltage() / 5.f;
        float inputR = inputs[RIGHT_INPUT].isConnected() ?
                       inputs[RIGHT_INPUT].getVoltage() / 5.f : inputL;

        // Store dry signal
        float dryL = inputL;
        float dryR = inputR;

        // Process reverb (8 parallel comb filters)
        float wetL = 0.f;
        float wetR = 0.f;
        for (int i = 0; i < 8; i++) {
            wetL += combL[i].process(inputL);
            wetR += combR[i].process(inputR);
        }
        wetL /= 8.f;
        wetR /= 8.f;

        // Process through 4 series allpass filters
        for (int i = 0; i < 4; i++) {
            wetL = allpassL[i].process(wetL);
            wetR = allpassR[i].process(wetR);
        }

        // Apply drive and filter based on routing mode
        auto applyDrive = [&](float& sampleL, float& sampleR) {
            sampleL = std::tanh(sampleL * driveGain);
            sampleR = std::tanh(sampleR * driveGain);
        };

        auto applyFilter = [&](float& sampleL, float& sampleR) {
            if (std::abs(filterValue) > 0.5f) {
                bool isLowPass = (filterValue < 0.f);

                if (isLowPass != previousWasLowPass) {
                    filterL.reset();
                    filterR.reset();
                }
                previousWasLowPass = isLowPass;

                if (isLowPass) {
                    float normalizedValue = std::abs(filterValue) / 100.f;
                    float cutoffHz = 20000.f * std::pow(10.f, -normalizedValue * std::log10(20000.f / 200.f));
                    cutoffHz = clamp(cutoffHz, 200.f, 20000.f);
                    filterL.setLowPass(args.sampleRate, cutoffHz, 0.707f);
                    filterR.setLowPass(args.sampleRate, cutoffHz, 0.707f);
                } else {
                    float normalizedValue = filterValue / 100.f;
                    float cutoffHz = 20.f * std::pow(10.f, normalizedValue * std::log10(10000.f / 20.f));
                    cutoffHz = clamp(cutoffHz, 20.f, 10000.f);
                    filterL.setHighPass(args.sampleRate, cutoffHz, 0.707f);
                    filterR.setHighPass(args.sampleRate, cutoffHz, 0.707f);
                }

                sampleL = filterL.process(sampleL);
                sampleR = filterR.process(sampleR);
            }
        };

        if (isPostMode) {
            applyDrive(wetL, wetR);
            applyFilter(wetL, wetR);
        } else {
            applyFilter(wetL, wetR);
            applyDrive(wetL, wetR);
        }

        // Mix dry and wet
        float outputL = dryL * (1.f - mix) + wetL * mix;
        float outputR = dryR * (1.f - mix) + wetR * mix;

        // Output (scale back to ±5V)
        outputs[LEFT_OUTPUT].setVoltage(outputL * 5.f);
        outputs[RIGHT_OUTPUT].setVoltage(outputR * 5.f);
    }
};

struct DriveVerbWidget : ModuleWidget {
    DriveVerbWidget(DriveVerb* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/DriveVerb.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // 8HP module = 40.64mm, center at 20.32mm
        // Knobs in 2 columns (10.16 and 30.48)
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 28)), module, DriveVerb::SIZE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 28)), module, DriveVerb::DECAY_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 52)), module, DriveVerb::MIX_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(30.48, 52)), module, DriveVerb::DRIVE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10.16, 76)), module, DriveVerb::FILTER_PARAM));

        // Filter position switch
        addParam(createParamCentered<CKSS>(mm2px(Vec(30.48, 76)), module, DriveVerb::FILTER_POS_PARAM));

        // CV input (center)
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32, 95)), module, DriveVerb::MIX_CV_INPUT));

        // Audio I/O (fit within 8HP)
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 112)), module, DriveVerb::LEFT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(17.78, 112)), module, DriveVerb::RIGHT_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(27.94, 112)), module, DriveVerb::LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.10, 112)), module, DriveVerb::RIGHT_OUTPUT));
    }
};

Model* modelDriveVerb = createModel<DriveVerb, DriveVerbWidget>("DriveVerb");
