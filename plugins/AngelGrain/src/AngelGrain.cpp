#include "plugin.hpp"

// Circular buffer for grain delay
struct GrainBuffer {
    static const int MAX_SIZE = 96000 * 4;  // 4 seconds at 96kHz
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

// Grain voice
struct GrainVoice {
    bool active = false;
    float readPosition = 0.0f;
    float windowPosition = 0.0f;
    float playbackRate = 1.0f;
    float pan = 0.5f;
    int grainLengthSamples = 0;
};

struct AngelGrain : Module {
    enum ParamId {
        DELAY_PARAM,
        SIZE_PARAM,
        FEEDBACK_PARAM,
        CHAOS_PARAM,
        CHARACTER_PARAM,
        MIX_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        LEFT_INPUT,
        RIGHT_INPUT,
        DELAY_CV_INPUT,
        CHAOS_CV_INPUT,
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

    static const int MAX_GRAINS = 32;

    GrainBuffer grainBuffer;
    GrainVoice grainVoices[MAX_GRAINS];

    float feedbackL = 0.0f;
    float feedbackR = 0.0f;
    int samplesSinceLastGrain = 0;
    int nextGrainInterval = 4410;

    AngelGrain() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(DELAY_PARAM, 0.05f, 2.0f, 0.5f, "Delay Time", " s");
        configParam(SIZE_PARAM, 0.005f, 0.5f, 0.1f, "Grain Size", " s");
        configParam(FEEDBACK_PARAM, 0.0f, 0.95f, 0.3f, "Feedback", "%", 0.0f, 100.0f);
        configParam(CHAOS_PARAM, 0.0f, 1.0f, 0.25f, "Chaos", "%", 0.0f, 100.0f);
        configParam(CHARACTER_PARAM, 0.0f, 1.0f, 0.5f, "Character", "%", 0.0f, 100.0f);
        configParam(MIX_PARAM, 0.0f, 1.0f, 0.5f, "Mix", "%", 0.0f, 100.0f);

        configInput(LEFT_INPUT, "Left");
        configInput(RIGHT_INPUT, "Right");
        configInput(DELAY_CV_INPUT, "Delay CV");
        configInput(CHAOS_CV_INPUT, "Chaos CV");
        configInput(MIX_CV_INPUT, "Mix CV");

        configOutput(LEFT_OUTPUT, "Left");
        configOutput(RIGHT_OUTPUT, "Right");
    }

    void onReset() override {
        grainBuffer.clear();
        feedbackL = feedbackR = 0.0f;
        for (auto& voice : grainVoices) voice.active = false;
    }

    float getTukeyWindow(float pos, float alpha) {
        if (pos < 0.0f) pos = 0.0f;
        if (pos >= 1.0f) pos = 0.9999f;

        if (pos < alpha / 2.0f) {
            return 0.5f * (1.0f - std::cos(2.0f * M_PI * pos / alpha));
        } else if (pos < 1.0f - alpha / 2.0f) {
            return 1.0f;
        } else {
            return 0.5f * (1.0f - std::cos(2.0f * M_PI * (1.0f - pos) / alpha));
        }
    }

    int selectPitchShift(float chaos) {
        if (chaos < 0.01f) return 0;
        if (random::uniform() > chaos) return 0;

        const int pitches[] = {-12, -7, 0, 7, 12};
        return pitches[(int)(random::uniform() * 5) % 5];
    }

    float getPlaybackRate(int semitones) {
        return std::pow(2.0f, semitones / 12.0f);
    }

    int findFreeVoice() {
        for (int i = 0; i < MAX_GRAINS; i++) {
            if (!grainVoices[i].active) return i;
        }
        return 0;  // Steal oldest
    }

    void spawnGrain(float sampleRate, float delayTime, float grainSize, float chaos) {
        int idx = findFreeVoice();
        auto& voice = grainVoices[idx];

        voice.grainLengthSamples = (int)(grainSize * sampleRate);
        if (voice.grainLengthSamples < 1) voice.grainLengthSamples = 1;

        float baseDelay = delayTime * sampleRate;
        float jitter = (random::uniform() - 0.5f) * chaos * 0.5f;
        voice.readPosition = baseDelay * (1.0f + jitter);
        voice.readPosition = clamp(voice.readPosition, 1.0f, (float)(GrainBuffer::MAX_SIZE - 2));

        voice.windowPosition = 0.0f;

        int pitchShift = selectPitchShift(chaos);
        voice.playbackRate = getPlaybackRate(pitchShift);

        float panRandom = (random::uniform() - 0.5f) * 2.0f;
        voice.pan = 0.5f + panRandom * 0.5f * chaos;
        voice.pan = clamp(voice.pan, 0.0f, 1.0f);

        voice.active = true;
    }

    void process(const ProcessArgs& args) override {
        float sampleRate = args.sampleRate;

        // Get parameters with CV modulation
        float delayTime = params[DELAY_PARAM].getValue();
        float grainSize = params[SIZE_PARAM].getValue();
        float feedback = params[FEEDBACK_PARAM].getValue();
        float chaos = params[CHAOS_PARAM].getValue();
        float character = params[CHARACTER_PARAM].getValue();
        float mix = params[MIX_PARAM].getValue();

        // CV modulation
        if (inputs[DELAY_CV_INPUT].isConnected()) {
            delayTime += inputs[DELAY_CV_INPUT].getVoltage() * 0.1f;
            delayTime = clamp(delayTime, 0.05f, 2.0f);
        }
        if (inputs[CHAOS_CV_INPUT].isConnected()) {
            chaos += inputs[CHAOS_CV_INPUT].getVoltage() * 0.1f;
            chaos = clamp(chaos, 0.0f, 1.0f);
        }
        if (inputs[MIX_CV_INPUT].isConnected()) {
            mix += inputs[MIX_CV_INPUT].getVoltage() * 0.1f;
            mix = clamp(mix, 0.0f, 1.0f);
        }

        // Get input (normalize right to left if mono)
        float inputL = inputs[LEFT_INPUT].getVoltage() / 5.0f;
        float inputR = inputs[RIGHT_INPUT].isConnected() ?
                       inputs[RIGHT_INPUT].getVoltage() / 5.0f : inputL;

        // Write input + feedback to buffer
        float writeL = inputL + feedbackL;
        float writeR = inputR + feedbackR;
        grainBuffer.write(writeL, writeR);

        // Grain scheduling
        float densityMult = 1.0f + character * 3.0f;
        nextGrainInterval = (int)((delayTime * sampleRate) / densityMult);
        if (nextGrainInterval < 1) nextGrainInterval = 1;

        int interval = nextGrainInterval;
        if (chaos > 0.01f) {
            float jitter = (random::uniform() - 0.5f) * chaos;
            interval = (int)(nextGrainInterval * (1.0f + jitter));
            if (interval < 1) interval = 1;
        }

        samplesSinceLastGrain++;
        if (samplesSinceLastGrain >= interval) {
            spawnGrain(sampleRate, delayTime, grainSize, chaos);
            samplesSinceLastGrain = 0;
        }

        // Process grains
        float wetL = 0.0f, wetR = 0.0f;
        float tukeyAlpha = 0.1f + character * 0.9f;

        for (auto& voice : grainVoices) {
            if (!voice.active) continue;

            float sampleL = grainBuffer.readL(voice.readPosition);
            float sampleR = grainBuffer.readR(voice.readPosition);

            float window = getTukeyWindow(voice.windowPosition, tukeyAlpha);
            float procL = sampleL * window;
            float procR = sampleR * window;

            // Pan crossfade
            float leftGain = std::cos(voice.pan * M_PI * 0.5f);
            float rightGain = std::sin(voice.pan * M_PI * 0.5f);

            wetL += (procL * leftGain + procR * (1.0f - rightGain)) * 0.707f;
            wetR += (procR * rightGain + procL * (1.0f - leftGain)) * 0.707f;

            // Advance grain
            voice.readPosition -= voice.playbackRate;
            voice.windowPosition += 1.0f / voice.grainLengthSamples;

            if (voice.windowPosition >= 1.0f || voice.readPosition < 0.0f) {
                voice.active = false;
            }
        }

        // Feedback with saturation
        feedbackL = wetL * feedback;
        feedbackR = wetR * feedback;
        if (feedback > 0.5f) {
            feedbackL = std::tanh(feedbackL);
            feedbackR = std::tanh(feedbackR);
        }

        // Mix dry/wet
        float outL = inputL * (1.0f - mix) + wetL * mix;
        float outR = inputR * (1.0f - mix) + wetR * mix;

        outputs[LEFT_OUTPUT].setVoltage(outL * 5.0f);
        outputs[RIGHT_OUTPUT].setVoltage(outR * 5.0f);
    }
};

struct AngelGrainWidget : ModuleWidget {
    AngelGrainWidget(AngelGrain* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/AngelGrain.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(0, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

        // 10HP = 50.8mm
        float centerX = 25.4f;
        float col1 = 12.7f;
        float col2 = 38.1f;

        // Top knobs row
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col1, 28.0f)), module, AngelGrain::DELAY_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col2, 28.0f)), module, AngelGrain::SIZE_PARAM));

        // Middle knobs row
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col1, 48.0f)), module, AngelGrain::FEEDBACK_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col2, 48.0f)), module, AngelGrain::CHAOS_PARAM));

        // Lower knobs row
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col1, 68.0f)), module, AngelGrain::CHARACTER_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col2, 68.0f)), module, AngelGrain::MIX_PARAM));

        // CV inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col1, 88.0f)), module, AngelGrain::DELAY_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(centerX, 88.0f)), module, AngelGrain::CHAOS_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, 88.0f)), module, AngelGrain::MIX_CV_INPUT));

        // Audio inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col1, 105.0f)), module, AngelGrain::LEFT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, 105.0f)), module, AngelGrain::RIGHT_INPUT));

        // Audio outputs
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col1, 118.0f)), module, AngelGrain::LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col2, 118.0f)), module, AngelGrain::RIGHT_OUTPUT));
    }
};

Model* modelAngelGrain = createModel<AngelGrain, AngelGrainWidget>("AngelGrain");
