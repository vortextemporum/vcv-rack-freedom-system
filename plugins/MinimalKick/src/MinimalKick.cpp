#include "plugin.hpp"

struct MinimalKick : Module {
    enum ParamId {
        SWEEP_PARAM,
        TIME_PARAM,
        ATTACK_PARAM,
        DECAY_PARAM,
        DRIVE_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        TRIGGER_INPUT,
        VOCT_INPUT,
        INPUTS_LEN
    };
    enum OutputId {
        AUDIO_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        TRIGGER_LIGHT,
        LIGHTS_LEN
    };

    // Oscillator state
    float phase = 0.f;
    float currentFrequency = 60.f;  // Default base frequency (Hz)

    // Pitch envelope state
    float pitchEnvelopeValue = 0.f;
    float pitchEnvelopeTime = 0.f;  // Time since trigger in seconds

    // Amplitude envelope state (simple AD envelope)
    float ampEnvelopeValue = 0.f;
    float ampEnvelopeTime = 0.f;
    bool inAttackPhase = false;
    bool envelopeActive = false;

    // Trigger detection
    dsp::SchmittTrigger trigger;
    dsp::PulseGenerator triggerLight;

    MinimalKick() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        // Sweep: 0-24 semitones (pitch envelope amount)
        configParam(SWEEP_PARAM, 0.f, 24.f, 12.f, "Sweep", " st");

        // Time: 5-500ms (pitch envelope decay time)
        configParam(TIME_PARAM, 5.f, 500.f, 50.f, "Time", " ms");

        // Attack: 0-50ms (amplitude envelope attack)
        configParam(ATTACK_PARAM, 0.f, 50.f, 5.f, "Attack", " ms");

        // Decay: 50-2000ms (amplitude envelope decay)
        configParam(DECAY_PARAM, 50.f, 2000.f, 400.f, "Decay", " ms");

        // Drive: 0-100% (saturation)
        configParam(DRIVE_PARAM, 0.f, 100.f, 20.f, "Drive", "%");

        configInput(TRIGGER_INPUT, "Trigger");
        configInput(VOCT_INPUT, "V/Oct pitch");
        configOutput(AUDIO_OUTPUT, "Audio");
    }

    void process(const ProcessArgs& args) override {
        // Check for trigger
        if (trigger.process(inputs[TRIGGER_INPUT].getVoltage(), 0.1f, 1.f)) {
            // Reset envelopes on trigger
            pitchEnvelopeValue = 1.f;
            pitchEnvelopeTime = 0.f;
            ampEnvelopeValue = 0.f;
            ampEnvelopeTime = 0.f;
            inAttackPhase = true;
            envelopeActive = true;
            phase = 0.f;  // Reset oscillator phase for consistent attack

            // Calculate base frequency from V/Oct input
            float pitch = 0.f;  // 0V = C4
            if (inputs[VOCT_INPUT].isConnected()) {
                pitch = inputs[VOCT_INPUT].getVoltage();
            }
            // Default to ~60Hz (bass) which is roughly B1
            // Shift down 3 octaves from C4 (261.63Hz / 8 ≈ 32.7Hz) + small adjustment
            currentFrequency = dsp::FREQ_C4 * std::pow(2.f, pitch - 2.1f);

            triggerLight.trigger(0.1f);
        }

        // Read parameters
        float sweepSemitones = params[SWEEP_PARAM].getValue();
        float pitchDecayMs = params[TIME_PARAM].getValue();
        float attackMs = params[ATTACK_PARAM].getValue();
        float decayMs = params[DECAY_PARAM].getValue();
        float drivePercent = params[DRIVE_PARAM].getValue();

        float output = 0.f;

        if (envelopeActive) {
            // Update pitch envelope (exponential decay)
            float pitchDecaySeconds = pitchDecayMs / 1000.f;
            float pitchDecayRate = -std::log(0.001f) / pitchDecaySeconds;
            pitchEnvelopeValue = std::exp(-pitchDecayRate * pitchEnvelopeTime);
            pitchEnvelopeTime += args.sampleTime;

            // Calculate modulated frequency
            float pitchOffsetSemitones = pitchEnvelopeValue * sweepSemitones;
            float frequencyMultiplier = std::pow(2.f, pitchOffsetSemitones / 12.f);
            float modulatedFrequency = currentFrequency * frequencyMultiplier;

            // Update oscillator phase
            phase += modulatedFrequency * args.sampleTime;
            if (phase >= 1.f) phase -= 1.f;

            // Generate sine wave
            float oscillatorSample = std::sin(2.f * M_PI * phase);

            // Update amplitude envelope (AD envelope)
            float attackSeconds = attackMs / 1000.f;
            float decaySeconds = decayMs / 1000.f;

            if (inAttackPhase) {
                if (attackSeconds > 0.f) {
                    ampEnvelopeValue += args.sampleTime / attackSeconds;
                    if (ampEnvelopeValue >= 1.f) {
                        ampEnvelopeValue = 1.f;
                        inAttackPhase = false;
                    }
                } else {
                    ampEnvelopeValue = 1.f;
                    inAttackPhase = false;
                }
            } else {
                // Decay phase (exponential decay)
                float decayRate = -std::log(0.001f) / decaySeconds;
                ampEnvelopeValue *= std::exp(-decayRate * args.sampleTime);

                // Stop envelope when below threshold
                if (ampEnvelopeValue < 0.0001f) {
                    ampEnvelopeValue = 0.f;
                    envelopeActive = false;
                }
            }

            // Apply amplitude envelope
            float envelopedSample = oscillatorSample * ampEnvelopeValue;

            // Apply saturation/drive (tanh waveshaping)
            float driveNormalized = drivePercent / 100.f;
            float gain = 1.f + (driveNormalized * 9.f);  // 1.0 to 10.0
            output = std::tanh(gain * envelopedSample);
        }

        // Output (±5V for audio)
        outputs[AUDIO_OUTPUT].setVoltage(output * 5.f);

        // Trigger light
        lights[TRIGGER_LIGHT].setBrightness(triggerLight.process(args.sampleTime));
    }
};

struct MinimalKickWidget : ModuleWidget {
    MinimalKickWidget(MinimalKick* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/MinimalKick.svg")));

        // Screws
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        // Knobs (6HP = 30.48mm, center at 15.24mm)
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(8, 28)), module, MinimalKick::SWEEP_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.48, 28)), module, MinimalKick::TIME_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(8, 52)), module, MinimalKick::ATTACK_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.48, 52)), module, MinimalKick::DECAY_PARAM));
        addParam(createParamCentered<RoundBigBlackKnob>(mm2px(Vec(15.24, 76)), module, MinimalKick::DRIVE_PARAM));

        // Inputs
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 100)), module, MinimalKick::TRIGGER_INPUT));
        addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.48, 100)), module, MinimalKick::VOCT_INPUT));

        // Output
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(15.24, 115)), module, MinimalKick::AUDIO_OUTPUT));

        // Trigger light
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(15.24, 95)), module, MinimalKick::TRIGGER_LIGHT));
    }
};

Model* modelMinimalKick = createModel<MinimalKick, MinimalKickWidget>("MinimalKick");
