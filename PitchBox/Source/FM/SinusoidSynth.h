#pragma once
#include "Oscillator.h"
#include <cmath>
#include "daisysp.h"

/// FrequencyModulation based synthesizer. Implementation based on the paper
/// "The Simulation of Natural Instrument Tones using Frequency Modulation with a Complex Modulating Wave
/// by  Bill Schottstaedt
class SinusoidSynth {
public:
	struct HarmonyRatio {
		float numerator;
		float denominator;
	};

	SinusoidSynth(HarmonyRatio ratio = {1.f, 1.f}) : harmonyRatio(ratio) {};
	~SinusoidSynth() = default;

	/// Resets Synth's internal oscillator phases to given value
	void init(const float sampleRate);

	/// Sets the base carrier frequency and updates internal values accordingly
	void setCarrierFrequency(const float carrierFrequency);

	/// Sets the sample rate and updates internal values accordingly
	void setSampleRate(const float sampleRate);

	/// Returns current synthesised value. This function is meant to be run every sample during processing. Automatically updates internal oscillators.
	float getNextValue();

	/// Tells synth that it is in attack phase and should scale the output for x miliseconds according to internal envolpe
	void startAttackPhase(const float miliseconds = 10);

	/// Tells synth that it is in decay phase and should scale the output for x miliseconds according to internal envolpe
	void startDecayPhase(const float miliseconds = 10);

private:
	/// Updates internal variables and oscillators
	void update();
	static float calculateHarmonyFrequency(const float baseFrequency, const HarmonyRatio& ratio);

	HarmonyRatio harmonyRatio;
	daisysp::StringVoice str;

	float carrierFrequency{0.f};
	float sampleRate{0.f};

	float envelope{1.f};
	float envelopeStep{0.f};

	const float twoPi = 2.f * 3.14159265358979323846f;
};
