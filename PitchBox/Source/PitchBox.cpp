#include "daisy_seed.h"
#include "daisysp.h"
#include "FM/SinusoidSynth.h"
#include <memory>

using namespace daisy;
using namespace daisysp;

struct HarmonyRatio {
    float numerator;
    float denominator;
};

HarmonyRatio fifthRatio{3.f, 2.f};
HarmonyRatio fourthRatio{4.f, 3.f};
HarmonyRatio thirdRatio{5.f, 4.f};
HarmonyRatio thirdMinorRatio{6.f, 5.f};
HarmonyRatio octaveRatio{2.f, 1.f};

std::unique_ptr<SinusoidSynth> mainSynth;
std::unique_ptr<SinusoidSynth> fifthSynth;
std::unique_ptr<SinusoidSynth> fourthSynth;
std::unique_ptr<SinusoidSynth> thirdSynth;
std::unique_ptr<SinusoidSynth> thirdMinorSynth;
std::unique_ptr<SinusoidSynth> octaveSynth;

bool isFifthOn{false};
bool isFourthOn{false};
bool isThirdOn{false};
bool isThirdMinorOn{false};
bool isOctaveOn{false};

float sampleRate;
float curPitch;

DaisySeed hw;

void init(){
    mainSynth = std::make_unique<SinusoidSynth>();
	fifthSynth = std::make_unique<SinusoidSynth>();
	fourthSynth = std::make_unique<SinusoidSynth>();
	thirdSynth = std::make_unique<SinusoidSynth>();
	thirdMinorSynth = std::make_unique<SinusoidSynth>();
	octaveSynth = std::make_unique<SinusoidSynth>();
}

float calculateHarmonyFrequency(const float baseFrequency, const HarmonyRatio& ratio){
	return baseFrequency * ratio.numerator / ratio.denominator;
}

void prepareSideSynth(const std::unique_ptr<SinusoidSynth>& synth, const HarmonyRatio& ratio, bool& prevState, const bool newState){
	// if the synth was just turned on/off reset it
	if(prevState != newState){
		synth->reset(mainSynth->getCarrierPhase());
		prevState = newState;
	}

	if(!newState) return; // synth is turned off, nothing to do

	// if the synth is turned on, update pitch and sample rate values
	synth->setCarrierFrequency(calculateHarmonyFrequency(curPitch, ratio));
	synth->setSampleRate(sampleRate);
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{

    curPitch = 440.f; // TODO pitch->get(); // get current value of pitch parameter

	mainSynth->setCarrierFrequency(curPitch); 	// update pitch of the main synth
	mainSynth->setSampleRate(sampleRate);		// update sample rate of the main synth

	// prapare all side synths
	prepareSideSynth(fifthSynth, fifthRatio, isFifthOn, false); //TODO replace false with button input
	prepareSideSynth(fourthSynth, fourthRatio, isFourthOn, false); //TODO replace false with button input
	prepareSideSynth(thirdSynth, thirdRatio, isThirdOn, false);  //TODO replace false with button input
	prepareSideSynth(thirdMinorSynth, thirdMinorRatio, isThirdMinorOn, false); //TODO replace false with button input
	prepareSideSynth(octaveSynth, octaveRatio, isOctaveOn, false); //TODO replace false with button input

	for(size_t i = 0; i < size; i++) {
		// get current sinusoid value and multiply it by desired gain
		auto output = mainSynth->getNextValue();

		// add intervals if they're turned on
		if(isFifthOn) output += fifthSynth->getNextValue();
		if(isFourthOn) output += fourthSynth->getNextValue();
		if(isThirdOn) output += thirdSynth->getNextValue();
		if(isThirdMinorOn) output += thirdMinorSynth->getNextValue();
		if(isOctaveOn) output += octaveSynth->getNextValue();

		output *= 0.2f; // TODO gain->get(); // multiply by the gain parameter

		// write the result to output buffer
		out[0][i] = out[1][i] = output;
    }
}

int main(void)
{
    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    sampleRate = hw.AudioSampleRate();

    hw.StartAudio(AudioCallback);
    while(1) {}
}