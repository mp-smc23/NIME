#include "daisy_seed.h"
#include "daisysp.h"

#include "FM/SinusoidSynth.h"
#include "Ultrasonic/Ultrasonic.h"
#include "Mappings/SonicSensor.h"
#include "Mappings/Smoothing.h"
#include "Mappings/Knobs.h"

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
Switch overdriveButton; 
Switch lowPassButton;
Switch chorusButton;
Switch leftRightButton;
// Switch button;

// LEDs
daisy::GPIO powerLed;
daisy::GPIO pitchClipLed;
daisy::GPIO volumeClipLed;

// Knobs
/*
0 - master volume
1 - intervals volume
2 - anchors size
3 - effects intensity
4 - cutoff freq
*/
AdcChannelConfig knobs[5];
Smoothing masterVolumeSmoothing{25};
Smoothing intervalsVolumeSmoothing{25};
Smoothing anchorsSizeSmoothing{25};
Smoothing effectsInternsitySmoothing{25};
Smoothing cutoffSmoothing{25};

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
Ultrasonic sensors[2] = {{seed::D22, seed::D23}, {seed::D26, seed::D27}};

Smoothing pitchDistanceSmoothing{10};
Smoothing volumeDistanceSmoothing{10};
float distancePitch, distanceVolume {1.f};
float curPitch, curVolume {1.f};

// Completely random effects
Tone lowPass;
Overdrive overdrive;
Chorus chorus;

#ifdef DEBUG
uint32_t timeStart, timeEnd; //timing debugging
#endif

void initSynths(){
    mainSynth = std::make_unique<SinusoidSynth>();
	fifthSynth = std::make_unique<SinusoidSynth>(SinusoidSynth::HarmonyRatio{3.f, 2.f});
	fourthSynth = std::make_unique<SinusoidSynth>(SinusoidSynth::HarmonyRatio{4.f, 3.f});
	thirdSynth = std::make_unique<SinusoidSynth>(SinusoidSynth::HarmonyRatio{5.f, 4.f});
	thirdMinorSynth = std::make_unique<SinusoidSynth>(SinusoidSynth::HarmonyRatio{6.f, 5.f});
	octaveSynth = std::make_unique<SinusoidSynth>(SinusoidSynth::HarmonyRatio{2.f, 1.f});
}

void initButtons(){
    octaveButton.Init(hw.GetPin(4), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    fifthButton.Init(hw.GetPin(3), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    fourthButton.Init(hw.GetPin(2), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    thirdButton.Init(hw.GetPin(1), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    thirdMinorButton.Init(hw.GetPin(0), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);

    lowPassButton.Init(hw.GetPin(5), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    overdriveButton.Init(hw.GetPin(6), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    chorusButton.Init(hw.GetPin(7), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    // button.Init(hw.GetPin(8), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
    sustainButton.Init(hw.GetPin(9), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);

    leftRightButton.Init(hw.GetPin(11), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
}

void initLeds(){
	pitchClipLed.Init(seed::D14, daisy::GPIO::Mode::OUTPUT, daisy::GPIO::Pull::NOPULL);
	powerLed.Init(seed::D13, daisy::GPIO::Mode::OUTPUT, daisy::GPIO::Pull::NOPULL);
	volumeClipLed.Init(seed::D12, daisy::GPIO::Mode::OUTPUT, daisy::GPIO::Pull::NOPULL);
}

void initKnobs(){
	knobs[0].InitSingle(seed::A0);
	knobs[1].InitSingle(seed::A1);
	knobs[2].InitSingle(seed::A2);
	knobs[3].InitSingle(seed::A3);
	knobs[4].InitSingle(seed::A4);

	hw.adc.Init(knobs, 5);
}

void initEffects(){
	lowPass.Init(sampleRate);
	
	overdrive.Init();
	overdrive.SetDrive(0.4f);

	chorus.Init(sampleRate);
    chorus.SetDelay(1.f);
    chorus.SetFeedback(0.5f);
	chorus.SetLfoDepth(1.f);
	chorus.SetLfoFreq(6.5f);
}

void prepareSideSynth(const std::unique_ptr<SinusoidSynth>& synth, bool& prevState, const bool newState){
	// if the synth was just turned on/off reset it
	if(prevState != newState){
		synth->reset(mainSynth->getCarrierPhase());
		synth->startAttackPhase();
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
	// Get and/or calculate values for processing
	curPitch = mapping::pitchFromDistance(pitchDistanceSmoothing.getNextValue(), anchorsSizeSmoothing.getNextValue());	
	if(!sustainButton.Pressed()) {
		curVolume = mapping::gainFromDistance(volumeDistanceSmoothing.getNextValue());
	}

	const auto volume = curVolume * mapping::equalLoudness(curPitch) * masterVolumeSmoothing.getNextValue();
	const auto intervalsVolume = intervalsVolumeSmoothing.getNextValue();

	mainSynth->setCarrierFrequency(curPitch); 	// update pitch of the main synth
	mainSynth->setSampleRate(sampleRate);		// update sample rate of the main synth

	// prapare all side synths
	prepareSideSynth(fifthSynth, isFifthOn, fifthButton.Pressed()); 
	prepareSideSynth(fourthSynth, isFourthOn, fourthButton.Pressed()); 
	prepareSideSynth(thirdSynth, isThirdOn, thirdButton.Pressed()); 
	prepareSideSynth(thirdMinorSynth, isThirdMinorOn, thirdMinorButton.Pressed()); 
	prepareSideSynth(octaveSynth, isOctaveOn, octaveButton.Pressed());

	lowPass.SetFreq(cutoffSmoothing.getNextValue());
	const auto effectsIntensity = effectsInternsitySmoothing.getNextValue();

	for(size_t i = 0; i < size; i++) {
		// get current sinusoid value and multiply it by desired gain
		auto output = mainSynth->getNextValue();

		// add intervals if they're turned on
		if(isFifthOn) output += fifthSynth->getNextValue() * intervalsVolume;
		if(isFourthOn) output += fourthSynth->getNextValue() * intervalsVolume;
		if(isThirdOn) output += thirdSynth->getNextValue() * intervalsVolume;
		if(isThirdMinorOn) output += thirdMinorSynth->getNextValue() * intervalsVolume;
		if(isOctaveOn) output += octaveSynth->getNextValue() * intervalsVolume;

		// Effects
		if(overdriveButton.Pressed()) output = (1 - effectsIntensity) * output + effectsIntensity * overdrive.Process(output);
		if(chorusButton.Pressed()) output = (1 - effectsIntensity) * output + effectsIntensity * chorus.Process(output);
		if(lowPassButton.Pressed()) output = lowPass.Process(output);

		// Gain
		output *= volume; 

		// write the result to output buffer
		out[0][i] = out[1][i] = output;
    }
}

int main(void)
{
    hw.Configure();
    hw.Init();
    sampleRate = hw.AudioSampleRate();

	initSynths();
	initButtons();
	initLeds();
	initKnobs();
	initEffects();
 
	hw.adc.Start(); // Start the ADC
    hw.StartAudio(AudioCallback); // Start audio callback

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
		lowPassButton.Debounce();
		overdriveButton.Debounce();
		chorusButton.Debounce();
		leftRightButton.Debounce();

		// Ultrasonic sensors
		distancePitch = sensors[!leftRightButton.Pressed()].getDistanceFiltered();
		pitchDistanceSmoothing.setTargetValue(distancePitch);

		daisy::System::Delay(10);

		distanceVolume = sensors[leftRightButton.Pressed()].getDistanceFiltered();
		volumeDistanceSmoothing.setTargetValue(distanceVolume);
	
		daisy::System::Delay(10);

		// LEDs
		pitchClipLed.Write(distancePitch > mapping::MAX_DISTANCE);
		volumeClipLed.Write(distanceVolume > mapping::MAX_DISTANCE);

		// Knobs
    	masterVolumeSmoothing.setTargetValue(hw.adc.GetFloat(0));
		intervalsVolumeSmoothing.setTargetValue(mapping::intervalVolumeScaled(hw.adc.GetFloat(1)));
		anchorsSizeSmoothing.setTargetValue(mapping::anchorsSizeScaled(hw.adc.GetFloat(2))); 
		effectsInternsitySmoothing.setTargetValue(mapping::effectsInternsityScaled(hw.adc.GetFloat(3)));
		cutoffSmoothing.setTargetValue(mapping::cutoffScaled(hw.adc.GetFloat(4)));
	
	#ifdef DEBUG 
		hw.SetLed(distancePitch > 50);

		// hw.PrintLine("Master Volume [* 100]: %d", static_cast<int>(hw.adc.GetFloat(0) * 100));
		// hw.PrintLine("Intervals Volume Scaled [* 100]: %d", static_cast<int>(mapping::intervalVolumeScaled(hw.adc.GetFloat(1)) * 100));
		// hw.PrintLine("Anchors Size Scaled: %d", static_cast<int>(mapping::anchorsSizeScaled(hw.adc.GetFloat(2))));
		hw.PrintLine("Effects Internsity Scaled [* 100]: %d", static_cast<int>(mapping::effectsInternsityScaled(hw.adc.GetFloat(3)) * 100));
		hw.PrintLine("Cutoff Scaled [Hz]: %d", static_cast<int>(mapping::cutoffScaled(hw.adc.GetFloat(4))));

		// hw.PrintLine("Low Pass Pressed: %d" , static_cast<int>(lowPassButton.Pressed()));
		// hw.PrintLine("Bit Crush Pressed: %d" , static_cast<int>(overdriveButton.Pressed()));
		// hw.PrintLine("Phaser Pressed: %d" , static_cast<int>(chorusButton.Pressed()));

		// hw.PrintLine("Pitch distance [mm]: %d", static_cast<int>(distancePitch));
		// hw.PrintLine("Pitch mapped [Hz]: %d", static_cast<int>(curPitch));
		// hw.PrintLine("Volume distance [mm]: %d", static_cast<int>(distanceVolume));
		// hw.PrintLine("Volume mapped [Hz]: %d", static_cast<int>(curVolume));
		// hw.PrintLine("Time to measure distance: %d", timeEnd - timeStart);
		// TODO implement left/right hand switch 
		daisy::System::Delay(100);
	#endif
	}
}