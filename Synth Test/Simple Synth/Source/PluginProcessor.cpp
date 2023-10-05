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
	// DEFINE AND ADD PARAMETERS - NEEDED FOR DEBUGGING PURPOSES
	addParameter (gain = new juce::AudioParameterFloat ({ "gain", 1 }, "Gain", 0.0f, 1.0f, 0.f));

	addParameter (fmA = new juce::AudioParameterFloat ({ "amplitude", 1 }, "FM Index", 0.0f, 1.0f, 0.f));

	addParameter (pitch = new juce::AudioParameterFloat({ "pitch", 1 },
														"Pitch",
														{ 16.35f, 7902.13f, 0.f, 0.199f },
														440.f,
														juce::AudioParameterFloatAttributes().withLabel(juce::String("Hz"))));

	addParameter (fmMod = new juce::AudioParameterFloat({ "fm", 1 },
														"FM Mod",
														{ 16.35f, 7902.13f, 0.f, 0.199f },
														440.f,
														juce::AudioParameterFloatAttributes().withLabel(juce::String("Hz"))));

	adsr.setParameters({0.1, 1.8, 0.5, 0.1});
}

void SimpleSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ScopedNoDenormals noDenormals;
	auto totalNumInputChannels  = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear (i, 0, buffer.getNumSamples());

	// =========== Here's where magic starts ===============

	// TODO: Where smoothing?? - Prob only need for gain smoothing, frequencies seem cool
	// TODO: Mapping of sensor input to log scale since frequencies are stupid
	// TODO: Polyphony calculation
	// TODO: Polyphony volume - how to concat sin waves in a smart way

	// GET CONSTANTS SAMPLE RATE AND BUFFER LENGTH
	const float sampleRate = getSampleRate();
	const float bufferLength = buffer.getNumSamples();

	// CALCULATE LFO PHASE CHANGE PER SAMPLE
	const auto fc = pitch->get();
	const auto lnfc = log(fc);

	S = fc * 0.005;
	I1 = 17 * (8 - lnfc)/ (lnfc * lnfc);
	I2 = 20 * (8 - lnfc)/ fc;

	lfoStep = fc / sampleRate;
	lfoM1Step = (fc + S) / sampleRate;
	lfoM2Step = (fc * 4.f + S) / sampleRate;

	// GET BOTH CHANNELS
	auto* leftChannel = buffer.getWritePointer(0);
	auto* rightChannel = buffer.getWritePointer(1);

	// e = A(t)sin[2*pi*fc*t + I1 * sin(2*pi*(fm1+S)*t) + I2 * sin(2*pi*(fm2+S)t)]
	// S = fc / 200;
	// I1 = 17*(8-ln(fc)) / (ln(fc))^2;
	// I2 = 20*(8-ln(fc)) / fc;
	// fc:fm1:fm2 == 1:1:4

	for(auto i = 0; i < bufferLength; i++) {
		lfoPhase += lfoStep;	// for each sample increment the phase

		if(lfoPhase > 1) { // modulo - we always want values between 0 and 1
			lfoPhase -= 1;
		}

		lfoM1Phase += lfoM1Step;	// for each sample increment the phase

		if(lfoM1Phase > 1) { //	 modulo - we always want values between 0 and 1
			lfoM1Phase -= 1;
		}

		lfoM2Phase += lfoM2Step;	// for each sample increment the phase

		if(lfoM2Phase > 1) { //	 modulo - we always want values between 0 and 1
			lfoM2Phase -= 1;
		}

		// calculate current sinusoid value and multiply it by desired gain
		const auto sinM1 = sin(juce::MathConstants<float>::twoPi * lfoM1Phase);
		const auto sinM2 = sin(juce::MathConstants<float>::twoPi * lfoM2Phase);
		auto output = gain->get() * sin(juce::MathConstants<float>::twoPi * lfoPhase + I1 * sinM1 + I2 * sinM2);

		// write the result to output buffer
		leftChannel[i] = output;
		rightChannel[i] = output;

		adsrResetCounter--;
		if (adsrResetCounter <= 0) {
			adsrResetCounter = 44100 * 2;
			adsr.noteOn();
		}
	}

	adsr.applyEnvelopeToBuffer(buffer, 0, bufferLength);
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
