#include "plugin.hpp"

// Generic drum voice that can be randomized
struct DrumVoice {
    enum Type { KICK, SNARE, HAT, TOM, CLAP, RIM, PERC, NOISE };

    Type type = KICK;
    bool active = false;
    float time = 0.0f;
    float velocity = 1.0f;

    // Randomizable parameters
    float baseFreq = 60.0f;
    float pitchDecay = 0.02f;
    float ampDecay = 0.3f;
    float noiseAmount = 0.0f;
    float toneMix = 1.0f;

    // Oscillator state
    float phase = 0.0f;
    float phases[6] = {0.0f};  // For metallic sounds

    // Noise filter
    float filterY1 = 0.0f;
    float filterCoef = 0.5f;

    void randomize() {
        // Randomize parameters based on voice type
        switch (type) {
            case KICK:
                baseFreq = 40.0f + random::uniform() * 40.0f;  // 40-80 Hz
                pitchDecay = 0.01f + random::uniform() * 0.04f;
                ampDecay = 0.15f + random::uniform() * 0.5f;
                noiseAmount = random::uniform() * 0.3f;
                break;
            case SNARE:
                baseFreq = 150.0f + random::uniform() * 100.0f;
                pitchDecay = 0.005f + random::uniform() * 0.02f;
                ampDecay = 0.1f + random::uniform() * 0.2f;
                noiseAmount = 0.4f + random::uniform() * 0.4f;
                break;
            case HAT:
                baseFreq = 300.0f + random::uniform() * 400.0f;
                ampDecay = 0.02f + random::uniform() * 0.15f;
                filterCoef = 0.1f + random::uniform() * 0.3f;
                break;
            case TOM:
                baseFreq = 80.0f + random::uniform() * 200.0f;
                pitchDecay = 0.02f + random::uniform() * 0.05f;
                ampDecay = 0.1f + random::uniform() * 0.3f;
                noiseAmount = random::uniform() * 0.2f;
                break;
            case CLAP:
                baseFreq = 1000.0f + random::uniform() * 2000.0f;
                ampDecay = 0.15f + random::uniform() * 0.2f;
                break;
            case RIM:
                baseFreq = 400.0f + random::uniform() * 400.0f;
                ampDecay = 0.01f + random::uniform() * 0.03f;
                break;
            case PERC:
                baseFreq = 200.0f + random::uniform() * 600.0f;
                pitchDecay = 0.005f + random::uniform() * 0.03f;
                ampDecay = 0.05f + random::uniform() * 0.2f;
                noiseAmount = random::uniform() * 0.5f;
                break;
            case NOISE:
                filterCoef = 0.05f + random::uniform() * 0.4f;
                ampDecay = 0.05f + random::uniform() * 0.3f;
                break;
        }
    }

    void trigger(float vel) {
        active = true;
        velocity = vel;
        time = 0.0f;
        phase = 0.0f;
        for (int i = 0; i < 6; i++) phases[i] = random::uniform();
        filterY1 = 0.0f;
    }

    float process(float level, float character, float sampleRate) {
        if (!active) return 0.0f;

        float output = 0.0f;

        // Scale parameters with character knob
        float effectiveDecay = ampDecay * (0.5f + character);

        switch (type) {
            case KICK: {
                // Sine with pitch envelope
                float pitchEnv = 1.0f + 3.0f * std::exp(-time / pitchDecay);
                float freq = baseFreq * pitchEnv;
                phase += freq / sampleRate;
                if (phase >= 1.0f) phase -= 1.0f;
                float tone = std::sin(2.0f * M_PI * phase);

                // Optional noise click
                float noise = (random::uniform() * 2.0f - 1.0f) * std::exp(-time / 0.005f);

                output = tone * (1.0f - noiseAmount) + noise * noiseAmount;
                break;
            }
            case SNARE: {
                // Tone + noise
                float pitchEnv = 1.0f + std::exp(-time / pitchDecay);
                float freq = baseFreq * pitchEnv;
                phase += freq / sampleRate;
                if (phase >= 1.0f) phase -= 1.0f;
                float tone = std::sin(2.0f * M_PI * phase);

                float noise = random::uniform() * 2.0f - 1.0f;
                // Highpass filter on noise
                filterY1 = filterCoef * noise + (1.0f - filterCoef) * filterY1;
                float hpNoise = noise - filterY1;

                output = tone * (1.0f - noiseAmount) + hpNoise * noiseAmount;
                break;
            }
            case HAT: {
                // Metallic (6 detuned square waves)
                float ratios[6] = {1.0f, 1.47f, 1.73f, 2.15f, 2.67f, 3.14f};
                float mixed = 0.0f;
                for (int i = 0; i < 6; i++) {
                    phases[i] += (baseFreq * ratios[i]) / sampleRate;
                    if (phases[i] >= 1.0f) phases[i] -= 1.0f;
                    float sq = phases[i] < 0.5f ? 1.0f : -1.0f;
                    mixed += sq / 6.0f;
                }
                // Highpass
                filterY1 = filterCoef * mixed + (1.0f - filterCoef) * filterY1;
                output = mixed - filterY1;
                break;
            }
            case TOM: {
                // Sine with pitch drop
                float pitchEnv = 1.0f + 0.5f * std::exp(-time / pitchDecay);
                float freq = baseFreq * pitchEnv;
                phase += freq / sampleRate;
                if (phase >= 1.0f) phase -= 1.0f;
                float tone = std::sin(2.0f * M_PI * phase);
                float noise = (random::uniform() * 2.0f - 1.0f) * std::exp(-time / 0.01f);
                output = tone + noise * noiseAmount;
                break;
            }
            case CLAP: {
                // Filtered noise with multi-attack
                float noise = random::uniform() * 2.0f - 1.0f;
                // Bandpass-ish
                filterY1 = 0.1f * noise + 0.9f * filterY1;
                float bp = noise - filterY1;

                // Multi-spike envelope
                float env = 0.0f;
                if (time < 0.01f) env = std::exp(-time / 0.002f);
                else if (time < 0.02f) env = 0.5f * std::exp(-(time - 0.01f) / 0.002f);
                else if (time < 0.03f) env = 0.25f * std::exp(-(time - 0.02f) / 0.002f);
                else env = std::exp(-(time - 0.03f) / effectiveDecay);

                output = bp * env;
                time += 1.0f / sampleRate;
                active = env > 0.001f;
                return output * velocity * level * 0.8f;
            }
            case RIM: {
                // Short click
                float noise = random::uniform() * 2.0f - 1.0f;
                phase += baseFreq / sampleRate;
                if (phase >= 1.0f) phase -= 1.0f;
                float tone = std::sin(2.0f * M_PI * phase);
                output = (tone * 0.5f + noise * 0.5f);
                break;
            }
            case PERC: {
                // Generic percussion (FM-ish)
                float pitchEnv = 1.0f + std::exp(-time / pitchDecay);
                float freq = baseFreq * pitchEnv;
                phase += freq / sampleRate;
                if (phase >= 1.0f) phase -= 1.0f;
                // FM modulation
                float mod = std::sin(4.0f * M_PI * phase) * std::exp(-time / 0.02f);
                float tone = std::sin(2.0f * M_PI * phase + mod * 2.0f);
                float noise = (random::uniform() * 2.0f - 1.0f);
                output = tone * (1.0f - noiseAmount) + noise * noiseAmount;
                break;
            }
            case NOISE: {
                // Pure filtered noise
                float noise = random::uniform() * 2.0f - 1.0f;
                filterY1 = filterCoef * noise + (1.0f - filterCoef) * filterY1;
                output = noise - filterY1;
                break;
            }
        }

        // Amplitude envelope
        float env = std::exp(-time / effectiveDecay);
        time += 1.0f / sampleRate;

        if (env < 0.001f) {
            active = false;
            return 0.0f;
        }

        return output * env * velocity * level;
    }
};

struct DrumRoulette : Module {
    enum ParamId {
        LEVEL_1_PARAM, LEVEL_2_PARAM, LEVEL_3_PARAM, LEVEL_4_PARAM,
        LEVEL_5_PARAM, LEVEL_6_PARAM, LEVEL_7_PARAM, LEVEL_8_PARAM,
        CHAR_1_PARAM, CHAR_2_PARAM, CHAR_3_PARAM, CHAR_4_PARAM,
        CHAR_5_PARAM, CHAR_6_PARAM, CHAR_7_PARAM, CHAR_8_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        TRIG_1_INPUT, TRIG_2_INPUT, TRIG_3_INPUT, TRIG_4_INPUT,
        TRIG_5_INPUT, TRIG_6_INPUT, TRIG_7_INPUT, TRIG_8_INPUT,
        RAND_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        MAIN_LEFT_OUTPUT,
        MAIN_RIGHT_OUTPUT,
        OUT_1_OUTPUT, OUT_2_OUTPUT, OUT_3_OUTPUT, OUT_4_OUTPUT,
        OUT_5_OUTPUT, OUT_6_OUTPUT, OUT_7_OUTPUT, OUT_8_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        LIGHT_1, LIGHT_2, LIGHT_3, LIGHT_4,
        LIGHT_5, LIGHT_6, LIGHT_7, LIGHT_8,
        RAND_LIGHT,
        LIGHTS_LEN
    };

    DrumVoice voices[8];
    dsp::SchmittTrigger triggers[8];
    dsp::SchmittTrigger randTrigger;
    float lightValues[8] = {0.0f};
    float randLight = 0.0f;

    DrumRoulette() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        const char* names[8] = {"Kick", "Snare", "Closed Hat", "Open Hat",
                                 "Low Tom", "High Tom", "Clap", "Perc"};
        DrumVoice::Type types[8] = {
            DrumVoice::KICK, DrumVoice::SNARE, DrumVoice::HAT, DrumVoice::HAT,
            DrumVoice::TOM, DrumVoice::TOM, DrumVoice::CLAP, DrumVoice::PERC
        };

        for (int i = 0; i < 8; i++) {
            voices[i].type = types[i];
            voices[i].randomize();

            configParam(LEVEL_1_PARAM + i, 0.0f, 1.0f, 0.75f,
                        std::string(names[i]) + " Level", "%", 0.0f, 100.0f);
            configParam(CHAR_1_PARAM + i, 0.0f, 1.0f, 0.5f,
                        std::string(names[i]) + " Character", "%", 0.0f, 100.0f);
            configInput(TRIG_1_INPUT + i, std::string(names[i]) + " Trigger");
            configOutput(OUT_1_OUTPUT + i, std::string(names[i]) + " Output");
        }

        // Open hat gets longer decay
        voices[3].ampDecay = 0.2f;

        configInput(RAND_INPUT, "Randomize All Trigger");
        configOutput(MAIN_LEFT_OUTPUT, "Main Left");
        configOutput(MAIN_RIGHT_OUTPUT, "Main Right");
    }

    void process(const ProcessArgs& args) override {
        // Check randomize trigger
        if (randTrigger.process(inputs[RAND_INPUT].getVoltage(), 0.1f, 2.0f)) {
            for (int i = 0; i < 8; i++) {
                voices[i].randomize();
            }
            randLight = 1.0f;
        }

        float mix = 0.0f;

        for (int i = 0; i < 8; i++) {
            // Check voice triggers
            if (triggers[i].process(inputs[TRIG_1_INPUT + i].getVoltage(), 0.1f, 2.0f)) {
                // Closed hat (2) chokes open hat (3)
                if (i == 2 && voices[3].active) {
                    voices[3].active = false;
                }
                voices[i].trigger(1.0f);
                lightValues[i] = 1.0f;
            }

            // Process voice
            float level = params[LEVEL_1_PARAM + i].getValue();
            float character = params[CHAR_1_PARAM + i].getValue();
            float out = voices[i].process(level, character, args.sampleRate);

            // Individual output
            outputs[OUT_1_OUTPUT + i].setVoltage(out * 5.0f);

            mix += out;
        }

        // Soft clip mix
        mix = std::tanh(mix);

        // Main outputs
        outputs[MAIN_LEFT_OUTPUT].setVoltage(mix * 5.0f);
        outputs[MAIN_RIGHT_OUTPUT].setVoltage(mix * 5.0f);

        // Update lights
        for (int i = 0; i < 8; i++) {
            lightValues[i] *= 0.999f;
            lights[LIGHT_1 + i].setBrightness(lightValues[i]);
        }
        randLight *= 0.99f;
        lights[RAND_LIGHT].setBrightness(randLight);
    }
};

struct DrumRouletteWidget : ModuleWidget {
    DrumRouletteWidget(DrumRoulette* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/DrumRoulette.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(0, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

        // 20HP = 101.6mm
        // 8 rows for voices plus randomize section

        float col1 = 10.0f;   // Light
        float col2 = 22.0f;   // Trigger
        float col3 = 38.0f;   // Level knob
        float col4 = 54.0f;   // Character knob
        float col5 = 70.0f;   // Individual output

        float rowHeight = 11.0f;
        float startY = 24.0f;

        // 8 voice rows
        for (int i = 0; i < 8; i++) {
            float y = startY + i * rowHeight;
            addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(col1, y)), module, DrumRoulette::LIGHT_1 + i));
            addInput(createInputCentered<PJ301MPort>(mm2px(Vec(col2, y)), module, DrumRoulette::TRIG_1_INPUT + i));
            addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col3, y)), module, DrumRoulette::LEVEL_1_PARAM + i));
            addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(col4, y)), module, DrumRoulette::CHAR_1_PARAM + i));
            addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(col5, y)), module, DrumRoulette::OUT_1_OUTPUT + i));
        }

        // Randomize section
        float randY = 115.0f;
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(15.0f, randY)), module, DrumRoulette::RAND_LIGHT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.0f, randY)), module, DrumRoulette::RAND_INPUT));

        // Main outputs
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(55.0f, randY)), module, DrumRoulette::MAIN_LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(70.0f, randY)), module, DrumRoulette::MAIN_RIGHT_OUTPUT));
    }
};

Model* modelDrumRoulette = createModel<DrumRoulette, DrumRouletteWidget>("DrumRoulette");
