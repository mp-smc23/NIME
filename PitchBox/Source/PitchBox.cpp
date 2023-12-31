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
Switch leftTop[2]; // third mj / chorus
Switch rightTop[2]; // third min / overdrive
Switch leftMiddle[2]; // fifth / mute
Switch rightMiddle[2]; // forth / nothign
Switch bottom[2]; // octave / sustain

Switch leftRightButton;
bool isLeftRight;

// Switch button;
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

Smoothing pitchDistanceSmoothing{25};
Smoothing volumeDistanceSmoothing{25};
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
	leftTop[0].Init(hw.GetPin(4), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);
    rightTop[0].Init(hw.GetPin(2), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);
    leftMiddle[0].Init(hw.GetPin(0), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);
    rightMiddle[0].Init(hw.GetPin(1), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);
	bottom[0].Init(hw.GetPin(3), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);

    leftTop[1].Init(hw.GetPin(7), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);
    rightTop[1].Init(hw.GetPin(9), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);
    leftMiddle[1].Init(hw.GetPin(6), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);
    rightMiddle[1].Init(hw.GetPin(5), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);
    bottom[1].Init(hw.GetPin(8), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_INVERTED, Switch::Pull::PULL_UP);

    leftRightButton.Init(hw.GetPin(11), 1000, Switch::Type::TYPE_MOMENTARY, Switch::Polarity::POLARITY_NORMAL, Switch::Pull::PULL_UP);
}

void initLeds(){
	pitchClipLed.Init(seed::D13, daisy::GPIO::Mode::OUTPUT, daisy::GPIO::Pull::NOPULL);
	powerLed.Init(seed::D12, daisy::GPIO::Mode::OUTPUT, daisy::GPIO::Pull::NOPULL);
	volumeClipLed.Init(seed::D14, daisy::GPIO::Mode::OUTPUT, daisy::GPIO::Pull::NOPULL);
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

/// Updates the interval synths' states. Turns them on and off depending on the current and previous states and initializes the attack and decay phases accordingly.
void prepareSideSynth(const std::unique_ptr<SinusoidSynth>& synth, bool& prevState, const bool newState){
	// if the synth was just turned on/off reset it
	if(prevState != newState){
		prevState = newState;

		if(newState) synth->startAttackPhase();
		else synth->startDecayPhase();
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
	if(!bottom[isLeftRight].Pressed()) { // If not in the Sustain Mode, update curVolume value
		curVolume = mapping::gainFromDistance(volumeDistanceSmoothing.getNextValue());
	}

	const auto volume = curVolume * mapping::equalLoudness(curPitch) * masterVolumeSmoothing.getNextValue(); // final volume
	const auto intervalsVolume = intervalsVolumeSmoothing.getNextValue();

	mainSynth->setCarrierFrequency(curPitch); 	// update pitch of the main synth
	mainSynth->setSampleRate(sampleRate);		// update sample rate of the main synth

	// prapare all interval synths
	prepareSideSynth(fifthSynth, isFifthOn, leftMiddle[!isLeftRight].Pressed()); 
	prepareSideSynth(fourthSynth, isFourthOn, rightMiddle[!isLeftRight].Pressed()); 
	prepareSideSynth(thirdSynth, isThirdOn, leftTop[!isLeftRight].Pressed()); 
	prepareSideSynth(thirdMinorSynth, isThirdMinorOn, rightTop[!isLeftRight].Pressed()); 
	prepareSideSynth(octaveSynth, isOctaveOn, bottom[!isLeftRight].Pressed());

	lowPass.SetFreq(cutoffSmoothing.getNextValue()); // set new lowPass cutoff frequency
	const auto effectsIntensity = effectsInternsitySmoothing.getNextValue(); // get current effects intensity value

	for(size_t i = 0; i < size; i++) {
		// get current sinusoid value
		auto output = mainSynth->getNextValue();

		// Add intervals scaled by the intervals volume
		output += fifthSynth->getNextValue() * intervalsVolume;
		output += fourthSynth->getNextValue() * intervalsVolume;
		output += thirdSynth->getNextValue() * intervalsVolume;
		output += thirdMinorSynth->getNextValue() * intervalsVolume;
		output += octaveSynth->getNextValue() * intervalsVolume;

		// Effects - effectsIntensity acts as a dry/wet
		if(rightTop[isLeftRight].Pressed()) output = (1 - effectsIntensity) * output + effectsIntensity * overdrive.Process(output);
		if(leftTop[isLeftRight].Pressed()) output = (1 - effectsIntensity) * output + effectsIntensity * chorus.Process(output);
		
		output = lowPass.Process(output); // Process the output through a low pass filter

		// Gain
		output *= volume; 

		// write the result to output buffer
		out[0][i] = out[1][i] = output;
    }
}

int main(void)
{
	// Initialize all the hardware
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
		// Debounce the buttons
		leftTop[0].Debounce();
		leftTop[1].Debounce();
		rightTop[0].Debounce();
		rightTop[1].Debounce();
		leftMiddle[0].Debounce();
		leftMiddle[1].Debounce();
		rightMiddle[0].Debounce();
		rightMiddle[1].Debounce();
		bottom[0].Debounce();
		bottom[1].Debounce();
		leftRightButton.Debounce();

		isLeftRight = leftRightButton.Pressed();

		// Read ultrasonic sensors distances
		distancePitch = sensors[!isLeftRight].getDistanceFiltered(0.5f, 6000U); // 6k microsec timeout ~ 1200 mm
		pitchDistanceSmoothing.setTargetValue(distancePitch < 0.f ? 0.f : distancePitch);

		daisy::System::Delay(5);

		distanceVolume = sensors[isLeftRight].getDistanceFiltered(0.5f, 6000U); // 6k microsec timeout ~ 1200 mm
		volumeDistanceSmoothing.setTargetValue(distanceVolume < 0.f ? 0.f : distanceVolume);
	
		daisy::System::Delay(5);

		// Update the LEDs
		pitchClipLed.Write(distancePitch > mapping::MAX_DISTANCE || distancePitch < 0);
		volumeClipLed.Write(distanceVolume > mapping::MAX_DISTANCE || distanceVolume < 0);

		// Read values of the knobs and udpate each smoothing's target value to the new readings
    	masterVolumeSmoothing.setTargetValue(leftMiddle[isLeftRight].Pressed() ? 0.f : hw.adc.GetFloat(0));
		intervalsVolumeSmoothing.setTargetValue(mapping::intervalVolumeScaled(hw.adc.GetFloat(1)));
		anchorsSizeSmoothing.setTargetValue(mapping::anchorsSizeScaled(hw.adc.GetFloat(2))); 
		effectsInternsitySmoothing.setTargetValue(mapping::effectsInternsityScaled(hw.adc.GetFloat(3)));
		cutoffSmoothing.setTargetValue(mapping::cutoffScaled(hw.adc.GetFloat(4)));
	
	#ifdef DEBUG 
		// hw.PrintLine("Master Volume [* 100]: %d", static_cast<int>(hw.adc.GetFloat(0) * 100));
		hw.PrintLine("Intervals Volume Scaled [* 100]: %d", static_cast<int>(mapping::intervalVolumeScaled(hw.adc.GetFloat(1)) * 100));
		// hw.PrintLine("Anchors Size Scaled: %d", static_cast<int>(mapping::anchorsSizeScaled(hw.adc.GetFloat(2))));
		// hw.PrintLine("Effects Internsity Scaled [* 100]: %d", static_cast<int>(mapping::effectsInternsityScaled(hw.adc.GetFloat(3)) * 100));
		// hw.PrintLine("Cutoff Scaled [Hz]: %d", static_cast<int>(mapping::cutoffScaled(hw.adc.GetFloat(4))));

		// hw.PrintLine("overdrive Pressed: %d" , static_cast<int>(overdriveButton.Pressed()));
		// hw.PrintLine("chorus Pressed: %d" , static_cast<int>(chorusButton.Pressed()));
		// hw.PrintLine("sustainButton: %d" , static_cast<int>(sustainButton.Pressed()));
		// hw.PrintLine("leftRightButton: %d" , static_cast<int>(leftRightButton.Pressed()));

		// hw.PrintLine("fifthButton: %d" , static_cast<int>(fifthButton.Pressed()));
		// hw.PrintLine("fourthButton: %d" , static_cast<int>(fourthButton.Pressed()));
		// hw.PrintLine("thirdButton: %d" , static_cast<int>(leftTop[!isLeftRight].Pressed()));
		// hw.PrintLine("thirdMinorButton: %d" , static_cast<int>(rightTop[!isLeftRight].Pressed()));
		// hw.PrintLine("Octave Pressed: %d" , static_cast<int>(octaveButton.Pressed()));


		// hw.PrintLine("Pitch [hz]: %d", static_cast<int>(mapping::pitchFromDistance(distancePitch, mapping::anchorsSizeScaled(hw.adc.GetFloat(2)))));
		// hw.PrintLine("note index: %d", static_cast<int>(mapping::indexFromDistance(distancePitch, mapping::anchorsSizeScaled(hw.adc.GetFloat(2)))));
		// hw.PrintLine("Volume distance [mm]: %d", static_cast<int>(distanceVolume));

		// hw.PrintLine("Pitch mapped [Hz]: %d", static_cast<int>(curPitch));
		// hw.PrintLine("Volume mapped [Hz]: %d", static_cast<int>(curVolume));

		// hw.PrintLine("Time to measure distance: %d", timeEnd - timeStart);

		daisy::System::Delay(50);
		hw.PrintLine("========================================");
	#endif
	}
}