#include "plugin.hpp"

// Simple ADSR envelope
struct ADSREnvelope {
    enum Stage { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };
    Stage stage = IDLE;
    float value = 0.0f;
    float attack = 0.3f, decay = 0.2f, sustain = 0.8f, release = 2.0f;

    void noteOn() {
        stage = ATTACK;
    }

    void noteOff() {
        if (stage != IDLE) stage = RELEASE;
    }

    bool isActive() const {
        return stage != IDLE;
    }

    float process(float sampleRate) {
        float rate;
        switch (stage) {
            case ATTACK:
                rate = 1.0f / (attack * sampleRate);
                value += rate;
                if (value >= 1.0f) {
                    value = 1.0f;
                    stage = DECAY;
                }
                break;
            case DECAY:
                rate = 1.0f / (decay * sampleRate);
                value -= rate * (1.0f - sustain);
                if (value <= sustain) {
                    value = sustain;
                    stage = SUSTAIN;
                }
                break;
            case SUSTAIN:
                value = sustain;
                break;
            case RELEASE:
                rate = 1.0f / (release * sampleRate);
                value -= rate;
                if (value <= 0.0f) {
                    value = 0.0f;
                    stage = IDLE;
                }
                break;
            default:
                value = 0.0f;
                break;
        }
        return value;
    }

    void reset() {
        stage = IDLE;
        value = 0.0f;
    }
};

// Simple one-pole lowpass filter
struct OnePoleLP {
    float y1 = 0.0f;

    float process(float x, float cutoff, float sampleRate) {
        float w = 2.0f * M_PI * cutoff / sampleRate;
        float coef = 1.0f - std::exp(-w);
        y1 += coef * (x - y1);
        return y1;
    }

    void reset() { y1 = 0.0f; }
};

// Simple allpass for reverb
struct AllpassDelay {
    static const int MAX_SIZE = 8192;
    float buffer[MAX_SIZE] = {0.0f};
    int writePos = 0;
    int delaySamples = 1000;
    float feedback = 0.5f;

    void setParams(int delay, float fb) {
        delaySamples = clamp(delay, 1, MAX_SIZE - 1);
        feedback = fb;
    }

    float process(float input) {
        int readPos = writePos - delaySamples;
        if (readPos < 0) readPos += MAX_SIZE;

        float delayed = buffer[readPos];
        float output = -input + delayed;
        buffer[writePos] = input + delayed * feedback;

        writePos = (writePos + 1) % MAX_SIZE;
        return output;
    }

    void clear() {
        memset(buffer, 0, sizeof(buffer));
    }
};

// Synth voice
struct PadVoice {
    bool active = false;
    float phase1 = 0.0f, phase2 = 0.0f, phase3 = 0.0f;
    float prevOut1 = 0.0f, prevOut2 = 0.0f, prevOut3 = 0.0f;
    float frequency = 440.0f;
    float velocity = 1.0f;
    ADSREnvelope env;
    OnePoleLP filter;

    // LFO for modulation
    float lfoPhase = 0.0f;
    float lfoFreq = 0.1f;

    void trigger(float freq, float vel) {
        active = true;
        frequency = freq;
        velocity = vel;
        phase1 = phase2 = phase3 = 0.0f;
        prevOut1 = prevOut2 = prevOut3 = 0.0f;
        lfoPhase = random::uniform() * 2.0f * M_PI;
        lfoFreq = 0.05f + random::uniform() * 0.15f;
        env.noteOn();
    }

    void release() {
        env.noteOff();
    }

    void process(float& outL, float& outR, float timbre, float filterCutoff, float sampleRate) {
        if (!active) {
            outL = outR = 0.0f;
            return;
        }

        // Update LFO
        lfoPhase += (lfoFreq * 2.0f * M_PI) / sampleRate;
        if (lfoPhase >= 2.0f * M_PI) lfoPhase -= 2.0f * M_PI;
        float lfoVal = std::sin(lfoPhase);

        // FM feedback depth modulated by timbre and LFO
        float fbDepth = timbre * 0.4f * (1.0f + lfoVal * 0.2f);

        // Detuning ratios (±7 cents)
        float ratio1 = 1.0f;
        float ratio2 = 1.00407f;  // +7 cents
        float ratio3 = 0.99593f;  // -7 cents

        // Generate 3 detuned FM oscillators
        float osc1 = std::sin(phase1 + fbDepth * prevOut1);
        float osc2 = std::sin(phase2 + fbDepth * prevOut2);
        float osc3 = std::sin(phase3 + fbDepth * prevOut3);

        prevOut1 = osc1;
        prevOut2 = osc2;
        prevOut3 = osc3;

        // Mix oscillators
        float mix = (osc1 + osc2 + osc3) / 3.0f;

        // Saturation modulated by timbre
        float satGain = 1.0f + timbre * 2.0f;
        mix = std::tanh(mix * satGain);

        // Filter with velocity-scaled cutoff
        float velCutoff = filterCutoff * (0.5f + 0.5f * velocity);
        mix = filter.process(mix, velCutoff, sampleRate);

        // Envelope
        float envVal = env.process(sampleRate);
        mix *= envVal * velocity;

        // Pan based on LFO
        float pan = 0.5f + lfoVal * 0.3f;
        outL = mix * (1.0f - pan);
        outR = mix * pan;

        // Advance phases
        float inc1 = (frequency * ratio1 * 2.0f * M_PI) / sampleRate;
        float inc2 = (frequency * ratio2 * 2.0f * M_PI) / sampleRate;
        float inc3 = (frequency * ratio3 * 2.0f * M_PI) / sampleRate;

        phase1 += inc1;
        phase2 += inc2;
        phase3 += inc3;

        if (phase1 >= 2.0f * M_PI) phase1 -= 2.0f * M_PI;
        if (phase2 >= 2.0f * M_PI) phase2 -= 2.0f * M_PI;
        if (phase3 >= 2.0f * M_PI) phase3 -= 2.0f * M_PI;

        // Check if voice finished
        if (!env.isActive()) {
            active = false;
        }
    }
};

struct LushPad : Module {
    enum ParamId {
        TIMBRE_PARAM,
        FILTER_PARAM,
        REVERB_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        VOCT_INPUT,
        GATE_INPUT,
        TIMBRE_CV_INPUT,
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

    static const int NUM_VOICES = 8;
    PadVoice voices[NUM_VOICES];
    int nextVoice = 0;

    // Gate tracking for polyphonic input
    dsp::SchmittTrigger gateTrigger[16];
    bool gateHigh[16] = {false};

    // Simple reverb (4 allpass delays)
    AllpassDelay allpass1L, allpass2L, allpass1R, allpass2R;

    LushPad() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(TIMBRE_PARAM, 0.0f, 1.0f, 0.35f, "Timbre", "%", 0.0f, 100.0f);
        configParam(FILTER_PARAM, 20.0f, 20000.0f, 2000.0f, "Filter Cutoff", " Hz");
        configParam(REVERB_PARAM, 0.0f, 1.0f, 0.4f, "Reverb", "%", 0.0f, 100.0f);

        configInput(VOCT_INPUT, "V/Oct");
        configInput(GATE_INPUT, "Gate");
        configInput(TIMBRE_CV_INPUT, "Timbre CV");
        configInput(FILTER_CV_INPUT, "Filter CV");

        configOutput(LEFT_OUTPUT, "Left");
        configOutput(RIGHT_OUTPUT, "Right");

        // Initialize reverb delays
        allpass1L.setParams(1051, 0.7f);
        allpass2L.setParams(337, 0.7f);
        allpass1R.setParams(1117, 0.7f);
        allpass2R.setParams(379, 0.7f);
    }

    void onReset() override {
        for (auto& voice : voices) {
            voice.active = false;
            voice.env.reset();
            voice.filter.reset();
        }
        allpass1L.clear();
        allpass2L.clear();
        allpass1R.clear();
        allpass2R.clear();
    }

    int findFreeVoice() {
        // First try to find inactive voice
        for (int i = 0; i < NUM_VOICES; i++) {
            if (!voices[i].active) return i;
        }
        // Steal oldest (round-robin)
        int idx = nextVoice;
        nextVoice = (nextVoice + 1) % NUM_VOICES;
        return idx;
    }

    void process(const ProcessArgs& args) override {
        float sampleRate = args.sampleRate;

        // Get parameters
        float timbre = params[TIMBRE_PARAM].getValue();
        float filterCutoff = params[FILTER_PARAM].getValue();
        float reverbMix = params[REVERB_PARAM].getValue();

        // CV modulation
        if (inputs[TIMBRE_CV_INPUT].isConnected()) {
            timbre += inputs[TIMBRE_CV_INPUT].getVoltage() * 0.1f;
            timbre = clamp(timbre, 0.0f, 1.0f);
        }
        if (inputs[FILTER_CV_INPUT].isConnected()) {
            // Exponential CV for filter
            filterCutoff *= std::pow(2.0f, inputs[FILTER_CV_INPUT].getVoltage());
            filterCutoff = clamp(filterCutoff, 20.0f, 20000.0f);
        }

        // Process polyphonic gates
        int numChannels = std::max(1, inputs[GATE_INPUT].getChannels());
        for (int c = 0; c < numChannels; c++) {
            float gate = inputs[GATE_INPUT].getVoltage(c);
            bool high = gate >= 1.0f;

            if (high && !gateHigh[c]) {
                // Note on
                float voct = inputs[VOCT_INPUT].getVoltage(c);
                float freq = 261.626f * std::pow(2.0f, voct);  // C4 base
                int v = findFreeVoice();
                voices[v].trigger(freq, 0.8f);
            } else if (!high && gateHigh[c]) {
                // Note off - find voice playing this frequency
                float voct = inputs[VOCT_INPUT].getVoltage(c);
                float freq = 261.626f * std::pow(2.0f, voct);
                for (auto& voice : voices) {
                    if (voice.active && std::abs(voice.frequency - freq) < 1.0f) {
                        voice.release();
                        break;
                    }
                }
            }
            gateHigh[c] = high;
        }

        // Process all voices
        float mixL = 0.0f, mixR = 0.0f;
        for (auto& voice : voices) {
            float vL, vR;
            voice.process(vL, vR, timbre, filterCutoff, sampleRate);
            mixL += vL;
            mixR += vR;
        }

        // Scale down
        mixL *= 0.25f;
        mixR *= 0.25f;

        // Simple reverb
        float dryL = mixL, dryR = mixR;
        float wetL = allpass1L.process(mixL);
        wetL = allpass2L.process(wetL);
        float wetR = allpass1R.process(mixR);
        wetR = allpass2R.process(wetR);

        // Mix dry/wet
        float outL = dryL * (1.0f - reverbMix) + wetL * reverbMix;
        float outR = dryR * (1.0f - reverbMix) + wetR * reverbMix;

        outputs[LEFT_OUTPUT].setVoltage(outL * 5.0f);
        outputs[RIGHT_OUTPUT].setVoltage(outR * 5.0f);
    }
};

struct LushPadWidget : ModuleWidget {
    LushPadWidget(LushPad* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/LushPad.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(0, 365)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15, 365)));

        // 8HP = 40.64mm
        float centerX = 20.32f;

        // Knobs
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(centerX, 28.0f)), module, LushPad::TIMBRE_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(centerX, 50.0f)), module, LushPad::FILTER_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(centerX, 72.0f)), module, LushPad::REVERB_PARAM));

        // CV inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0f, 92.0f)), module, LushPad::TIMBRE_CV_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.0f, 92.0f)), module, LushPad::FILTER_CV_INPUT));

        // Main inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(10.0f, 108.0f)), module, LushPad::VOCT_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(30.0f, 108.0f)), module, LushPad::GATE_INPUT));

        // Outputs
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.0f, 120.0f)), module, LushPad::LEFT_OUTPUT));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(30.0f, 120.0f)), module, LushPad::RIGHT_OUTPUT));
    }
};

Model* modelLushPad = createModel<LushPad, LushPadWidget>("LushPad");
