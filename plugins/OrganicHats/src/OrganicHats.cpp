#include "plugin.hpp"

// Simple one-pole filter for noise coloring
struct OnePoleFilter {
    float y1 = 0.0f;
    float a0 = 1.0f;
    float b1 = 0.0f;

    void setLowpass(float cutoff, float sampleRate) {
        float w = 2.0f * M_PI * cutoff / sampleRate;
        float cosw = std::cos(w);
        b1 = std::sqrt(2.0f) - cosw - std::sqrt((std::sqrt(2.0f) - cosw) * (std::sqrt(2.0f) - cosw) - 1.0f);
        if (b1 > 0.999f) b1 = 0.999f;
        if (b1 < 0.0f) b1 = 0.0f;
        a0 = 1.0f - b1;
    }

    void setHighpass(float cutoff, float sampleRate) {
        setLowpass(cutoff, sampleRate);
    }

    float processLowpass(float x) {
        y1 = a0 * x + b1 * y1;
        return y1;
    }

    float processHighpass(float x) {
        y1 = a0 * x + b1 * y1;
        return x - y1;
    }

    void reset() {
        y1 = 0.0f;
    }
};

// Bandpass filter (two poles)
struct BandpassFilter {
    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;
    float a0 = 1.0f, a1 = 0.0f, a2 = 0.0f;
    float b1 = 0.0f, b2 = 0.0f;

    void setParams(float freq, float q, float sampleRate) {
        float w0 = 2.0f * M_PI * freq / sampleRate;
        float cosw0 = std::cos(w0);
        float sinw0 = std::sin(w0);
        float alpha = sinw0 / (2.0f * q);

        float norm = 1.0f / (1.0f + alpha);
        a0 = alpha * norm;
        a1 = 0.0f;
        a2 = -alpha * norm;
        b1 = -2.0f * cosw0 * norm;
        b2 = (1.0f - alpha) * norm;
    }

    float process(float x) {
        float y = a0 * x + a1 * x1 + a2 * x2 - b1 * y1 - b2 * y2;
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        return y;
    }

    void reset() {
        x1 = x2 = y1 = y2 = 0.0f;
    }
};

// Hi-hat voice
struct HatVoice {
    // Envelope
    float envelope = 0.0f;
    float decayRate = 0.0f;
    bool active = false;

    // Metallic tone (6 detuned square oscillators)
    float phases[6] = {0.0f};
    float frequencies[6] = {0.0f};

    // Noise filter
    OnePoleFilter noiseFilter;
    BandpassFilter bandpass1, bandpass2;

    float toneLevel = 0.5f;
    float noiseColor = 0.5f;
    bool useHighpass = false;

    void trigger(float tone, float decay, float color, float sampleRate, bool isOpen) {
        active = true;
        envelope = 1.0f;

        // Decay time: closed = 20-150ms, open = 100-800ms
        float decayTime;
        if (isOpen) {
            decayTime = 0.1f + decay * 0.7f;  // 100-800ms
        } else {
            decayTime = 0.02f + decay * 0.13f;  // 20-150ms
        }
        decayRate = 1.0f / (decayTime * sampleRate);

        // Metallic frequencies based on tone
        // Classic 808 hat ratios: 205, 302, 369, 522, 565, 808 Hz (roughly)
        float baseFreq = 200.0f + tone * 600.0f;  // 200-800 Hz base
        frequencies[0] = baseFreq * 1.0f;
        frequencies[1] = baseFreq * 1.47f;
        frequencies[2] = baseFreq * 1.80f;
        frequencies[3] = baseFreq * 2.55f;
        frequencies[4] = baseFreq * 2.76f;
        frequencies[5] = baseFreq * 3.94f;

        // Random phase for natural sound
        for (int i = 0; i < 6; i++) {
            phases[i] = random::uniform();
        }

        // Noise color: 0 = dark (lowpass), 1 = bright (highpass)
        noiseColor = color;
        useHighpass = color > 0.5f;

        float cutoff;
        if (useHighpass) {
            cutoff = 2000.0f + (color - 0.5f) * 2.0f * 8000.0f;  // 2k-10k Hz
            noiseFilter.setHighpass(cutoff, sampleRate);
        } else {
            cutoff = 500.0f + color * 2.0f * 4500.0f;  // 500-5000 Hz
            noiseFilter.setLowpass(cutoff, sampleRate);
        }

        // Bandpass filters for multi-band noise
        bandpass1.setParams(3000.0f + tone * 3000.0f, 2.0f, sampleRate);
        bandpass2.setParams(8000.0f + tone * 4000.0f, 1.5f, sampleRate);

        toneLevel = 0.3f + tone * 0.4f;  // More tone at higher settings
    }

    void choke() {
        // Quick fade out instead of abrupt stop
        decayRate = 1.0f / (0.005f * 44100.0f);  // 5ms choke
    }

    float process(float sampleRate) {
        if (!active) return 0.0f;

        // Generate metallic tone (sum of square waves)
        float tone = 0.0f;
        for (int i = 0; i < 6; i++) {
            phases[i] += frequencies[i] / sampleRate;
            if (phases[i] >= 1.0f) phases[i] -= 1.0f;
            // Square wave with soft edges
            float sq = phases[i] < 0.5f ? 1.0f : -1.0f;
            tone += sq * (1.0f / 6.0f);
        }

        // Generate noise
        float noise = random::uniform() * 2.0f - 1.0f;

        // Filter noise
        float filteredNoise;
        if (useHighpass) {
            filteredNoise = noiseFilter.processHighpass(noise);
        } else {
            filteredNoise = noiseFilter.processLowpass(noise);
        }

        // Add bandpass filtered noise for shimmer
        float bp1 = bandpass1.process(noise);
        float bp2 = bandpass2.process(noise);
        filteredNoise = filteredNoise * 0.6f + bp1 * 0.25f + bp2 * 0.15f;

        // Mix tone and noise
        float mix = tone * toneLevel + filteredNoise * (1.0f - toneLevel * 0.5f);

        // Apply envelope
        float output = mix * envelope * 0.7f;

        // Exponential decay
        envelope -= envelope * decayRate;
        if (envelope < 0.001f) {
            active = false;
            envelope = 0.0f;
        }

        return output;
    }
};

struct OrganicHats : Module {
    enum ParamId {
        CLOSED_TONE_PARAM,
        CLOSED_DECAY_PARAM,
        CLOSED_COLOR_PARAM,
        OPEN_TONE_PARAM,
        OPEN_RELEASE_PARAM,
        OPEN_COLOR_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        CLOSED_TRIG_INPUT,
        OPEN_TRIG_INPUT,
        CLOSED_TONE_CV_INPUT,
        CLOSED_DECAY_CV_INPUT,
        OPEN_TONE_CV_INPUT,
        OPEN_RELEASE_CV_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        CLOSED_LIGHT,
        OPEN_LIGHT,
        LIGHTS_LEN
    };

    // Voices - 4 closed, 8 open (open hats ring longer)
    static const int NUM_CLOSED_VOICES = 4;
    static const int NUM_OPEN_VOICES = 8;
    HatVoice closedVoices[NUM_CLOSED_VOICES];
    HatVoice openVoices[NUM_OPEN_VOICES];
    int nextClosedVoice = 0;
    int nextOpenVoice = 0;

    // Trigger detection
    dsp::SchmittTrigger closedTrigger;
    dsp::SchmittTrigger openTrigger;

    // Light smoothing
    float closedLight = 0.0f;
    float openLight = 0.0f;

    OrganicHats() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        // Closed hi-hat parameters
        configParam(CLOSED_TONE_PARAM, 0.0f, 1.0f, 0.5f, "Closed Tone", "%", 0.0f, 100.0f);
        configParam(CLOSED_DECAY_PARAM, 0.0f, 1.0f, 0.3f, "Closed Decay", "%", 0.0f, 100.0f);
        configParam(CLOSED_COLOR_PARAM, 0.0f, 1.0f, 0.6f, "Closed Noise Color", "%", 0.0f, 100.0f);

        // Open hi-hat parameters
        configParam(OPEN_TONE_PARAM, 0.0f, 1.0f, 0.4f, "Open Tone", "%", 0.0f, 100.0f);
        configParam(OPEN_RELEASE_PARAM, 0.0f, 1.0f, 0.5f, "Open Release", "%", 0.0f, 100.0f);
        configParam(OPEN_COLOR_PARAM, 0.0f, 1.0f, 0.7f, "Open Noise Color", "%", 0.0f, 100.0f);

        // Inputs
        configInput(CLOSED_TRIG_INPUT, "Closed Hi-Hat Trigger");
        configInput(OPEN_TRIG_INPUT, "Open Hi-Hat Trigger");
        configInput(CLOSED_TONE_CV_INPUT, "Closed Tone CV");
        configInput(CLOSED_DECAY_CV_INPUT, "Closed Decay CV");
        configInput(OPEN_TONE_CV_INPUT, "Open Tone CV");
        configInput(OPEN_RELEASE_CV_INPUT, "Open Release CV");

        // Outputs
        configOutput(LEFT_OUTPUT, "Left");
        configOutput(RIGHT_OUTPUT, "Right");
    }

    void process(const ProcessArgs& args) override {
        float sampleRate = args.sampleRate;

        // Get parameters with CV modulation
        float closedTone = params[CLOSED_TONE_PARAM].getValue();
        float closedDecay = params[CLOSED_DECAY_PARAM].getValue();
        float closedColor = params[CLOSED_COLOR_PARAM].getValue();
        float openTone = params[OPEN_TONE_PARAM].getValue();
        float openRelease = params[OPEN_RELEASE_PARAM].getValue();
        float openColor = params[OPEN_COLOR_PARAM].getValue();

        // Apply CV modulation (±5V = ±0.5 range)
        if (inputs[CLOSED_TONE_CV_INPUT].isConnected()) {
            closedTone += inputs[CLOSED_TONE_CV_INPUT].getVoltage() * 0.1f;
            closedTone = clamp(closedTone, 0.0f, 1.0f);
        }
        if (inputs[CLOSED_DECAY_CV_INPUT].isConnected()) {
            closedDecay += inputs[CLOSED_DECAY_CV_INPUT].getVoltage() * 0.1f;
            closedDecay = clamp(closedDecay, 0.0f, 1.0f);
        }
        if (inputs[OPEN_TONE_CV_INPUT].isConnected()) {
            openTone += inputs[OPEN_TONE_CV_INPUT].getVoltage() * 0.1f;
            openTone = clamp(openTone, 0.0f, 1.0f);
        }
        if (inputs[OPEN_RELEASE_CV_INPUT].isConnected()) {
            openRelease += inputs[OPEN_RELEASE_CV_INPUT].getVoltage() * 0.1f;
            openRelease = clamp(openRelease, 0.0f, 1.0f);
        }

        // Trigger detection
        if (closedTrigger.process(inputs[CLOSED_TRIG_INPUT].getVoltage(), 0.1f, 2.0f)) {
            // Closed hi-hat chokes all open hats
            for (int i = 0; i < NUM_OPEN_VOICES; i++) {
                if (openVoices[i].active) {
                    openVoices[i].choke();
                }
            }

            // Trigger new closed hat
            closedVoices[nextClosedVoice].trigger(closedTone, closedDecay, closedColor, sampleRate, false);
            nextClosedVoice = (nextClosedVoice + 1) % NUM_CLOSED_VOICES;
            closedLight = 1.0f;
        }

        if (openTrigger.process(inputs[OPEN_TRIG_INPUT].getVoltage(), 0.1f, 2.0f)) {
            // Trigger new open hat
            openVoices[nextOpenVoice].trigger(openTone, openRelease, openColor, sampleRate, true);
            nextOpenVoice = (nextOpenVoice + 1) % NUM_OPEN_VOICES;
            openLight = 1.0f;
        }

        // Process all voices
        float output = 0.0f;

        for (int i = 0; i < NUM_CLOSED_VOICES; i++) {
            output += closedVoices[i].process(sampleRate);
        }
        for (int i = 0; i < NUM_OPEN_VOICES; i++) {
            output += openVoices[i].process(sampleRate);
        }

        // Soft clip output
        output = std::tanh(output);

        // Output (same signal to both for mono, but stereo ready)
        float outVoltage = output * 5.0f;
        outputs[LEFT_OUTPUT].setVoltage(outVoltage);
        outputs[RIGHT_OUTPUT].setVoltage(outVoltage);

        // Update lights
        closedLight *= 0.999f;
        openLight *= 0.995f;
        lights[CLOSED_LIGHT].setBrightness(closedLight);
        lights[OPEN_LIGHT].setBrightness(openLight);
    }
};

struct OrganicHatsWidget : ModuleWidget {
    OrganicHatsWidget(OrganicHats* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/OrganicHats.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(0, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

        // 10HP = 50.8mm
        float colLeft = 12.7f;   // Left column
        float colRight = 38.1f; // Right column

        // Row positions
        float row1 = 25.0f;   // Labels/Lights
        float row2 = 38.0f;   // Tone knobs
        float row3 = 55.0f;   // Decay/Release knobs
        float row4 = 72.0f;   // Color knobs
        float row5 = 92.0f;   // CV inputs row 1
        float row6 = 105.0f;  // CV inputs row 2
        float row7 = 118.0f;  // Trigger inputs & outputs

        // Closed section (left column)
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(colLeft, row1)), module, OrganicHats::CLOSED_LIGHT));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(colLeft, row2)), module, OrganicHats::CLOSED_TONE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(colLeft, row3)), module, OrganicHats::CLOSED_DECAY_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(colLeft, row4)), module, OrganicHats::CLOSED_COLOR_PARAM));

        // Open section (right column)
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(colRight, row1)), module, OrganicHats::OPEN_LIGHT));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(colRight, row2)), module, OrganicHats::OPEN_TONE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(colRight, row3)), module, OrganicHats::OPEN_RELEASE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(colRight, row4)), module, OrganicHats::OPEN_COLOR_PARAM));

        // CV inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colLeft, row5)), module, OrganicHats::CLOSED_TONE_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colLeft, row6)), module, OrganicHats::CLOSED_DECAY_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colRight, row5)), module, OrganicHats::OPEN_TONE_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colRight, row6)), module, OrganicHats::OPEN_RELEASE_CV_INPUT));

        // Trigger inputs at bottom
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colLeft, row7)), module, OrganicHats::CLOSED_TRIG_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(colRight, row7)), module, OrganicHats::OPEN_TRIG_INPUT));

        // Outputs at very bottom center
        float outY = 128.5f - 10.0f;
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(colLeft, outY)), module, OrganicHats::LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(colRight, outY)), module, OrganicHats::RIGHT_OUTPUT));
    }
};

Model* modelOrganicHats = createModel<OrganicHats, OrganicHatsWidget>("OrganicHats");
