#pragma once
#include "Oscillator.h"
#include <cmath>

/// @brief FrequencyModulation based synthesizer. Implementation based on the paper
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

	/// @brief Resets Synth's internal oscillator phases to given value
	/// @param startPhase Internal oscillator phases will be set to this value 
	void reset(const float startPhase);

	/// @brief Sets the base carrier frequency and updates internal values accordingly
	/// @param carrierFrequency The new carrier frequency
	void setCarrierFrequency(const float carrierFrequency);

	/// @brief Sets the sample rate and updates internal values accordingly
	/// @param sampleRate The new sample rete
	void setSampleRate(const float sampleRate);

	/// @brief Returns current synthesised value. This function is meant to be run every sample during processing. Automatically updates internal oscillators.
	/// @return Next sample
	float getNextValue();

	/// @brief Returns current value of carrier's phase
	float getCarrierPhase() const { return carrierOsc.getPhase(); }

	/// @brief Tells synth that it is in attack phase and should scale the output for x miliseconds according to internal envolpe
	/// @param miliseconds The length of the attack phase
	void startAttackPhase(const float miliseconds = 10);

	/// @brief Tells synth that it is in decay phase and should scale the output for x miliseconds according to internal envolpe
	/// @param miliseconds The length of the decay phase
	void startDecayPhase(const float miliseconds = 10);

private:
	/// Updates internal variables and oscillators
	void update();
	static float calculateHarmonyFrequency(const float baseFrequency, const HarmonyRatio& ratio);

	HarmonyRatio harmonyRatio;

	Oscillator carrierOsc;
	Oscillator m1Osc;
	Oscillator m2Osc;

	float I1{0.f};
	float I2{0.f};

	float carrierFrequency{0.f};
	float sampleRate{0.f};

	float envelope{1.f};
	float envelopeStep{0.f};

	const float twoPi = 2.f * 3.14159265358979323846f;
};
