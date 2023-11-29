#include "daisy_seed.h"
#include "daisysp.h"

#include "FM/SinusoidSynth.h"
#include "Ultrasonic/Ultrasonic.h"
#include "Mappings/Pitch.h"
#include "Mappings/Smoothing.h"

#include <memory>

using namespace daisy;
using namespace daisysp;

// Hardware
DaisySeed hw;
float sampleRate;

// Buttons
Switch octaveButton;
Switch fifthButton;
Switch fourthButton;
Switch thirdButton;
Switch thirdMinorButton;

Switch sustainButton;
// Switch button; 
// Switch button;
// Switch button;
// Switch button;

// LEDs
daisy::GPIO powerLed;
daisy::GPIO pitchClipLed;
daisy::GPIO volumeClipLed;

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

// Ultrasonic sensors
Ultrasonic pitchSensor{seed::D22, seed::D23};
Ultrasonic volumeSensor{seed::D26, seed::D27};

Smoothing pitchDistanceSmoothing{10};
Smoothing volumeDistanceSmoothing{10};
float distancePitch, distanceVolume;

float curPitch;
float curGain;

#ifdef DEBUG
uint32_t timeStart,timeEnd; //timing debugging
#endif

void init(){
    mainSynth = std::make_unique<SinusoidSynth>();
	fifthSynth = std::make_unique<SinusoidSynth>(SinusoidSynth::HarmonyRatio{3.f, 2.f});
	fourthSynth = std::make_unique<SinusoidSynth>(SinusoidSynth::HarmonyRatio{4.f, 3.f});
	thirdSynth = std::make_unique<SinusoidSynth>(SinusoidSynth::HarmonyRatio{5.f, 4.f});
	thirdMinorSynth = std::make_unique<SinusoidSynth>(SinusoidSynth::HarmonyRatio{6.f, 5.f});
	octaveSynth = std::make_unique<SinusoidSynth>(SinusoidSynth::HarmonyRatio{2.f, 1.f});
}

void prepareSideSynth(const std::unique_ptr<SinusoidSynth>& synth, bool& prevState, const bool newState){
	// if the synth was just turned on/off reset it
	if(prevState != newState){
		synth->reset(mainSynth->getCarrierPhase());
		prevState = newState;
	}

	if(!newState) return; // synth is turned off, nothing to do

	// if the synth is turned on, update pitch and sample rate values
	synth->setCarrierFrequency(curPitch);
	synth->setSampleRate(sampleRate);
}

void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
	curPitch = mapping::pitchFromDistance(pitchDistanceSmoothing.getNextValue());

	mainSynth->setCarrierFrequency(curPitch); 	// update pitch of the main synth
	mainSynth->setSampleRate(sampleRate);		// update sample rate of the main synth

	// prapare all side synths
	prepareSideSynth(fifthSynth, isFifthOn, fifthButton.Pressed()); 
	prepareSideSynth(fourthSynth, isFourthOn, fourthButton.Pressed()); 
	prepareSideSynth(thirdSynth, isThirdOn, thirdButton.Pressed()); 
	prepareSideSynth(thirdMinorSynth, isThirdMinorOn, thirdMinorButton.Pressed()); 
	prepareSideSynth(octaveSynth, isOctaveOn, octaveButton.Pressed());

	for(size_t i = 0; i < size; i++) {
		// get current sinusoid value and multiply it by desired gain
		auto output = mainSynth->getNextValue();

		// add intervals if they're turned on
		if(isFifthOn) output += fifthSynth->getNextValue();
		if(isFourthOn) output += fourthSynth->getNextValue();
		if(isThirdOn) output += thirdSynth->getNextValue();
		if(isThirdMinorOn) output += thirdMinorSynth->getNextValue();
		if(isOctaveOn) output += octaveSynth->getNextValue();

		output *= 0.1f; // TODO map distance to gain; // multiply by the gain parameter

		// write the result to output buffer
		out[0][i] = out[1][i] = output;
    }
}

int main(void)
{
	init();

    hw.Configure();
    hw.Init();
    hw.SetAudioBlockSize(4);
    sampleRate = hw.AudioSampleRate();

    octaveButton.Init(hw.GetPin(4), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    fifthButton.Init(hw.GetPin(3), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    fourthButton.Init(hw.GetPin(2), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    thirdButton.Init(hw.GetPin(1), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    thirdMinorButton.Init(hw.GetPin(0), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);

    // button.Init(hw.GetPin(5), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    // button.Init(hw.GetPin(6), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    // button.Init(hw.GetPin(7), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    // button.Init(hw.GetPin(8), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    sustainButton.Init(hw.GetPin(9), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);

	pitchClipLed.Init(seed::D14, daisy::GPIO::Mode::OUTPUT, daisy::GPIO::Pull::NOPULL);
	powerLed.Init(seed::D13, daisy::GPIO::Mode::OUTPUT, daisy::GPIO::Pull::NOPULL);
	volumeClipLed.Init(seed::D12, daisy::GPIO::Mode::OUTPUT, daisy::GPIO::Pull::NOPULL);

    hw.StartAudio(AudioCallback);

#ifdef DEBUG
	hw.StartLog(true);
#endif

	powerLed.Write(true);

    while(1) {
		// Buttons
		octaveButton.Debounce();
		fifthButton.Debounce();
		fourthButton.Debounce();
		thirdButton.Debounce();
		thirdMinorButton.Debounce();
		sustainButton.Debounce();

		// Ultrasonic sensors
		distancePitch = pitchSensor.getDistanceFiltered();
		pitchDistanceSmoothing.setTargetValue(distancePitch);

		distanceVolume = volumeSensor.getDistanceFiltered();
		volumeDistanceSmoothing.setTargetValue(distanceVolume);
	
		pitchClipLed.Write(distancePitch > mapping::MAX_DISTANCE);
		volumeClipLed.Write(distanceVolume > mapping::MAX_DISTANCE);

		daisy::System::Delay(10);
		
	#ifdef DEBUG 
		hw.SetLed(distancePitch > 50);

		hw.PrintLine("Pitch distance [mm]: %d", static_cast<int>(distancePitch));
		hw.PrintLine("Pitch mapped [Hz]: %d", static_cast<int>(curPitch));
		hw.PrintLine("Time to measure distance: %d", timeEnd - timeStart);
	#endif
	}
}