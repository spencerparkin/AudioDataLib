#include "AudioSink.h"
#include "WaveForm.h"

using namespace AudioDataLib;

AudioSink::AudioSink()
{
	this->audioStreamInArray = new std::vector<AudioStream*>();
	this->audioStreamOut = nullptr;
}

/*virtual*/ AudioSink::~AudioSink()
{
	this->Clear();

	delete this->audioStreamInArray;
}

void AudioSink::Clear()
{
	for (AudioStream* audioStream : *this->audioStreamInArray)
		delete audioStream;

	this->audioStreamInArray->clear();

	this->SetAudioOutput(nullptr);
}

void AudioSink::SetAudioOutput(AudioStream* audioStreamOut)
{
	if (this->audioStreamOut)
		delete this->audioStreamOut;
	
	this->audioStreamOut = audioStreamOut;
}

// I wonder if this is going to be really, really slow.
void AudioSink::MixAudio(double desiredSecondsAvailable, double secondsAddedPerMix)
{
	// There's nothing for us to do if we don't have a place to put generated audio.
	if (!this->audioStreamOut)
		return;

	// Early out here if there is already enough data available.
	uint64_t numDesiredBytesAvailable = this->audioStreamOut->GetFormat().BytesFromSeconds(desiredSecondsAvailable);
	uint64_t numActualBytesAvailable = this->audioStreamOut->GetSize();
	if (numDesiredBytesAvailable <= numActualBytesAvailable)
		return;

	// How much data do we need to add to the output stream?
	uint64_t numBytesNeeded = numDesiredBytesAvailable - numActualBytesAvailable;
	uint64_t numBytesAddedPerMix = this->audioStreamOut->GetFormat().BytesFromSeconds(secondsAddedPerMix);
	if (numBytesNeeded < numBytesAddedPerMix)
		numBytesNeeded = numBytesAddedPerMix;
	numBytesNeeded = this->audioStreamOut->GetFormat().RoundUpToNearestFrameMultiple(numBytesNeeded);

	// In the trivial case, don't use the wave form stuff.
	if (this->audioStreamInArray->size() == 0)
	{
		uint8_t silenceByte = 0;
		while (numBytesNeeded-- > 0)
			this->audioStreamOut->WriteBytesToStream(&silenceByte, 1);
		
		return;
	}

	// Grab audio buffers from all the inputs and transform them into wave form space.
	std::string error;
	double secondsNeeded = this->audioStreamOut->GetFormat().BytesToSeconds(numBytesNeeded);
	auto waveFormListArray = new std::list<WaveForm*>[this->audioStreamOut->GetFormat().numChannels];
	for (uint32_t i = 0; i < this->audioStreamOut->GetFormat().numChannels; i++)
	{
		for (AudioStream* audioStreamIn : *this->audioStreamInArray)
		{
			uint64_t bufferSize = audioStreamIn->GetFormat().BytesFromSeconds(secondsNeeded);
			bufferSize = audioStreamIn->GetFormat().RoundUpToNearestFrameMultiple(bufferSize);
			uint8_t* buffer = new uint8_t[(uint32_t)bufferSize];
			uint64_t numBytesRead = audioStreamIn->ReadBytesFromStream(buffer, bufferSize);
			for (uint64_t i = numBytesRead; i < bufferSize; i++)
				buffer[i] = 0;

			WaveForm* waveForm = new WaveForm();
			waveForm->ConvertFromAudioBuffer(audioStreamIn->GetFormat(), buffer, bufferSize, i, error);
			waveFormListArray[i].push_back(waveForm);
			delete[] buffer;
		}
	}

	// Allocate space for the audio data we're going to geneate.
	uint8_t* mixedAudioBuffer = new uint8_t[(uint32_t)numBytesNeeded];
	::memset(mixedAudioBuffer, 0, (size_t)numBytesNeeded);

	// Build each channel in the audio output stream.
	for (uint32_t i = 0; i < this->audioStreamOut->GetFormat().numChannels; i++)
	{
		// Add all the wave forms up into a single wave form for the channel.
		WaveForm mixedWave;
		mixedWave.SumTogether(waveFormListArray[i]);
		for (WaveForm* waveForm : waveFormListArray[i])
			delete waveForm;

		// Convert the aggregated wave form into audio data in the target format.
		mixedWave.ConvertToAudioBuffer(this->audioStreamOut->GetFormat(), mixedAudioBuffer, numBytesNeeded, i, error);
	}

	// Lastly, write the generated audio data to the output stream.
	this->audioStreamOut->WriteBytesToStream(mixedAudioBuffer, numBytesNeeded);

	// Free any remaining memory we used.
	delete[] waveFormListArray;
	delete[] mixedAudioBuffer;
}

void AudioSink::AddAudioInput(AudioStream* AudioStream)
{
	this->audioStreamInArray->push_back(AudioStream);
}