#pragma once

/// @brief Simple oscillator class implementation
class Oscillator {
public:
	Oscillator() = default;
	~Oscillator() = default;

	void setPhase(const float newPhase) { phase = newPhase; }
	void setStep(const float newStep) { step = newStep; }

	float getPhase() const { return phase; }

	/// @brief Updates the internal phase of the oscillator and returns it.
	/// @return Current phase of the oscillator
	float getNextPhaseValue(){
		phase += step; // for each sample increment the phase

		if(phase > 1.f){ // modulo; always keep the values between 0 and 1
			phase -= 1.f;
		}

		return phase;
	}

private:
	float phase{0.f};
	float step{0.f};
};
