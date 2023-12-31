#pragma once
#include <JuceHeader.h>
#include "Oscillator.h"


/// FrequencyModulation based synthesizer. Implementation based on the paper
/// "The Simulation of Natural Instrument Tones using Frequency Modulation with a Complex Modulating Wave
/// by  Bill Schottstaedt
class SinusoidSynth {
public:
	SinusoidSynth() = default;
	~SinusoidSynth() = default;

	/// Resets Synth's internal oscillator phases to given value
	void reset(const float startPhase);

	/// Sets the base carrier frequency and updates internal values accordingly
	void setCarrierFrequency(const float carrierFrequency);

	/// Sets the sample rate and updates internal values accordingly
	void setSampleRate(const float sampleRate);

	/// Returns current synthesised value. This function is meant to be run every sample during processing. Automatically updates internal oscillators.
	float getNextValue();

	/// Returns current value of carrier's phase
	float getCarrierPhase() const { return carrierOsc.getPhase(); }

private:
	/// Updates internal variables and oscillators
	void update();
	
	Oscillator carrierOsc;
	Oscillator m1Osc;
	Oscillator m2Osc;

	float I1{0.f};
	float I2{0.f};

	float carrierFrequency{0.f};
	float sampleRate{0.f};
};
