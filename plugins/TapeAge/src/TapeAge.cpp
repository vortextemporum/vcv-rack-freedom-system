#include "plugin.hpp"

// Delay line for wow/flutter
struct TapeDelayLine {
    static const int MAX_SIZE = 48000;  // ~1 second at 48kHz
    float bufferL[MAX_SIZE] = {0.0f};
    float bufferR[MAX_SIZE] = {0.0f};
    int writePos = 0;

    void write(float L, float R) {
        bufferL[writePos] = L;
        bufferR[writePos] = R;
        writePos = (writePos + 1) % MAX_SIZE;
    }

    float readL(float delaySamples) const {
        if (delaySamples < 0) delaySamples = 0;
        if (delaySamples >= MAX_SIZE - 1) delaySamples = MAX_SIZE - 2;

        int idx0 = writePos - 1 - (int)delaySamples;
        while (idx0 < 0) idx0 += MAX_SIZE;
        int idx1 = (idx0 + 1) % MAX_SIZE;

        float frac = delaySamples - (int)delaySamples;
        return bufferL[idx0] * (1.0f - frac) + bufferL[idx1] * frac;
    }

    float readR(float delaySamples) const {
        if (delaySamples < 0) delaySamples = 0;
        if (delaySamples >= MAX_SIZE - 1) delaySamples = MAX_SIZE - 2;

        int idx0 = writePos - 1 - (int)delaySamples;
        while (idx0 < 0) idx0 += MAX_SIZE;
        int idx1 = (idx0 + 1) % MAX_SIZE;

        float frac = delaySamples - (int)delaySamples;
        return bufferR[idx0] * (1.0f - frac) + bufferR[idx1] * frac;
    }

    void clear() {
        memset(bufferL, 0, sizeof(bufferL));
        memset(bufferR, 0, sizeof(bufferR));
        writePos = 0;
    }
};

struct TapeAge : Module {
    enum ParamId {
        INPUT_PARAM,
        DRIVE_PARAM,
        AGE_PARAM,
        MIX_PARAM,
        OUTPUT_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        LEFT_INPUT,
        RIGHT_INPUT,
        DRIVE_CV_INPUT,
        AGE_CV_INPUT,
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

    TapeDelayLine delayLine;

    // LFO phases for wow/flutter
    float wowPhaseL = 0.0f, wowPhaseR = 0.0f;
    float flutterPhaseL = 0.0f, flutterPhaseR = 0.0f;

    // Noise filter state
    float noiseFilterL = 0.0f, noiseFilterR = 0.0f;

    // Dropout state
    int dropoutCountdown = 4800;
    bool inDropout = false;
    int dropoutRemaining = 0;
    float dropoutEnv = 1.0f;

    // Highpass for DC blocking
    float dcBlockL = 0.0f, dcBlockR = 0.0f;

    TapeAge() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(INPUT_PARAM, -12.0f, 12.0f, 0.0f, "Input Gain", " dB");
        configParam(DRIVE_PARAM, 0.0f, 1.0f, 0.5f, "Drive", "%", 0.0f, 100.0f);
        configParam(AGE_PARAM, 0.0f, 1.0f, 0.25f, "Age", "%", 0.0f, 100.0f);
        configParam(MIX_PARAM, 0.0f, 1.0f, 1.0f, "Mix", "%", 0.0f, 100.0f);
        configParam(OUTPUT_PARAM, -12.0f, 12.0f, 0.0f, "Output Gain", " dB");

        configInput(LEFT_INPUT, "Left");
        configInput(RIGHT_INPUT, "Right");
        configInput(DRIVE_CV_INPUT, "Drive CV");
        configInput(AGE_CV_INPUT, "Age CV");

        configOutput(LEFT_OUTPUT, "Left");
        configOutput(RIGHT_OUTPUT, "Right");

        // Randomize LFO phases
        wowPhaseL = random::uniform() * 2.0f * M_PI;
        wowPhaseR = random::uniform() * 2.0f * M_PI;
        flutterPhaseL = random::uniform() * 2.0f * M_PI;
        flutterPhaseR = random::uniform() * 2.0f * M_PI;
    }

    void onReset() override {
        delayLine.clear();
        noiseFilterL = noiseFilterR = 0.0f;
        dcBlockL = dcBlockR = 0.0f;
        dropoutEnv = 1.0f;
        inDropout = false;
    }

    void process(const ProcessArgs& args) override {
        float sampleRate = args.sampleRate;

        // Get parameters
        float inputDB = params[INPUT_PARAM].getValue();
        float drive = params[DRIVE_PARAM].getValue();
        float age = params[AGE_PARAM].getValue();
        float mix = params[MIX_PARAM].getValue();
        float outputDB = params[OUTPUT_PARAM].getValue();

        // CV modulation
        if (inputs[DRIVE_CV_INPUT].isConnected()) {
            drive += inputs[DRIVE_CV_INPUT].getVoltage() * 0.1f;
            drive = clamp(drive, 0.0f, 1.0f);
        }
        if (inputs[AGE_CV_INPUT].isConnected()) {
            age += inputs[AGE_CV_INPUT].getVoltage() * 0.1f;
            age = clamp(age, 0.0f, 1.0f);
        }

        // Calculate gains
        float inputGain = std::pow(10.0f, inputDB / 20.0f);
        float outputGain = std::pow(10.0f, outputDB / 20.0f);

        // Calculate drive gain (progressive curve)
        float driveGain;
        if (drive <= 0.3f) {
            driveGain = 1.0f + (drive / 0.3f);
        } else if (drive <= 0.7f) {
            driveGain = 2.0f + ((drive - 0.3f) / 0.4f) * 6.0f;
        } else {
            driveGain = 8.0f + ((drive - 0.7f) / 0.3f) * 12.0f;
        }
        float makeupGain = 1.0f / std::sqrt(driveGain);

        // Get input
        float dryL = inputs[LEFT_INPUT].getVoltage() / 5.0f;
        float dryR = inputs[RIGHT_INPUT].isConnected() ?
                     inputs[RIGHT_INPUT].getVoltage() / 5.0f : dryL;

        // Apply input gain
        float wetL = dryL * inputGain;
        float wetR = dryR * inputGain;

        // === Saturation ===
        wetL = std::tanh(wetL * driveGain) * makeupGain;
        wetR = std::tanh(wetR * driveGain) * makeupGain;

        // === Wow/Flutter (pitch modulation via delay) ===
        float wowFreq = 1.0f + age;  // 1-2 Hz
        float flutterFreq = 6.0f;
        float wowInc = (wowFreq * 2.0f * M_PI) / sampleRate;
        float flutterInc = (flutterFreq * 2.0f * M_PI) / sampleRate;

        // Modulation depth based on age (±25 cents max)
        float modDepth = age * 0.0145f;  // ~25 cents

        // Calculate combined modulation
        float modL = std::sin(wowPhaseL) + std::sin(flutterPhaseL) * 0.2f;
        float modR = std::sin(wowPhaseR) + std::sin(flutterPhaseR) * 0.2f;

        // Write to delay line
        delayLine.write(wetL, wetR);

        // Calculate modulated delay (base 50ms + modulation)
        float baseDelay = sampleRate * 0.05f;
        float delayL = baseDelay + modL * modDepth * baseDelay;
        float delayR = baseDelay + modR * modDepth * baseDelay;

        // Read from delay line
        wetL = delayLine.readL(delayL);
        wetR = delayLine.readR(delayR);

        // Advance LFO phases
        wowPhaseL += wowInc;
        wowPhaseR += wowInc;
        flutterPhaseL += flutterInc;
        flutterPhaseR += flutterInc;
        if (wowPhaseL >= 2.0f * M_PI) wowPhaseL -= 2.0f * M_PI;
        if (wowPhaseR >= 2.0f * M_PI) wowPhaseR -= 2.0f * M_PI;
        if (flutterPhaseL >= 2.0f * M_PI) flutterPhaseL -= 2.0f * M_PI;
        if (flutterPhaseR >= 2.0f * M_PI) flutterPhaseR -= 2.0f * M_PI;

        // === Age-dependent lowpass (high frequency rolloff) ===
        if (age > 0.01f) {
            // Simple one-pole lowpass: 20kHz -> 8kHz based on age
            float cutoff = 20000.0f * std::pow(0.4f, age);
            float coef = 1.0f - std::exp(-2.0f * M_PI * cutoff / sampleRate);
            dcBlockL += coef * (wetL - dcBlockL);
            dcBlockR += coef * (wetR - dcBlockR);
            wetL = dcBlockL;
            wetR = dcBlockR;
        }

        // === Dropout events ===
        dropoutCountdown--;
        if (dropoutCountdown <= 0) {
            dropoutCountdown = (int)(sampleRate * 0.1f);  // Check every 100ms

            if (random::uniform() < age * 0.02f && !inDropout) {
                inDropout = true;
                dropoutRemaining = (int)(sampleRate * (0.05f + random::uniform() * 0.1f));
            }
        }

        // Process dropout envelope
        float envRate = 1.0f / (sampleRate * 0.0075f);  // 7.5ms attack/release
        if (inDropout && dropoutRemaining > 0) {
            float target = 0.1f + random::uniform() * 0.2f;
            if (dropoutEnv > target) dropoutEnv -= envRate;
            dropoutRemaining--;
            if (dropoutRemaining <= 0) inDropout = false;
        } else if (dropoutEnv < 1.0f) {
            dropoutEnv += envRate;
            if (dropoutEnv > 1.0f) dropoutEnv = 1.0f;
        }

        wetL *= dropoutEnv;
        wetR *= dropoutEnv;

        // === Tape noise ===
        float noiseGain = age * 0.001f;  // -60dB at full age
        if (noiseGain > 0.0f) {
            float noiseCoef = 1.0f - std::exp(-2.0f * M_PI * 8000.0f / sampleRate);
            float noiseL = random::uniform() * 2.0f - 1.0f;
            float noiseR = random::uniform() * 2.0f - 1.0f;
            noiseFilterL += noiseCoef * (noiseL - noiseFilterL);
            noiseFilterR += noiseCoef * (noiseR - noiseFilterR);
            wetL += noiseFilterL * noiseGain;
            wetR += noiseFilterR * noiseGain;
        }

        // === Mix dry/wet ===
        float outL = dryL * (1.0f - mix) + wetL * mix;
        float outR = dryR * (1.0f - mix) + wetR * mix;

        // Apply output gain
        outL *= outputGain;
        outR *= outputGain;

        outputs[LEFT_OUTPUT].setVoltage(outL * 5.0f);
        outputs[RIGHT_OUTPUT].setVoltage(outR * 5.0f);
    }
};

struct TapeAgeWidget : ModuleWidget {
    TapeAgeWidget(TapeAge* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/TapeAge.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(0, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

        // 8HP = 40.64mm
        float centerX = 20.32f;

        // Knobs in column
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(centerX, 25.0f)), module, TapeAge::INPUT_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(centerX, 43.0f)), module, TapeAge::DRIVE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(centerX, 61.0f)), module, TapeAge::AGE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(centerX, 79.0f)), module, TapeAge::MIX_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(centerX, 97.0f)), module, TapeAge::OUTPUT_PARAM));

        // CV inputs
        float cvY = 110.0f;
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0f, cvY)), module, TapeAge::DRIVE_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.0f, cvY)), module, TapeAge::AGE_CV_INPUT));

        // Audio I/O
        float ioY = 120.0f;
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0f, ioY)), module, TapeAge::LEFT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(20.32f, ioY)), module, TapeAge::RIGHT_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.0f, ioY)), module, TapeAge::LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.0f, ioY)), module, TapeAge::RIGHT_OUTPUT));
    }
};

Model* modelTapeAge = createModel<TapeAge, TapeAgeWidget>("TapeAge");
