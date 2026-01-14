#include "plugin.hpp"

// Bandpass filter for tom/clap tones
struct BiquadBandpass {
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

// Kick voice
struct KickVoice {
    bool active = false;
    float velocity = 1.0f;
    float phase = 0.0f;
    float time = 0.0f;

    void trigger(float vel) {
        active = true;
        velocity = vel;
        phase = 0.0f;
        time = 0.0f;
    }

    float process(float level, float decay, float sampleRate) {
        if (!active) return 0.0f;

        // Base frequency with pitch envelope
        float baseFreq = 55.0f;  // A1
        float pitchEnv = 1.0f + 3.0f * std::exp(-time / 0.02f);  // Pitch sweep
        float freq = baseFreq * pitchEnv;

        // Sine oscillator
        phase += freq / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;
        float osc = std::sin(2.0f * M_PI * phase);

        // Amplitude envelope
        float decayTime = 0.1f + decay * 0.9f;  // 100-1000ms
        float env = std::exp(-time / decayTime);

        time += 1.0f / sampleRate;

        if (env < 0.001f) {
            active = false;
            return 0.0f;
        }

        return osc * env * velocity * level;
    }
};

// Tom voice
struct TomVoice {
    bool active = false;
    float velocity = 1.0f;
    float phase = 0.0f;
    float time = 0.0f;
    float baseFreq = 100.0f;
    BiquadBandpass filter;

    void trigger(float vel, float freq) {
        active = true;
        velocity = vel;
        phase = 0.0f;
        time = 0.0f;
        baseFreq = freq;
    }

    float process(float level, float decay, float sampleRate) {
        if (!active) return 0.0f;

        // Pitch envelope
        float pitchEnv = 1.0f + 0.5f * std::exp(-time / 0.03f);
        float freq = baseFreq * pitchEnv;

        // Sine oscillator
        phase += freq / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;
        float osc = std::sin(2.0f * M_PI * phase);

        // Filter
        filter.setParams(freq, 2.0f, sampleRate);
        float filtered = filter.process(osc);

        // Amplitude envelope
        float decayTime = 0.05f + decay * 0.45f;  // 50-500ms
        float env = std::exp(-time / decayTime);

        time += 1.0f / sampleRate;

        if (env < 0.001f) {
            active = false;
            return 0.0f;
        }

        return filtered * env * velocity * level;
    }
};

// Clap voice
struct ClapVoice {
    bool active = false;
    float velocity = 1.0f;
    int sampleCount = 0;
    BiquadBandpass filter;
    int spike2Start, spike3Start, decayStart;

    void trigger(float vel, float sampleRate) {
        active = true;
        velocity = vel;
        sampleCount = 0;
        spike2Start = (int)(sampleRate * 0.010f);
        spike3Start = (int)(sampleRate * 0.020f);
        decayStart = (int)(sampleRate * 0.030f);
    }

    float process(float level, float tone, float sampleRate) {
        if (!active) return 0.0f;

        // White noise
        float noise = random::uniform() * 2.0f - 1.0f;

        // Bandpass filter (1-3kHz range)
        float freq = 1000.0f + tone * 2000.0f;
        filter.setParams(freq, 3.0f, sampleRate);
        float filtered = filter.process(noise);

        // Multi-spike envelope
        float env = 0.0f;
        float t = sampleCount / sampleRate;

        if (sampleCount < spike2Start) {
            env = std::exp(-t / 0.003f);  // First spike
        } else if (sampleCount < spike3Start) {
            float tSpike = (sampleCount - spike2Start) / sampleRate;
            env = 0.6f * std::exp(-tSpike / 0.003f);  // Second spike
        } else if (sampleCount < decayStart) {
            float tSpike = (sampleCount - spike3Start) / sampleRate;
            env = 0.3f * std::exp(-tSpike / 0.003f);  // Third spike
        } else {
            float tDecay = (sampleCount - decayStart) / sampleRate;
            env = std::exp(-tDecay / 0.2f);  // Main decay
        }

        sampleCount++;

        if (env < 0.001f && sampleCount > decayStart) {
            active = false;
            return 0.0f;
        }

        return filtered * env * velocity * level;
    }
};

// Hi-hat voice (shared between closed and open)
struct HiHatVoice {
    bool active = false;
    float velocity = 1.0f;
    float time = 0.0f;
    float phases[6] = {0.0f};
    BiquadBandpass filter;

    void trigger(float vel) {
        active = true;
        velocity = vel;
        time = 0.0f;
        for (int i = 0; i < 6; i++) {
            phases[i] = random::uniform();
        }
    }

    void choke() {
        active = false;
    }

    float process(float level, float decay, float sampleRate) {
        if (!active) return 0.0f;

        // 6 detuned square wave oscillators (808 hat frequencies)
        float baseFreq = 320.0f;
        float ratios[6] = {1.0f, 1.47f, 1.80f, 2.55f, 2.76f, 3.94f};

        float mixed = 0.0f;
        for (int i = 0; i < 6; i++) {
            phases[i] += (baseFreq * ratios[i]) / sampleRate;
            if (phases[i] >= 1.0f) phases[i] -= 1.0f;
            // Square wave
            float sq = phases[i] < 0.5f ? 1.0f : -1.0f;
            mixed += sq / 6.0f;
        }

        // High-pass filtered noise
        float noise = random::uniform() * 2.0f - 1.0f;

        // Bandpass at high frequencies
        filter.setParams(8000.0f, 2.0f, sampleRate);
        float filteredMix = filter.process(mixed + noise * 0.5f);

        // Envelope
        float decayTime = 0.02f + decay * 0.78f;  // 20-800ms
        float env = std::exp(-time / decayTime);

        time += 1.0f / sampleRate;

        if (env < 0.001f) {
            active = false;
            return 0.0f;
        }

        return filteredMix * env * velocity * level * 0.5f;
    }
};

struct Drum808 : Module {
    enum ParamId {
        KICK_LEVEL_PARAM,
        KICK_DECAY_PARAM,
        LOWTOM_LEVEL_PARAM,
        LOWTOM_DECAY_PARAM,
        MIDTOM_LEVEL_PARAM,
        MIDTOM_DECAY_PARAM,
        CLAP_LEVEL_PARAM,
        CLAP_TONE_PARAM,
        CLOSEDHAT_LEVEL_PARAM,
        CLOSEDHAT_DECAY_PARAM,
        OPENHAT_LEVEL_PARAM,
        OPENHAT_DECAY_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        KICK_TRIG_INPUT,
        LOWTOM_TRIG_INPUT,
        MIDTOM_TRIG_INPUT,
        CLAP_TRIG_INPUT,
        CLOSEDHAT_TRIG_INPUT,
        OPENHAT_TRIG_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        MAIN_LEFT_OUTPUT,
        MAIN_RIGHT_OUTPUT,
        KICK_OUTPUT,
        LOWTOM_OUTPUT,
        MIDTOM_OUTPUT,
        CLAP_OUTPUT,
        CLOSEDHAT_OUTPUT,
        OPENHAT_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        KICK_LIGHT,
        LOWTOM_LIGHT,
        MIDTOM_LIGHT,
        CLAP_LIGHT,
        CLOSEDHAT_LIGHT,
        OPENHAT_LIGHT,
        LIGHTS_LEN
    };

    // Voices
    KickVoice kick;
    TomVoice lowTom, midTom;
    ClapVoice clap;
    HiHatVoice closedHat, openHat;

    // Triggers
    dsp::SchmittTrigger kickTrig, lowTomTrig, midTomTrig, clapTrig, closedHatTrig, openHatTrig;

    // Lights
    float kickLight = 0.0f, lowTomLight = 0.0f, midTomLight = 0.0f;
    float clapLight = 0.0f, closedHatLight = 0.0f, openHatLight = 0.0f;

    Drum808() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(KICK_LEVEL_PARAM, 0.0f, 1.0f, 0.8f, "Kick Level", "%", 0.0f, 100.0f);
        configParam(KICK_DECAY_PARAM, 0.0f, 1.0f, 0.5f, "Kick Decay", "%", 0.0f, 100.0f);

        configParam(LOWTOM_LEVEL_PARAM, 0.0f, 1.0f, 0.75f, "Low Tom Level", "%", 0.0f, 100.0f);
        configParam(LOWTOM_DECAY_PARAM, 0.0f, 1.0f, 0.4f, "Low Tom Decay", "%", 0.0f, 100.0f);

        configParam(MIDTOM_LEVEL_PARAM, 0.0f, 1.0f, 0.75f, "Mid Tom Level", "%", 0.0f, 100.0f);
        configParam(MIDTOM_DECAY_PARAM, 0.0f, 1.0f, 0.35f, "Mid Tom Decay", "%", 0.0f, 100.0f);

        configParam(CLAP_LEVEL_PARAM, 0.0f, 1.0f, 0.7f, "Clap Level", "%", 0.0f, 100.0f);
        configParam(CLAP_TONE_PARAM, 0.0f, 1.0f, 0.5f, "Clap Tone", "%", 0.0f, 100.0f);

        configParam(CLOSEDHAT_LEVEL_PARAM, 0.0f, 1.0f, 0.65f, "Closed Hat Level", "%", 0.0f, 100.0f);
        configParam(CLOSEDHAT_DECAY_PARAM, 0.0f, 1.0f, 0.2f, "Closed Hat Decay", "%", 0.0f, 100.0f);

        configParam(OPENHAT_LEVEL_PARAM, 0.0f, 1.0f, 0.6f, "Open Hat Level", "%", 0.0f, 100.0f);
        configParam(OPENHAT_DECAY_PARAM, 0.0f, 1.0f, 0.6f, "Open Hat Decay", "%", 0.0f, 100.0f);

        configInput(KICK_TRIG_INPUT, "Kick Trigger");
        configInput(LOWTOM_TRIG_INPUT, "Low Tom Trigger");
        configInput(MIDTOM_TRIG_INPUT, "Mid Tom Trigger");
        configInput(CLAP_TRIG_INPUT, "Clap Trigger");
        configInput(CLOSEDHAT_TRIG_INPUT, "Closed Hat Trigger");
        configInput(OPENHAT_TRIG_INPUT, "Open Hat Trigger");

        configOutput(MAIN_LEFT_OUTPUT, "Main Left");
        configOutput(MAIN_RIGHT_OUTPUT, "Main Right");
        configOutput(KICK_OUTPUT, "Kick");
        configOutput(LOWTOM_OUTPUT, "Low Tom");
        configOutput(MIDTOM_OUTPUT, "Mid Tom");
        configOutput(CLAP_OUTPUT, "Clap");
        configOutput(CLOSEDHAT_OUTPUT, "Closed Hat");
        configOutput(OPENHAT_OUTPUT, "Open Hat");
    }

    void process(const ProcessArgs& args) override {
        float sampleRate = args.sampleRate;

        // Get parameters
        float kickLevel = params[KICK_LEVEL_PARAM].getValue();
        float kickDecay = params[KICK_DECAY_PARAM].getValue();
        float lowTomLevel = params[LOWTOM_LEVEL_PARAM].getValue();
        float lowTomDecay = params[LOWTOM_DECAY_PARAM].getValue();
        float midTomLevel = params[MIDTOM_LEVEL_PARAM].getValue();
        float midTomDecay = params[MIDTOM_DECAY_PARAM].getValue();
        float clapLevel = params[CLAP_LEVEL_PARAM].getValue();
        float clapTone = params[CLAP_TONE_PARAM].getValue();
        float closedHatLevel = params[CLOSEDHAT_LEVEL_PARAM].getValue();
        float closedHatDecay = params[CLOSEDHAT_DECAY_PARAM].getValue();
        float openHatLevel = params[OPENHAT_LEVEL_PARAM].getValue();
        float openHatDecay = params[OPENHAT_DECAY_PARAM].getValue();

        // Check triggers
        if (kickTrig.process(inputs[KICK_TRIG_INPUT].getVoltage(), 0.1f, 2.0f)) {
            kick.trigger(1.0f);
            kickLight = 1.0f;
        }
        if (lowTomTrig.process(inputs[LOWTOM_TRIG_INPUT].getVoltage(), 0.1f, 2.0f)) {
            lowTom.trigger(1.0f, 110.0f);  // Low Tom at A2
            lowTomLight = 1.0f;
        }
        if (midTomTrig.process(inputs[MIDTOM_TRIG_INPUT].getVoltage(), 0.1f, 2.0f)) {
            midTom.trigger(1.0f, 165.0f);  // Mid Tom at E3
            midTomLight = 1.0f;
        }
        if (clapTrig.process(inputs[CLAP_TRIG_INPUT].getVoltage(), 0.1f, 2.0f)) {
            clap.trigger(1.0f, sampleRate);
            clapLight = 1.0f;
        }
        if (closedHatTrig.process(inputs[CLOSEDHAT_TRIG_INPUT].getVoltage(), 0.1f, 2.0f)) {
            // Closed hat chokes open hat
            openHat.choke();
            closedHat.trigger(1.0f);
            closedHatLight = 1.0f;
        }
        if (openHatTrig.process(inputs[OPENHAT_TRIG_INPUT].getVoltage(), 0.1f, 2.0f)) {
            openHat.trigger(1.0f);
            openHatLight = 1.0f;
        }

        // Process voices
        float kickOut = kick.process(kickLevel, kickDecay, sampleRate);
        float lowTomOut = lowTom.process(lowTomLevel, lowTomDecay, sampleRate);
        float midTomOut = midTom.process(midTomLevel, midTomDecay, sampleRate);
        float clapOut = clap.process(clapLevel, clapTone, sampleRate);
        float closedHatOut = closedHat.process(closedHatLevel, closedHatDecay, sampleRate);
        float openHatOut = openHat.process(openHatLevel, openHatDecay, sampleRate);

        // Mix all voices
        float mix = kickOut + lowTomOut + midTomOut + clapOut + closedHatOut + openHatOut;
        mix = std::tanh(mix);  // Soft clipping

        // Main output (stereo)
        float mainOut = mix * 5.0f;
        outputs[MAIN_LEFT_OUTPUT].setVoltage(mainOut);
        outputs[MAIN_RIGHT_OUTPUT].setVoltage(mainOut);

        // Individual outputs
        outputs[KICK_OUTPUT].setVoltage(kickOut * 5.0f);
        outputs[LOWTOM_OUTPUT].setVoltage(lowTomOut * 5.0f);
        outputs[MIDTOM_OUTPUT].setVoltage(midTomOut * 5.0f);
        outputs[CLAP_OUTPUT].setVoltage(clapOut * 5.0f);
        outputs[CLOSEDHAT_OUTPUT].setVoltage(closedHatOut * 5.0f);
        outputs[OPENHAT_OUTPUT].setVoltage(openHatOut * 5.0f);

        // Update lights
        const float lightDecay = 0.999f;
        kickLight *= lightDecay;
        lowTomLight *= lightDecay;
        midTomLight *= lightDecay;
        clapLight *= lightDecay;
        closedHatLight *= lightDecay;
        openHatLight *= lightDecay;

        lights[KICK_LIGHT].setBrightness(kickLight);
        lights[LOWTOM_LIGHT].setBrightness(lowTomLight);
        lights[MIDTOM_LIGHT].setBrightness(midTomLight);
        lights[CLAP_LIGHT].setBrightness(clapLight);
        lights[CLOSEDHAT_LIGHT].setBrightness(closedHatLight);
        lights[OPENHAT_LIGHT].setBrightness(openHatLight);
    }
};

struct Drum808Widget : ModuleWidget {
    Drum808Widget(Drum808* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Drum808.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(0, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

        // 24HP = 121.92mm
        // Layout: 6 rows for 6 voices
        // Each row: Light, Trig, Level, Decay, IndivOut

        float col1 = 10.0f;   // Light
        float col2 = 22.0f;   // Trigger
        float col3 = 38.0f;   // Level knob
        float col4 = 58.0f;   // Decay/Tone knob
        float col5 = 74.0f;   // Individual output

        float rowHeight = 15.0f;
        float startY = 28.0f;

        // Kick row
        float y = startY;
        addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(col1, y)), module, Drum808::KICK_LIGHT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, y)), module, Drum808::KICK_TRIG_INPUT));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col3, y)), module, Drum808::KICK_LEVEL_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col4, y)), module, Drum808::KICK_DECAY_PARAM));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col5, y)), module, Drum808::KICK_OUTPUT));

        // Low Tom row
        y += rowHeight;
        addChild(createLightCentered<SmallLight<YellowLight>>(mm2px(Vec(col1, y)), module, Drum808::LOWTOM_LIGHT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, y)), module, Drum808::LOWTOM_TRIG_INPUT));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col3, y)), module, Drum808::LOWTOM_LEVEL_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col4, y)), module, Drum808::LOWTOM_DECAY_PARAM));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col5, y)), module, Drum808::LOWTOM_OUTPUT));

        // Mid Tom row
        y += rowHeight;
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(col1, y)), module, Drum808::MIDTOM_LIGHT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, y)), module, Drum808::MIDTOM_TRIG_INPUT));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col3, y)), module, Drum808::MIDTOM_LEVEL_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col4, y)), module, Drum808::MIDTOM_DECAY_PARAM));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col5, y)), module, Drum808::MIDTOM_OUTPUT));

        // Clap row
        y += rowHeight;
        addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(col1, y)), module, Drum808::CLAP_LIGHT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, y)), module, Drum808::CLAP_TRIG_INPUT));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col3, y)), module, Drum808::CLAP_LEVEL_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col4, y)), module, Drum808::CLAP_TONE_PARAM));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col5, y)), module, Drum808::CLAP_OUTPUT));

        // Closed Hat row
        y += rowHeight;
        addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(col1, y)), module, Drum808::CLOSEDHAT_LIGHT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, y)), module, Drum808::CLOSEDHAT_TRIG_INPUT));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col3, y)), module, Drum808::CLOSEDHAT_LEVEL_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col4, y)), module, Drum808::CLOSEDHAT_DECAY_PARAM));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col5, y)), module, Drum808::CLOSEDHAT_OUTPUT));

        // Open Hat row
        y += rowHeight;
        addChild(createLightCentered<SmallLight<BlueLight>>(mm2px(Vec(col1, y)), module, Drum808::OPENHAT_LIGHT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, y)), module, Drum808::OPENHAT_TRIG_INPUT));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col3, y)), module, Drum808::OPENHAT_LEVEL_PARAM));
        addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col4, y)), module, Drum808::OPENHAT_DECAY_PARAM));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col5, y)), module, Drum808::OPENHAT_OUTPUT));

        // Main outputs at bottom
        float outY = 118.0f;
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.0f, outY)), module, Drum808::MAIN_LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(50.0f, outY)), module, Drum808::MAIN_RIGHT_OUTPUT));
    }
};

Model* modelDrum808 = createModel<Drum808, Drum808Widget>("Drum808");
