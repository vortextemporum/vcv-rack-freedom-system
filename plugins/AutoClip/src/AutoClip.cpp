#include "plugin.hpp"

// Simple ring buffer for lookahead delay
struct RingBuffer {
    static constexpr int MAX_SIZE = 4096;
    float buffer[MAX_SIZE] = {};
    int writePos = 0;
    int size = 256;

    void setSize(int newSize) {
        size = clamp(newSize, 1, MAX_SIZE - 1);
    }

    void push(float sample) {
        buffer[writePos] = sample;
        writePos = (writePos + 1) % size;
    }

    float read(int delay) const {
        int readPos = (writePos - delay + size * 2) % size;
        return buffer[readPos];
    }

    void clear() {
        std::fill(std::begin(buffer), std::end(buffer), 0.f);
        writePos = 0;
    }
};

struct AutoClip : Module {
    enum ParamId {
        THRESHOLD_PARAM,
        SOLO_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        LEFT_INPUT,
        RIGHT_INPUT,
        THRESHOLD_CV_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        CLIP_LIGHT,
        LIGHTS_LEN
    };

    // Lookahead delay buffers (5ms at 48kHz ≈ 240 samples)
    RingBuffer delayL;
    RingBuffer delayR;
    int lookaheadSamples = 240;

    // Gain smoothing
    float smoothedGain = 1.f;
    float gainSmoothingCoeff = 0.001f;  // ~50ms at 48kHz

    // Peak detection (per block, approximated with decay)
    float inputPeak = 0.f;
    float outputPeak = 0.f;
    float peakDecay = 0.9999f;

    // Clip indicator
    dsp::PulseGenerator clipPulse;
    bool isClipping = false;

    AutoClip() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        // Threshold: 0-100% (0% = no clipping, 100% = max clipping)
        configParam(THRESHOLD_PARAM, 0.f, 100.f, 0.f, "Clip Threshold", "%");

        // Solo: toggle to hear only what's clipped
        configSwitch(SOLO_PARAM, 0.f, 1.f, 0.f, "Clip Solo", {"Off", "On"});

        configInput(LEFT_INPUT, "Left audio");
        configInput(RIGHT_INPUT, "Right audio");
        configInput(THRESHOLD_CV_INPUT, "Threshold CV");

        configOutput(LEFT_OUTPUT, "Left audio");
        configOutput(RIGHT_OUTPUT, "Right audio");

        configBypass(LEFT_INPUT, LEFT_OUTPUT);
        configBypass(RIGHT_INPUT, RIGHT_OUTPUT);
    }

    void onSampleRateChange() override {
        float sampleRate = APP->engine->getSampleRate();
        lookaheadSamples = static_cast<int>(0.005f * sampleRate);  // 5ms
        delayL.setSize(lookaheadSamples + 1);
        delayR.setSize(lookaheadSamples + 1);
        delayL.clear();
        delayR.clear();

        // Gain smoothing: ~50ms time constant
        gainSmoothingCoeff = 1.f - std::exp(-1.f / (0.05f * sampleRate));
    }

    void process(const ProcessArgs& args) override {
        // Read threshold parameter with CV
        float thresholdPercent = params[THRESHOLD_PARAM].getValue();
        if (inputs[THRESHOLD_CV_INPUT].isConnected()) {
            thresholdPercent += inputs[THRESHOLD_CV_INPUT].getVoltage() * 10.f;
            thresholdPercent = clamp(thresholdPercent, 0.f, 100.f);
        }

        // Convert to actual threshold (0% = 1.0, 100% = 0.0 threshold)
        // Inverted: higher % = more clipping
        float clipThreshold = 1.f - (thresholdPercent * 0.01f);
        clipThreshold = std::max(clipThreshold, 0.01f);  // Prevent divide by zero

        bool soloClipped = params[SOLO_PARAM].getValue() > 0.5f;

        // Get inputs (normalize to ±1 range for processing, then scale back)
        float inputL = inputs[LEFT_INPUT].getVoltage() / 5.f;
        float inputR = inputs[RIGHT_INPUT].isConnected() ?
                       inputs[RIGHT_INPUT].getVoltage() / 5.f : inputL;

        // Original signal not needed since we use delayed version

        // Push to lookahead buffers
        delayL.push(inputL);
        delayR.push(inputR);

        // Read delayed samples
        float delayedL = delayL.read(lookaheadSamples);
        float delayedR = delayR.read(lookaheadSamples);

        // Track input peaks (with decay)
        inputPeak = std::max(inputPeak * peakDecay, std::max(std::abs(delayedL), std::abs(delayedR)));

        // Apply hard clipping
        float clippedL = clamp(delayedL, -clipThreshold, clipThreshold);
        float clippedR = clamp(delayedR, -clipThreshold, clipThreshold);

        // Track output peaks
        outputPeak = std::max(outputPeak * peakDecay, std::max(std::abs(clippedL), std::abs(clippedR)));

        // Detect if clipping occurred
        isClipping = (std::abs(delayedL) > clipThreshold || std::abs(delayedR) > clipThreshold);
        if (isClipping) {
            clipPulse.trigger(0.05f);
        }

        // Calculate target gain for compensation
        float targetGain = 1.f;
        if (outputPeak > 0.001f && inputPeak > 0.001f) {
            targetGain = inputPeak / outputPeak;
        }

        // Smooth the gain
        smoothedGain += gainSmoothingCoeff * (targetGain - smoothedGain);

        // Apply gain compensation
        float outputL = clippedL * smoothedGain;
        float outputR = clippedR * smoothedGain;

        // Clip solo: output difference signal
        if (soloClipped) {
            // Use undelayed original for proper alignment
            float delayedOrigL = delayL.read(lookaheadSamples);
            float delayedOrigR = delayR.read(lookaheadSamples);
            outputL = delayedOrigL - clippedL * smoothedGain;
            outputR = delayedOrigR - clippedR * smoothedGain;
        }

        // Output (scale back to ±5V)
        outputs[LEFT_OUTPUT].setVoltage(outputL * 5.f);
        outputs[RIGHT_OUTPUT].setVoltage(outputR * 5.f);

        // Clip indicator light
        lights[CLIP_LIGHT].setBrightness(clipPulse.process(args.sampleTime) ? 1.f : 0.f);
    }
};

struct AutoClipWidget : ModuleWidget {
    AutoClipWidget(AutoClip* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/AutoClip.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Big threshold knob (center)
        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 40)), module, AutoClip::THRESHOLD_PARAM));

        // Solo switch
        addParam(createParamCentered<CKSS>(mm2px(Vec(15.24, 65)), module, AutoClip::SOLO_PARAM));

        // Clip light
        addChild(createLightCentered<LargeLight<RedLight>>(mm2px(Vec(15.24, 80)), module, AutoClip::CLIP_LIGHT));

        // CV input
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(15.24, 95)), module, AutoClip::THRESHOLD_CV_INPUT));

        // Audio I/O
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 110)), module, AutoClip::LEFT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.48, 110)), module, AutoClip::RIGHT_INPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8, 120)), module, AutoClip::LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.48, 120)), module, AutoClip::RIGHT_OUTPUT));
    }
};

Model* modelAutoClip = createModel<AutoClip, AutoClipWidget>("AutoClip");
