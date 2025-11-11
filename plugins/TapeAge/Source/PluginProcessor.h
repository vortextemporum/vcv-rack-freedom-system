#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/**
 * TapeAge - Vintage Tape Saturator
 *
 * A warm tape saturation plugin with musical degradation effects including
 * wow, flutter, dropout, and tape noise.
 */
class TapeAgeAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    TapeAgeAudioProcessor();
    ~TapeAgeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    //==============================================================================
    // Parameter layout creation
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    // DSP Components (declared BEFORE parameters for initialization order)

    // Oversampling for aliasing reduction (2x, polyphase IIR)
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    // Dry buffer for mix stage (Phase 4.4)
    juce::AudioBuffer<float> dryBuffer;

    // Phase 4.2: Modulation System (Wow & Flutter)
    // Delay buffer for pitch modulation (10ms capacity)
    juce::AudioBuffer<float> delayBuffer;
    int delayBufferLength = 0;
    int delayWritePosition = 0;

    // LFO state for wow and flutter
    double wowPhase = 0.0;       // Wow LFO phase accumulator
    double flutterPhase = 0.0;   // Flutter LFO phase accumulator
    double wowFrequency = 0.3;   // Wow frequency in Hz (< 1 Hz)
    double flutterFrequency = 7.0; // Flutter frequency in Hz (5-10 Hz)

    // Phase 4.3: Degradation Effects (Dropout & Noise)
    // Dropout state machine
    bool dropoutActive = false;           // Currently in dropout event
    int dropoutSamplesRemaining = 0;      // Samples left in current dropout
    float dropoutGain = 1.0f;             // Current dropout gain (0.0 to 1.0)
    float dropoutTargetGain = 1.0f;       // Target gain for envelope
    juce::Random dropoutRandom;           // RNG for dropout events

    // Dropout envelope smoothing (10ms attack, 30ms release)
    juce::SmoothedValue<float> dropoutEnvelope;

    // Pink noise generator (Paul Kellet's method)
    float pinkNoise_b0 = 0.0f;
    float pinkNoise_b1 = 0.0f;
    float pinkNoise_b2 = 0.0f;
    float pinkNoise_b3 = 0.0f;
    float pinkNoise_b4 = 0.0f;
    float pinkNoise_b5 = 0.0f;
    float pinkNoise_b6 = 0.0f;
    juce::Random noiseRandom;             // RNG for noise generation

    // Audio parameter value tree state
    juce::AudioProcessorValueTreeState parameters;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeAgeAudioProcessor)
};
