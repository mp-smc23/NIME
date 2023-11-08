#include "SinusoidSynth.h"

void SinusoidSynth::reset(const float startPhase){
	carrierOsc.setPhase(startPhase);
	m1Osc.setPhase(startPhase);
	m2Osc.setPhase(startPhase);
}

void SinusoidSynth::setCarrierFrequency(const float carrierFrequency){
	const auto newFrequency =  calculateHarmonyFrequency(carrierFrequency, harmonyRatio);
	if(abs(this->carrierFrequency - newFrequency) < 0.0001f) return;

	this->carrierFrequency = newFrequency;
	update();
}

void SinusoidSynth::setSampleRate(const float sampleRate){
	if(abs(this->sampleRate - sampleRate) < 0.0001f) return;

	this->sampleRate = sampleRate;
	update();
}

float SinusoidSynth::getNextValue(){
	// e = A(t)sin[2*pi*fc*t + I1 * sin(2*pi*(fm1+S)*t) + I2 * sin(2*pi*(fm2+S)t)]
	const auto sinM1 = sin(twoPi * m1Osc.getNextPhaseValue());
	const auto sinM2 = sin(twoPi * m2Osc.getNextPhaseValue());
	return sin(twoPi * carrierOsc.getNextPhaseValue() + I1 * sinM1 + I2 * sinM2);
}

void SinusoidSynth::update(){
	const auto lnfc = log(carrierFrequency);

	const auto S = carrierFrequency * 0.005;					// S = fc / 200;
	I1 = 17 * (8 - lnfc) / (lnfc * lnfc);						// I1 = 17*(8-ln(fc)) / (ln(fc))^2;
	I2 = 20 * (8 - lnfc) / carrierFrequency; 					// I2 = 20*(8-ln(fc)) / fc;

	carrierOsc.setStep(carrierFrequency / sampleRate);			// fc:fm1:fm2 == 1:1:4
	m1Osc.setStep((carrierFrequency + S) / sampleRate);			// fm1 + S
	m2Osc.setStep((carrierFrequency * 4.f + S) / sampleRate);	// fm2 + S
}

float SinusoidSynth::calculateHarmonyFrequency(const float baseFrequency, const HarmonyRatio& ratio){
	return baseFrequency * ratio.numerator / ratio.denominator;
}