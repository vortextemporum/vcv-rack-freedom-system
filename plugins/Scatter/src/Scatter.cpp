#include "plugin.hpp"

// Delay buffer
struct ScatterBuffer {
    static const int MAX_SIZE = 96000 * 2;  // 2 seconds at 96kHz
    float buffer[MAX_SIZE] = {0.0f};
    int writePos = 0;

    void write(float sample) {
        buffer[writePos] = sample;
        writePos = (writePos + 1) % MAX_SIZE;
    }

    float read(float delaySamples) const {
        if (delaySamples < 0) delaySamples = 0;
        if (delaySamples >= MAX_SIZE - 1) delaySamples = MAX_SIZE - 2;

        int idx0 = writePos - 1 - (int)delaySamples;
        while (idx0 < 0) idx0 += MAX_SIZE;
        int idx1 = (idx0 + 1) % MAX_SIZE;

        float frac = delaySamples - (int)delaySamples;
        return buffer[idx0] * (1.0f - frac) + buffer[idx1] * frac;
    }

    void clear() {
        memset(buffer, 0, sizeof(buffer));
        writePos = 0;
    }
};

// Grain voice
struct ScatterGrain {
    bool active = false;
    float readPosition = 0.0f;
    float windowPosition = 0.0f;
    float playbackRate = 1.0f;
    int grainSizeSamples = 0;
    float pan = 0.5f;
    bool reverse = false;
};

// Scale tables (semitone offsets from root)
const int CHROMATIC[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
const int MAJOR[] = {0, 2, 4, 5, 7, 9, 11};
const int MINOR[] = {0, 2, 3, 5, 7, 8, 10};
const int PENTATONIC[] = {0, 2, 4, 7, 9};
const int BLUES[] = {0, 3, 5, 6, 7, 10};

struct Scatter : Module {
    enum ParamId {
        DELAY_PARAM,
        SIZE_PARAM,
        DENSITY_PARAM,
        PITCH_PARAM,
        SCALE_PARAM,
        PAN_PARAM,
        FEEDBACK_PARAM,
        MIX_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        LEFT_INPUT,
        RIGHT_INPUT,
        DELAY_CV_INPUT,
        PITCH_CV_INPUT,
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

    ScatterBuffer delayBuffer;
    ScatterGrain grainVoices[MAX_GRAINS];

    float feedbackL = 0.0f;
    float feedbackR = 0.0f;
    int grainSpawnCounter = 0;

    Scatter() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(DELAY_PARAM, 0.1f, 2.0f, 0.5f, "Delay Time", " s");
        configParam(SIZE_PARAM, 0.005f, 0.5f, 0.1f, "Grain Size", " s");
        configParam(DENSITY_PARAM, 0.0f, 1.0f, 0.5f, "Density", "%", 0.0f, 100.0f);
        configParam(PITCH_PARAM, 0.0f, 1.0f, 0.3f, "Pitch Random", "%", 0.0f, 100.0f);
        configParam(SCALE_PARAM, 0.0f, 4.0f, 0.0f, "Scale");
        configParam(PAN_PARAM, 0.0f, 1.0f, 0.75f, "Pan Random", "%", 0.0f, 100.0f);
        configParam(FEEDBACK_PARAM, 0.0f, 0.95f, 0.3f, "Feedback", "%", 0.0f, 100.0f);
        configParam(MIX_PARAM, 0.0f, 1.0f, 0.5f, "Mix", "%", 0.0f, 100.0f);

        configInput(LEFT_INPUT, "Left");
        configInput(RIGHT_INPUT, "Right");
        configInput(DELAY_CV_INPUT, "Delay CV");
        configInput(PITCH_CV_INPUT, "Pitch CV");
        configInput(MIX_CV_INPUT, "Mix CV");

        configOutput(LEFT_OUTPUT, "Left");
        configOutput(RIGHT_OUTPUT, "Right");
    }

    void onReset() override {
        delayBuffer.clear();
        feedbackL = feedbackR = 0.0f;
        for (auto& grain : grainVoices) grain.active = false;
    }

    float getHannWindow(float pos) {
        if (pos < 0.0f) pos = 0.0f;
        if (pos >= 1.0f) pos = 0.9999f;
        return 0.5f * (1.0f - std::cos(2.0f * M_PI * pos));
    }

    int quantizePitchToScale(float pitch, int scaleIndex) {
        const int* scale;
        int scaleSize;

        switch (scaleIndex) {
            case 1: scale = MAJOR; scaleSize = 7; break;
            case 2: scale = MINOR; scaleSize = 7; break;
            case 3: scale = PENTATONIC; scaleSize = 5; break;
            case 4: scale = BLUES; scaleSize = 6; break;
            default: scale = CHROMATIC; scaleSize = 12; break;
        }

        int pitchInt = (int)std::round(pitch);
        int octave = pitchInt / 12;
        int semitone = pitchInt % 12;
        if (semitone < 0) { semitone += 12; octave--; }

        // Find nearest scale degree
        int nearest = scale[0];
        int minDist = std::abs(semitone - scale[0]);
        for (int i = 0; i < scaleSize; i++) {
            int dist = std::abs(semitone - scale[i]);
            if (dist < minDist) {
                minDist = dist;
                nearest = scale[i];
            }
        }

        return clamp(octave * 12 + nearest, -12, 12);
    }

    int findFreeVoice() {
        for (int i = 0; i < MAX_GRAINS; i++) {
            if (!grainVoices[i].active) return i;
        }
        return 0;
    }

    void spawnGrain(float sampleRate, float grainSize, float pitchRandom, float panRandom, int scaleIndex) {
        int idx = findFreeVoice();
        auto& grain = grainVoices[idx];

        grain.grainSizeSamples = (int)(grainSize * sampleRate);
        if (grain.grainSizeSamples < 1) grain.grainSizeSamples = 1;

        grain.readPosition = 0.0f;
        grain.windowPosition = 0.0f;

        // Random pitch
        float randomPitch = (random::uniform() * 2.0f - 1.0f) * 7.0f * pitchRandom;
        int quantizedPitch = quantizePitchToScale(randomPitch, scaleIndex);
        grain.playbackRate = std::pow(2.0f, quantizedPitch / 12.0f);

        // Random pan
        float panAmount = (random::uniform() - 0.5f) * panRandom;
        grain.pan = clamp(0.5f + panAmount, 0.0f, 1.0f);

        // Random reverse (50%)
        grain.reverse = random::uniform() > 0.5f;

        grain.active = true;
    }

    void process(const ProcessArgs& args) override {
        float sampleRate = args.sampleRate;

        // Get parameters
        float delayTime = params[DELAY_PARAM].getValue();
        float grainSize = params[SIZE_PARAM].getValue();
        float density = params[DENSITY_PARAM].getValue();
        float pitchRandom = params[PITCH_PARAM].getValue();
        int scaleIndex = (int)params[SCALE_PARAM].getValue();
        float panRandom = params[PAN_PARAM].getValue();
        float feedback = params[FEEDBACK_PARAM].getValue();
        float mix = params[MIX_PARAM].getValue();

        // CV modulation
        if (inputs[DELAY_CV_INPUT].isConnected()) {
            delayTime += inputs[DELAY_CV_INPUT].getVoltage() * 0.1f;
            delayTime = clamp(delayTime, 0.1f, 2.0f);
        }
        if (inputs[PITCH_CV_INPUT].isConnected()) {
            pitchRandom += inputs[PITCH_CV_INPUT].getVoltage() * 0.1f;
            pitchRandom = clamp(pitchRandom, 0.0f, 1.0f);
        }
        if (inputs[MIX_CV_INPUT].isConnected()) {
            mix += inputs[MIX_CV_INPUT].getVoltage() * 0.1f;
            mix = clamp(mix, 0.0f, 1.0f);
        }

        // Get input (mono mix for grain buffer)
        float inputL = inputs[LEFT_INPUT].getVoltage() / 5.0f;
        float inputR = inputs[RIGHT_INPUT].isConnected() ?
                       inputs[RIGHT_INPUT].getVoltage() / 5.0f : inputL;
        float inputMono = (inputL + inputR) * 0.5f;

        // Write input + feedback to buffer
        delayBuffer.write(inputMono + (feedbackL + feedbackR) * 0.5f * feedback);

        // Grain scheduling
        float densityNorm = std::max(0.01f, density);
        int grainSizeSamples = (int)(grainSize * sampleRate);
        int spawnInterval = (int)(grainSizeSamples / (densityNorm * 2.0f));
        if (spawnInterval < 1) spawnInterval = 1;

        grainSpawnCounter++;
        if (grainSpawnCounter >= spawnInterval) {
            spawnGrain(sampleRate, grainSize, pitchRandom, panRandom, scaleIndex);
            grainSpawnCounter = 0;
        }

        // Process grains
        float wetL = 0.0f, wetR = 0.0f;

        for (auto& grain : grainVoices) {
            if (!grain.active) continue;

            if (grain.windowPosition >= 1.0f) {
                grain.active = false;
                continue;
            }

            float sample = delayBuffer.read(grain.readPosition);
            float window = getHannWindow(grain.windowPosition);
            float grainOut = sample * window;

            // Pan
            wetL += grainOut * (1.0f - grain.pan);
            wetR += grainOut * grain.pan;

            // Advance
            grain.windowPosition += 1.0f / grain.grainSizeSamples;

            if (grain.reverse) {
                grain.readPosition -= grain.playbackRate;
                if (grain.readPosition < 0) grain.readPosition += ScatterBuffer::MAX_SIZE;
            } else {
                grain.readPosition += grain.playbackRate;
                if (grain.readPosition >= ScatterBuffer::MAX_SIZE) grain.readPosition -= ScatterBuffer::MAX_SIZE;
            }
        }

        // Feedback
        feedbackL = wetL;
        feedbackR = wetR;

        // Mix
        float outL = inputL * (1.0f - mix) + wetL * mix;
        float outR = inputR * (1.0f - mix) + wetR * mix;

        outputs[LEFT_OUTPUT].setVoltage(outL * 5.0f);
        outputs[RIGHT_OUTPUT].setVoltage(outR * 5.0f);
    }
};

struct ScatterWidget : ModuleWidget {
    ScatterWidget(Scatter* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Scatter.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(0, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

        // 10HP = 50.8mm
        float col1 = 12.7f;
        float col2 = 38.1f;

        // Top row - Delay/Size
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col1, 25.0f)), module, Scatter::DELAY_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col2, 25.0f)), module, Scatter::SIZE_PARAM));

        // Row 2 - Density/Pitch
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col1, 43.0f)), module, Scatter::DENSITY_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col2, 43.0f)), module, Scatter::PITCH_PARAM));

        // Row 3 - Scale/Pan
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col1, 61.0f)), module, Scatter::SCALE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col2, 61.0f)), module, Scatter::PAN_PARAM));

        // Row 4 - Feedback/Mix
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col1, 79.0f)), module, Scatter::FEEDBACK_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(col2, 79.0f)), module, Scatter::MIX_PARAM));

        // CV inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col1, 95.0f)), module, Scatter::DELAY_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(25.4f, 95.0f)), module, Scatter::PITCH_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, 95.0f)), module, Scatter::MIX_CV_INPUT));

        // Audio inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col1, 108.0f)), module, Scatter::LEFT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, 108.0f)), module, Scatter::RIGHT_INPUT));

        // Audio outputs
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col1, 120.0f)), module, Scatter::LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col2, 120.0f)), module, Scatter::RIGHT_OUTPUT));
    }
};

Model* modelScatter = createModel<Scatter, ScatterWidget>("Scatter");
