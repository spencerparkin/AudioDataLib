#include "AudioSink.h"
#include "WaveForm.h"

using namespace AudioDataLib;

AudioSink::AudioSink()
{
	this->audioDataInArray = new std::vector<AudioData*>();
}

/*virtual*/ AudioSink::~AudioSink()
{
	for (AudioData* audioData : *this->audioDataInArray)
		delete audioData;

	delete this->audioDataInArray;
}

void AudioSink::MixAudio(double desiredSecondsAvailable, double secondsAddedPerMix)
{
	// Early out here if there is already enough data available.
	uint64_t numDesiredBytesAvailable = this->audioDataOut.GetFormat().BytesFromSeconds(desiredSecondsAvailable);
	uint64_t numActualBytesAvailable = this->audioDataOut.GetAudioStream()->GetSize();
	if (numDesiredBytesAvailable <= numActualBytesAvailable)
		return;

	// How much data do we need to add to the output stream?
	uint64_t numBytesNeeded = numDesiredBytesAvailable - numActualBytesAvailable;
	uint64_t numBytesAddedPerMix = this->audioDataOut.GetFormat().BytesFromSeconds(secondsAddedPerMix);
	if (numBytesNeeded < numBytesAddedPerMix)
		numBytesNeeded = numBytesAddedPerMix;
	numBytesNeeded = this->audioDataOut.GetFormat().RoundUpToNearestFrameMultiple(numBytesNeeded);

	// In the trivial case, don't use the wave form stuff.
	if (this->audioDataInArray->size() == 0)
	{
		uint8_t silenceByte = 0;
		while (numBytesNeeded-- > 0)
			this->audioDataOut.GetAudioStream()->WriteBytesToStream(&silenceByte, 1);
		
		return;
	}

	// Grab audio buffers from all the inputs and transform them into wave form space.
	double secondsNeeded = this->audioDataOut.GetFormat().BytesToSeconds(numBytesNeeded);
	std::list<WaveForm*> waveFormList;	
	for (AudioData* audioDataIn : *this->audioDataInArray)
	{
		uint64_t bufferSize = audioDataIn->GetFormat().BytesFromSeconds(secondsNeeded);
		bufferSize = audioDataIn->GetFormat().RoundUpToNearestFrameMultiple(bufferSize);
		uint8_t* buffer = new uint8_t[(uint32_t)bufferSize];
		uint64_t numBytesRead = audioDataIn->GetAudioStream()->ReadBytesFromStream(buffer, bufferSize);
		for (uint64_t i = numBytesRead; i < bufferSize; i++)
			buffer[i] = 0;

		WaveForm* waveForm = new WaveForm();
		waveForm->ConvertFromAudioBuffer(audioDataIn->GetFormat(), buffer, bufferSize);
		waveFormList.push_back(waveForm);

		delete[] buffer;
	}

	// Now add all the wave forms up into a single wave form.
	WaveForm mixedWave;
	mixedWave.SumTogether(waveFormList);
	for (WaveForm* waveForm : waveFormList)
		delete waveForm;
	waveFormList.clear();

	// Finally, convert the aggregated wave form into audio data in the target format.
	uint64_t bufferSize = 0;
	uint8_t* buffer = nullptr;
	mixedWave.ConvertToAudioBuffer(this->audioDataOut.GetFormat(), buffer, bufferSize);
	this->audioDataOut.GetAudioStream()->WriteBytesToStream(buffer, bufferSize);
	delete[] buffer;
}

void AudioSink::AddAudioSource(AudioData* audioData)
{
	this->audioDataInArray->push_back(audioData);
}