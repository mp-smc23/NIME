/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FM/SinusoidSynth.h"

//==============================================================================
/**
*/
class SimpleSynthAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SimpleSynthAudioProcessor();
    ~SimpleSynthAudioProcessor() override;

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

    struct HarmonyRatio {
        float numerator;
        float denominator;
    };

	juce::AudioParameterFloat* pitch;
	juce::AudioParameterFloat* gain;

    juce::AudioParameterBool* fifth;
    juce::AudioParameterBool* fourth;
    juce::AudioParameterBool* third;
    juce::AudioParameterBool* thirdMinor;
    juce::AudioParameterBool* octave;

	std::unique_ptr<SinusoidSynth> mainSynth;
	std::unique_ptr<SinusoidSynth> fifthSynth;
	std::unique_ptr<SinusoidSynth> fourthSynth;
	std::unique_ptr<SinusoidSynth> thirdSynth;
	std::unique_ptr<SinusoidSynth> thirdMinorSynth;
	std::unique_ptr<SinusoidSynth> octaveSynth;

	float curPitch{0.f};
	bool isFifthOn{false};
	bool isFourthOn{false};
	bool isThirdOn{false};
	bool isThirdMinorOn{false};
	bool isOctaveOn{false};

	HarmonyRatio fifthRatio{3.f, 2.f};
	HarmonyRatio fourthRatio{4.f, 3.f};
	HarmonyRatio thirdRatio{5.f, 4.f};
	HarmonyRatio thirdMinorRatio{6.f, 5.f};
	HarmonyRatio octaveRatio{2.f, 1.f};

	juce::ADSR adsr;
	int adsrResetCounter {0};

	void prepareSideSynth(const std::unique_ptr<SinusoidSynth>& synth, const HarmonyRatio& ratio, bool& state, const bool isOn);
    float calculateHarmonyFrequency(const float baseFrequency, const HarmonyRatio& ratio) const;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleSynthAudioProcessor)
};
