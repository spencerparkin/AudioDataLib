#include "MidiSynth.h"
#include "Error.h"
#include "SynthModule.h"
#include "WaveForm.h"
#include "WaveFileFormat.h"

using namespace AudioDataLib;

MidiSynth::MidiSynth(bool ownsAudioStream)
{
	this->minLatencySeconds = 0.05;
	this->maxLatencySeconds = 0.10;
	this->ownsAudioStream = ownsAudioStream;
	this->audioStream = nullptr;
#ifdef ADL_DEBUG_SYNTH_AUDIO_STREAM
	this->debugStream = nullptr;
#endif //ADL_DEBUG_SYNTH_AUDIO_STREAM
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
	if (!this->audioStream)
		return true;

	if (this->maxLatencySeconds <= this->minLatencySeconds || this->minLatencySeconds <= 0.0)
	{
		error.Add("Min/max latency parameters are not set correctly.");
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
		if (!synthModule->GenerateSound(timeNeededSeconds, format.SamplesPerSecondPerChannel(), waveForm, error))
		{
			error.Add(FormatString("Failed to generate wave-form for channel %d.", i));
			break;
		}

		if (!waveForm.ConvertToAudioBuffer(format, audioBuffer, audioBufferSize, i, error))
		{
			error.Add(FormatString("Failed to generate audio for channel %d.", i));
			break;
		}
	}

	if (!error)
	{
		this->audioStream->WriteBytesToStream(audioBuffer, audioBufferSize);

#ifdef ADL_DEBUG_SYNTH_AUDIO_STREAM
		if (!this->debugStream)
		{
			this->debugStream = new AudioStream();
			this->debugStream->SetFormat(this->audioStream->GetFormat());
		}

		this->debugStream->WriteBytesToStream(audioBuffer, audioBufferSize);

		static bool dumpDebugStream = false;
		if (dumpDebugStream)
		{
			AudioData debugData;
			debugData.SetFormat(this->debugStream->GetFormat());
			uint64_t debugDataSize = this->debugStream->GetSize();
			debugData.SetAudioBufferSize(debugDataSize);
			this->debugStream->ReadBytesFromStream(debugData.GetAudioBuffer(), debugDataSize);
			FileOutputStream outputStream("Debug.wav");
			WaveFileFormat fileFormat;
			bool wroteFile = fileFormat.WriteToStream(outputStream, &debugData, error);
			if (!wroteFile)
			{
				wroteFile = false;
			}
		}
#endif //ADL_DEBUG_SYNTH_AUDIO_STREAM
	}

	delete[] audioBuffer;

	return !error;
}

/*virtual*/ SynthModule* MidiSynth::GetRootModule(uint16_t channel)
{
	return nullptr;
}

double MidiSynth::MidiPitchToFrequency(uint8_t pitchValue)
{
	// 57 = A  = 440
	// 58 = A# = 440*2^{1/12}
	// 59 = B  = 440*(2^{1/12})^2
	// 60 = C  = 440*(2^{1/12))^3

	constexpr double halfStepRatio = 1.0594630943592953;	// This is 2^{1/12}.
	double exponent = double(int32_t(pitchValue) - 57);
	double factor = ::pow(halfStepRatio, exponent);
	double frequency = 440.0 * factor;
	return frequency;
}

double MidiSynth::MidiVelocityToAmplitude(uint8_t velocityValue)
{
	return double(velocityValue) / 127.0;
}