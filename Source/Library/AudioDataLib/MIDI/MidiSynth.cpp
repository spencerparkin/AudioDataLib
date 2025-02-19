#include "AudioDataLib/MIDI/MidiSynth.h"
#include "AudioDataLib/ErrorSystem.h"
#include "AudioDataLib/SynthModules/SynthModule.h"
#include "AudioDataLib/WaveForm.h"
#include "AudioDataLib/FileFormats/WaveFileFormat.h"

using namespace AudioDataLib;

MidiSynth::MidiSynth()
{
	this->minLatencySeconds = 0.05;
	this->maxLatencySeconds = 0.10;
}

/*virtual*/ MidiSynth::~MidiSynth()
{
	this->SetAudioStream(nullptr);
}

void MidiSynth::SetAudioStream(std::shared_ptr<AudioStream> audioStream)
{
	this->audioStream = audioStream;
}

void MidiSynth::SetMinMaxLatency(double minLatencySeconds, double maxLatencySeconds)
{
	this->minLatencySeconds = minLatencySeconds;
	this->maxLatencySeconds = maxLatencySeconds;
}

void MidiSynth::GetMinMaxLatency(double& minLatencySeconds, double& maxLatencySeconds) const
{
	minLatencySeconds = this->minLatencySeconds;
	maxLatencySeconds = this->maxLatencySeconds;
}

/*virtual*/ bool MidiSynth::Process()
{
	if (!this->audioStream)
		return true;

	if (this->maxLatencySeconds <= this->minLatencySeconds || this->minLatencySeconds <= 0.0)
	{
		ErrorSystem::Get()->Add("Min/max latency parameters are not set correctly.");
		return false;
	}

	const AudioData::Format& format = this->audioStream->GetFormat();
	uint64_t currentStreamSizeBytes = this->audioStream->GetSize();
	double currentBufferedTimeSeconds = format.BytesToSeconds(currentStreamSizeBytes);
	if (currentBufferedTimeSeconds >= this->minLatencySeconds)
		return true;

	double timeNeededSeconds = this->maxLatencySeconds - currentBufferedTimeSeconds;

	uint64_t audioBufferSize = format.BytesFromSeconds(timeNeededSeconds);
	uint8_t* audioBuffer = new uint8_t[audioBufferSize];
	::memset(audioBuffer, 0, audioBufferSize);

	for (uint16_t i = 0; i < format.numChannels; i++)
	{
		SynthModule* synthModule = this->GetRootModule(i);
		if (!synthModule)
			continue;

		WaveForm waveForm;
		if (!synthModule->GenerateSound(timeNeededSeconds, format.SamplesPerSecondPerChannel(), waveForm, nullptr))
		{
			ErrorSystem::Get()->Add(std::format("Failed to generate wave-form for channel {}.", i));
			break;
		}

		if (!waveForm.ConvertToAudioBuffer(format, audioBuffer, audioBufferSize, i))
		{
			ErrorSystem::Get()->Add(std::format("Failed to generate audio for channel {}.", i));
			break;
		}
	}

	if (!ErrorSystem::Get()->Errors())
		this->audioStream->WriteBytesToStream(audioBuffer, audioBufferSize);

	delete[] audioBuffer;

	return !ErrorSystem::Get()->Errors();
}

/*static*/ double MidiSynth::MidiPitchToFrequency(uint8_t pitchValue)
{
	// 69 = A  = 440
	// 70 = A# = 440*2^{1/12}
	// 71 = B  = 440*(2^{1/12})^2
	// 72 = C  = 440*(2^{1/12))^3

	double exponent = double(int32_t(pitchValue) - 69);
	double factor = ::pow(ADL_SEMITONE, exponent);
	double frequency = 440.0 * factor;
	return frequency;
}

/*static*/ double MidiSynth::TunePitch(double pitchHz, int16_t fineTuneCents)
{
	double fineTuneSemitones = double(fineTuneCents) / 100.0;
	double factor = ::pow(ADL_SEMITONE, fineTuneSemitones);
	double frequency = pitchHz * factor;
	return frequency;
}

/*static*/ double MidiSynth::MidiVelocityToAmplitude(uint8_t velocityValue)
{
	return double(velocityValue) / 127.0;
}