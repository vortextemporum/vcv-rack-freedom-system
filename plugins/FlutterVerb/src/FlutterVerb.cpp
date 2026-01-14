#include "plugin.hpp"

// Reuse comb/allpass/biquad from DriveVerb pattern
struct CombFilter {
    static constexpr int MAX_SIZE = 8192;
    float buffer[MAX_SIZE] = {};
    int size = 1000;
    int writePos = 0;
    float feedback = 0.5f;
    float damp = 0.5f;
    float filterStore = 0.f;

    void setSize(int newSize) { size = clamp(newSize, 1, MAX_SIZE - 1); }

    float process(float input) {
        int readPos = (writePos - size + MAX_SIZE) % size;
        float output = buffer[readPos];
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

struct AllpassFilter {
    static constexpr int MAX_SIZE = 4096;
    float buffer[MAX_SIZE] = {};
    int size = 500;
    int writePos = 0;
    float feedback = 0.5f;

    void setSize(int newSize) { size = clamp(newSize, 1, MAX_SIZE - 1); }

    float process(float input) {
        int readPos = (writePos - size + MAX_SIZE) % size;
        float bufOut = buffer[readPos];
        float output = -input + bufOut;
        buffer[writePos] = input + (bufOut * feedback);
        writePos = (writePos + 1) % size;
        return output;
    }

    void clear() { std::fill(std::begin(buffer), std::end(buffer), 0.f); }
};

// Modulation delay line
struct ModulationDelay {
    static constexpr int MAX_SIZE = 16384;
    float buffer[MAX_SIZE] = {};
    int writePos = 0;

    float read(float delaySamples) const {
        float readPos = writePos - delaySamples;
        while (readPos < 0) readPos += MAX_SIZE;
        int pos0 = static_cast<int>(readPos) % MAX_SIZE;
        int pos1 = (pos0 + 1) % MAX_SIZE;
        float frac = readPos - std::floor(readPos);
        return buffer[pos0] * (1.f - frac) + buffer[pos1] * frac;
    }

    void write(float sample) {
        buffer[writePos] = sample;
        writePos = (writePos + 1) % MAX_SIZE;
    }

    void clear() { std::fill(std::begin(buffer), std::end(buffer), 0.f); }
};

struct BiquadFilter {
    float a0 = 1.f, a1 = 0.f, a2 = 0.f, b1 = 0.f, b2 = 0.f;
    float z1 = 0.f, z2 = 0.f;

    void reset() { z1 = z2 = 0.f; }

    void setLowPass(float sampleRate, float cutoff, float Q) {
        float w0 = 2.f * M_PI * cutoff / sampleRate;
        float cosw0 = std::cos(w0), sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.f * Q);
        float a0_coef = 1.f + alpha;
        a0 = ((1.f - cosw0) / 2.f) / a0_coef;
        a1 = (1.f - cosw0) / a0_coef;
        a2 = ((1.f - cosw0) / 2.f) / a0_coef;
        b1 = (-2.f * cosw0) / a0_coef;
        b2 = (1.f - alpha) / a0_coef;
    }

    void setHighPass(float sampleRate, float cutoff, float Q) {
        float w0 = 2.f * M_PI * cutoff / sampleRate;
        float cosw0 = std::cos(w0), sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.f * Q);
        float a0_coef = 1.f + alpha;
        a0 = ((1.f + cosw0) / 2.f) / a0_coef;
        a1 = (-(1.f + cosw0)) / a0_coef;
        a2 = ((1.f + cosw0) / 2.f) / a0_coef;
        b1 = (-2.f * cosw0) / a0_coef;
        b2 = (1.f - alpha) / a0_coef;
    }

    float process(float input) {
        float temp = input - b1 * z1 - b2 * z2;
        float output = a0 * temp + a1 * z1 + a2 * z2;
        z2 = z1;
        z1 = temp;
        return output;
    }
};

struct FlutterVerb : Module {
    enum ParamId {
        SIZE_PARAM, DECAY_PARAM, MIX_PARAM, AGE_PARAM, DRIVE_PARAM, TONE_PARAM, MOD_MODE_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        LEFT_INPUT, RIGHT_INPUT, MIX_CV_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        LEFT_OUTPUT, RIGHT_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId { LIGHTS_LEN };

    // Reverb
    CombFilter combL[8], combR[8];
    AllpassFilter allpassL[4], allpassR[4];
    const int combTunings[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
    const int allpassTunings[4] = {556, 441, 341, 225};
    const int stereoSpread = 23;

    // Modulation
    ModulationDelay modDelayL, modDelayR;
    float wowPhaseL = 0.f, wowPhaseR = 0.f;
    float flutterPhaseL = 0.f, flutterPhaseR = 0.f;

    // Filter
    BiquadFilter filterL, filterR;
    bool previousWasLowPass = false;

    FlutterVerb() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(SIZE_PARAM, 0.f, 100.f, 50.f, "Size", "%");
        configParam(DECAY_PARAM, 0.1f, 10.f, 2.5f, "Decay", " s");
        configParam(MIX_PARAM, 0.f, 100.f, 25.f, "Mix", "%");
        configParam(AGE_PARAM, 0.f, 100.f, 20.f, "Age", "%");
        configParam(DRIVE_PARAM, 0.f, 100.f, 20.f, "Drive", "%");
        configParam(TONE_PARAM, -100.f, 100.f, 0.f, "Tone", "%");
        configSwitch(MOD_MODE_PARAM, 0.f, 1.f, 0.f, "Mod Mode", {"Wet Only", "Wet+Dry"});

        configInput(LEFT_INPUT, "Left audio");
        configInput(RIGHT_INPUT, "Right audio");
        configInput(MIX_CV_INPUT, "Mix CV");
        configOutput(LEFT_OUTPUT, "Left audio");
        configOutput(RIGHT_OUTPUT, "Right audio");

        configBypass(LEFT_INPUT, LEFT_OUTPUT);
        configBypass(RIGHT_INPUT, RIGHT_OUTPUT);

        for (int i = 0; i < 8; i++) {
            combL[i].setSize(combTunings[i]);
            combR[i].setSize(combTunings[i] + stereoSpread);
        }
        for (int i = 0; i < 4; i++) {
            allpassL[i].setSize(allpassTunings[i]);
            allpassR[i].setSize(allpassTunings[i] + stereoSpread);
            allpassL[i].feedback = allpassR[i].feedback = 0.5f;
        }
    }

    void onSampleRateChange() override {
        float ratio = APP->engine->getSampleRate() / 44100.f;
        for (int i = 0; i < 8; i++) {
            combL[i].setSize(static_cast<int>(combTunings[i] * ratio));
            combR[i].setSize(static_cast<int>((combTunings[i] + stereoSpread) * ratio));
            combL[i].clear(); combR[i].clear();
        }
        for (int i = 0; i < 4; i++) {
            allpassL[i].setSize(static_cast<int>(allpassTunings[i] * ratio));
            allpassR[i].setSize(static_cast<int>((allpassTunings[i] + stereoSpread) * ratio));
            allpassL[i].clear(); allpassR[i].clear();
        }
        modDelayL.clear(); modDelayR.clear();
    }

    void process(const ProcessArgs& args) override {
        float size = params[SIZE_PARAM].getValue() / 100.f;
        float decay = params[DECAY_PARAM].getValue();
        float mix = params[MIX_PARAM].getValue();
        if (inputs[MIX_CV_INPUT].isConnected()) {
            mix += inputs[MIX_CV_INPUT].getVoltage() * 10.f;
            mix = clamp(mix, 0.f, 100.f);
        }
        mix /= 100.f;

        float age = params[AGE_PARAM].getValue() / 100.f;
        float drive = params[DRIVE_PARAM].getValue() / 100.f;
        float tone = params[TONE_PARAM].getValue();
        bool wetDryMode = params[MOD_MODE_PARAM].getValue() > 0.5f;

        // Reverb params
        float feedback = clamp(0.5f + (decay / 20.f), 0.5f, 0.98f);
        float damp = clamp(0.5f - (size * 0.3f), 0.1f, 0.7f);
        for (int i = 0; i < 8; i++) {
            combL[i].feedback = combR[i].feedback = feedback;
            combL[i].damp = combR[i].damp = damp;
        }

        // Get inputs
        float inputL = inputs[LEFT_INPUT].getVoltage() / 5.f;
        float inputR = inputs[RIGHT_INPUT].isConnected() ? inputs[RIGHT_INPUT].getVoltage() / 5.f : inputL;
        float dryL = inputL, dryR = inputR;

        // Modulation function
        auto applyModulation = [&](float& sampleL, float& sampleR) {
            if (age > 0.f) {
                float scaledAge = age * age;  // Exponential response
                float wowFreq = 1.f, flutterFreq = 6.f;
                float baseDelayMs = 50.f, maxModDepth = 0.2f;

                float wowPhaseInc = wowFreq * 2.f * M_PI / args.sampleRate;
                float flutterPhaseInc = flutterFreq * 2.f * M_PI / args.sampleRate;

                // L channel
                float modL = (std::sin(wowPhaseL) + std::sin(flutterPhaseL)) * 0.5f * scaledAge;
                float baseDelaySamples = (baseDelayMs / 1000.f) * args.sampleRate;
                float delayL = baseDelaySamples * (1.f + maxModDepth * modL);
                modDelayL.write(sampleL);
                sampleL = modDelayL.read(clamp(delayL, 1.f, 8000.f));

                // R channel
                float modR = (std::sin(wowPhaseR + 0.5f) + std::sin(flutterPhaseR + 0.3f)) * 0.5f * scaledAge;
                float delayR = baseDelaySamples * (1.f + maxModDepth * modR);
                modDelayR.write(sampleR);
                sampleR = modDelayR.read(clamp(delayR, 1.f, 8000.f));

                wowPhaseL += wowPhaseInc; if (wowPhaseL >= 2.f * M_PI) wowPhaseL -= 2.f * M_PI;
                wowPhaseR += wowPhaseInc; if (wowPhaseR >= 2.f * M_PI) wowPhaseR -= 2.f * M_PI;
                flutterPhaseL += flutterPhaseInc; if (flutterPhaseL >= 2.f * M_PI) flutterPhaseL -= 2.f * M_PI;
                flutterPhaseR += flutterPhaseInc; if (flutterPhaseR >= 2.f * M_PI) flutterPhaseR -= 2.f * M_PI;
            }
        };

        auto applyDrive = [&](float& sampleL, float& sampleR) {
            if (drive > 0.f) {
                float gain = 1.f + drive * 9.f;
                sampleL = std::tanh(gain * sampleL);
                sampleR = std::tanh(gain * sampleR);
            }
        };

        auto applyTone = [&](float& sampleL, float& sampleR) {
            if (std::abs(tone) > 0.5f) {
                bool isLP = tone < 0.f;
                if (isLP != previousWasLowPass) { filterL.reset(); filterR.reset(); }
                previousWasLowPass = isLP;

                if (isLP) {
                    float norm = std::abs(tone) / 100.f;
                    float cutoff = clamp(20000.f * std::pow(10.f, -norm * 2.f), 200.f, 20000.f);
                    filterL.setLowPass(args.sampleRate, cutoff, 0.707f);
                    filterR.setLowPass(args.sampleRate, cutoff, 0.707f);
                } else {
                    float norm = tone / 100.f;
                    float cutoff = clamp(20.f * std::pow(10.f, norm * 2.7f), 20.f, 10000.f);
                    filterL.setHighPass(args.sampleRate, cutoff, 0.707f);
                    filterR.setHighPass(args.sampleRate, cutoff, 0.707f);
                }
                sampleL = filterL.process(sampleL);
                sampleR = filterR.process(sampleR);
            }
        };

        // Wet+Dry mode: apply effects before reverb
        if (wetDryMode) {
            applyModulation(inputL, inputR);
            applyDrive(inputL, inputR);
            applyTone(inputL, inputR);
            dryL = inputL; dryR = inputR;
        }

        // Reverb
        float wetL = 0.f, wetR = 0.f;
        for (int i = 0; i < 8; i++) {
            wetL += combL[i].process(inputL);
            wetR += combR[i].process(inputR);
        }
        wetL /= 8.f; wetR /= 8.f;
        for (int i = 0; i < 4; i++) {
            wetL = allpassL[i].process(wetL);
            wetR = allpassR[i].process(wetR);
        }

        // Wet-only mode: apply effects after reverb
        if (!wetDryMode) {
            applyModulation(wetL, wetR);
            applyDrive(wetL, wetR);
            applyTone(wetL, wetR);
        }

        // Mix
        float outL = dryL * (1.f - mix) + wetL * mix;
        float outR = dryR * (1.f - mix) + wetR * mix;

        outputs[LEFT_OUTPUT].setVoltage(outL * 5.f);
        outputs[RIGHT_OUTPUT].setVoltage(outR * 5.f);
    }
};

struct FlutterVerbWidget : ModuleWidget {
    FlutterVerbWidget(FlutterVerb* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/FlutterVerb.svg")));

        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // 10HP = 50.8mm
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10, 25)), module, FlutterVerb::SIZE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(25.4, 25)), module, FlutterVerb::DECAY_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(40.8, 25)), module, FlutterVerb::MIX_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(10, 50)), module, FlutterVerb::AGE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(25.4, 50)), module, FlutterVerb::DRIVE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(40.8, 50)), module, FlutterVerb::TONE_PARAM));
        addParam(createParamCentered<CKSS>(mm2px(Vec(25.4, 75)), module, FlutterVerb::MOD_MODE_PARAM));

        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 95)), module, FlutterVerb::MIX_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10, 112)), module, FlutterVerb::LEFT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22, 112)), module, FlutterVerb::RIGHT_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(34, 112)), module, FlutterVerb::LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(46, 112)), module, FlutterVerb::RIGHT_OUTPUT));
    }
};

Model* modelFlutterVerb = createModel<FlutterVerb, FlutterVerbWidget>("FlutterVerb");
