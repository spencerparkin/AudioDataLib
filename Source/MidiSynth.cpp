#include "MidiSynth.h"
#include "Error.h"

using namespace AudioDataLib;

MidiSynth::MidiSynth(bool ownsAudioStream)
{
	this->ownsAudioStream = ownsAudioStream;
	this->audioStream = nullptr;
}

/*virtual*/ MidiSynth::~MidiSynth()
{
	this->SetAudioStream(nullptr);
}

void MidiSynth::SetAudioStream(AudioStream* audioStream)
{
	if (this->ownsAudioStream)
		delete this->audioStream;

	this->audioStream = audioStream;
}

/*virtual*/ bool MidiSynth::ReceiveMessage(double deltaTimeSeconds, const uint8_t* message, uint64_t messageSize, Error& error)
{
	error.Add("Method not overridden.");
	return false;
}

/*virtual*/ bool MidiSynth::GenerateAudio(Error& error)
{
	error.Add("Method not overridden.");
	return false;
}

double MidiSynth::MidiPitchToFrequency(uint8_t pitchValue)
{
	// 56 = A  = 440
	// 57 = A# = 440*2^{1/12}
	// 58 = B  = 440*(2^{1/12})^2
	// 59 = B# = 440*(2^{1/12))^3
	// 60 = C  = 440*(2^{1/12))^4

	constexpr double halfStepRatio = 1.0594630943592953;	// This is 2^{1/12}.
	double exponent = double(int32_t(pitchValue) - 56);
	double factor = ::pow(halfStepRatio, exponent);
	double frequency = 440.0 * factor;
	return frequency;
}

double MidiSynth::MidiVelocityToAmplitude(uint8_t velocityValue)
{
	return double(velocityValue) / 127.0;
}