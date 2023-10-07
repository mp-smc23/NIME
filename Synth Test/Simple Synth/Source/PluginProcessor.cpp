/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleSynthAudioProcessor::SimpleSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	mainSynth = std::make_unique<SinusoidSynth>();
	fifthSynth = std::make_unique<SinusoidSynth>();
	fourthSynth = std::make_unique<SinusoidSynth>();
	thirdSynth = std::make_unique<SinusoidSynth>();
	thirdMinorSynth = std::make_unique<SinusoidSynth>();
	octaveSynth = std::make_unique<SinusoidSynth>();

	// DEFINE AND ADD PARAMETERS - NEEDED FOR DEBUGGING PURPOSES
	addParameter (gain = new juce::AudioParameterFloat ({ "gain", 1 }, "Gain", 0.0f, 1.0f, 0.f));

	addParameter (pitch = new juce::AudioParameterFloat({ "pitch", 1 },
														"Pitch",
														{ 16.35f, 7902.13f, 0.f, 0.199f },
														440.f,
														juce::AudioParameterFloatAttributes().withLabel(juce::String("Hz"))));

	addParameter(fifth = new juce::AudioParameterBool({ "fifth", 1 }, "5th", false));
	addParameter(fourth = new juce::AudioParameterBool({ "fourth", 1 }, "4th", false));
	addParameter(third = new juce::AudioParameterBool({ "third", 1 }, "3rd", false));
	addParameter(thirdMinor = new juce::AudioParameterBool({ "thirdMinor", 1 }, "3rd Minor", false));
	addParameter(octave = new juce::AudioParameterBool({ "octave", 1 }, "Octave", false));

	adsr.setParameters({0.1, 1.8, 0.5, 0.1});
}

void SimpleSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels  = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear (i, 0, buffer.getNumSamples());

	// =========== Here's where the magic starts ===============

	// TODO: Where smoothing?? - Prob only need for gain smoothing, frequencies seem cool
	// TODO: Mapping of sensor input to log scale since frequencies are stupid
	// TODO: Polyphony volume - how to concat sin waves in a smart way
	// TODO: volumne of the intervals, diff frequencies are perceived as diff volumes

	// GET CONSTANTS SAMPLE RATE AND BUFFER LENGTH
	const float sampleRate = getSampleRate();
	const float bufferLength = buffer.getNumSamples();

	curPitch = pitch->get(); // get current value of pitch parameter

	mainSynth->setCarrierFrequency(curPitch); 	// update pitch of the main synth
	mainSynth->setSampleRate(sampleRate);		// update sample rate of the main synth

	// prapare all side synths
	prepareSideSynth(fifthSynth, fifthRatio, isFifthOn, fifth->get());
	prepareSideSynth(fourthSynth, fourthRatio, isFourthOn, fourth->get());
	prepareSideSynth(thirdSynth, thirdRatio, isThirdOn, third->get());
	prepareSideSynth(thirdMinorSynth, thirdMinorRatio, isThirdMinorOn, thirdMinor->get());
	prepareSideSynth(octaveSynth,octaveRatio, isOctaveOn, octave->get());

	// GET BOTH CHANNELS
	auto* leftChannel = buffer.getWritePointer(0);
	auto* rightChannel = buffer.getWritePointer(1);

	for(auto i = 0; i < bufferLength; i++) {
		// get current sinusoid value and multiply it by desired gain
		auto output = mainSynth->getNextValue();

		// add intervals if they're turned on
		if(isFifthOn) output += fifthSynth->getNextValue();
		if(isFourthOn) output += fourthSynth->getNextValue();
		if(isThirdOn) output += thirdSynth->getNextValue();
		if(isThirdMinorOn) output += thirdMinorSynth->getNextValue();
		if(isOctaveOn) output += octaveSynth->getNextValue();

		output *= gain->get(); // multiply by the gain parameter

		// write the result to output buffer
		leftChannel[i] = output;
		rightChannel[i] = output;

		adsrResetCounter--;
		if (adsrResetCounter <= 0) {
			adsrResetCounter = 44100 * 2;
			adsr.noteOn();
		}
	}

	// slap an envelope onto it so it sounds better
	adsr.applyEnvelopeToBuffer(buffer, 0, bufferLength);
}

void SimpleSynthAudioProcessor::prepareSideSynth(const std::unique_ptr<SinusoidSynth>& synth, const HarmonyRatio& ratio, bool& prevState, const bool newState){
	// if the synth was just turned on/off reset it
	if(prevState != newState){
		synth->reset(mainSynth->getCarrierPhase());
		prevState = newState;
	}

	if(!newState) return; // synth is turned off, nothing to do

	// if the synth is turned on, update pitch and sample rate values
	const auto sampleRate = getSampleRate();
	synth->setCarrierFrequency(calculateHarmonyFrequency(curPitch, ratio));
	synth->setSampleRate(sampleRate);
}

float SimpleSynthAudioProcessor::calculateHarmonyFrequency(const float baseFrequency, const HarmonyRatio& ratio) const {
	return baseFrequency * ratio.numerator / ratio.denominator;
}

SimpleSynthAudioProcessor::~SimpleSynthAudioProcessor()
{}

//==============================================================================
const juce::String SimpleSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	adsr.setSampleRate(sampleRate);
}

void SimpleSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================
bool SimpleSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleSynthAudioProcessor::createEditor()
{
    return new SimpleSynthAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleSynthAudioProcessor();
}
