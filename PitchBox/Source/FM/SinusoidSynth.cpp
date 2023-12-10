#include "SinusoidSynth.h"


void SinusoidSynth::init(const float newSampleRate){
	str.Init(newSampleRate);
	setSampleRate(newSampleRate);
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
	envelope += envelopeStep;
	envelope = daisysp::fclamp(envelope, 0.f, 1.f);
	
	return str.Process() * envelope;
}

void SinusoidSynth::update(){
	str.SetFreq(carrierFrequency);

    str.SetAccent(0.1f);
	str.SetDamping(0.1f);
	str.SetStructure(0.1f);
	str.SetBrightness(0.5);
	
	str.SetSustain(true);
}

float SinusoidSynth::calculateHarmonyFrequency(const float baseFrequency, const HarmonyRatio& ratio){
	return baseFrequency * ratio.numerator / ratio.denominator;
}

void SinusoidSynth::startAttackPhase(const float miliseconds){
	envelopeStep = 1 / (miliseconds * 0.001 * sampleRate);
}

void SinusoidSynth::startDecayPhase(const float miliseconds){
	envelopeStep = -1 / (miliseconds * 0.001 * sampleRate);
}