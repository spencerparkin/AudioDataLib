#include "AudioSink.h"
#include "WaveForm.h"

using namespace AudioDataLib;

AudioSink::AudioSink(bool takeOwnershipOfStreamMembory)
{
	this->ownsStreamMemory = takeOwnershipOfStreamMembory;
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
	if (this->ownsStreamMemory)
		for (AudioStream* audioStream : *this->audioStreamInArray)
			delete audioStream;

	this->audioStreamInArray->clear();

	this->SetAudioOutput(nullptr);
}

void AudioSink::SetAudioOutput(AudioStream* audioStreamOut)
{
	if (this->audioStreamOut)
	{
		if (this->ownsStreamMemory)
			delete this->audioStreamOut;
	}
	
	this->audioStreamOut = audioStreamOut;
}

void AudioSink::GenerateAudio(double desiredSecondsAvailable, double minSecondsAddedPerMix)
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
	uint64_t minBytesAddedPerMix = this->audioStreamOut->GetFormat().BytesFromSeconds(minSecondsAddedPerMix);
	if (numBytesNeeded < minBytesAddedPerMix)
		numBytesNeeded = minBytesAddedPerMix;
	numBytesNeeded = this->audioStreamOut->GetFormat().RoundUpToNearestFrameMultiple(numBytesNeeded);

	// In the trivial case, we need only write a bunch of silence to the stream.
	if (this->audioStreamInArray->size() == 0)
	{
        // Note that rather than write one byte of silence at a time, we want to
        // write all the silence at once to avoid thrashing a potential mutex
        // lock occurring on the stream if it is a thread-safe stream.
        if(numBytesNeeded > 0)
        {
            uint8_t* silenceBuffer = new uint8_t[(uint32_t)numBytesNeeded];
            ::memset(silenceBuffer, 0, (size_t)numBytesNeeded);
            this->audioStreamOut->WriteBytesToStream(silenceBuffer, numBytesNeeded);
            delete[] silenceBuffer;
        }
		return;
	}

	// Are all the formats the same?
	bool allSameFormat = true;
	for (AudioStream* audioStreamIn : *this->audioStreamInArray)
	{
		if (audioStreamIn->GetFormat() != this->audioStreamOut->GetFormat())
		{
			allSameFormat = false;
			break;
		}
	}

	// Allocate space for the audio data we're going to geneate.
	uint8_t* mixedAudioBuffer = new uint8_t[(uint32_t)numBytesNeeded];
	::memset(mixedAudioBuffer, 0, (size_t)numBytesNeeded);

	// Handle this case specifically, because it's easy and fast.
	if (allSameFormat)
	{
		uint64_t numSamples = numBytesNeeded / this->audioStreamOut->GetFormat().BytesPerSample();
		uint64_t bytesPerSample = this->audioStreamOut->GetFormat().BytesPerSample();
		for (uint64_t i = 0; i < numSamples; i++)
		{
			uint8_t* sampleBuffer = &mixedAudioBuffer[i * bytesPerSample];
			switch (this->audioStreamOut->GetFormat().bitsPerSample)
			{
				case 8:
				{
					int8_t netSample = this->CalcNetSample<int8_t>();
					::memcpy(sampleBuffer, (const void*)&netSample, sizeof(int8_t));
					break;
				}
				case 16:
				{
					int16_t netSample = this->CalcNetSample<int16_t>();
					::memcpy(sampleBuffer, (const void*)&netSample, sizeof(int16_t));
					break;
				}
				case 32:
				{
					int32_t netSample = this->CalcNetSample<int32_t>();
					::memcpy(sampleBuffer, (const void*)&netSample, sizeof(int32_t));
					break;
				}
			}
		}
	}
	else
	{
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

		// Build each channel in the audio output stream.
		for (uint32_t i = 0; i < this->audioStreamOut->GetFormat().numChannels; i++)
		{
			// If we're not mixing audio, just convert/resample.
			if (waveFormListArray[i].size() == 1)
			{
				const WaveForm* waveForm = *waveFormListArray[i].begin();
				waveForm->ConvertToAudioBuffer(this->audioStreamOut->GetFormat(), mixedAudioBuffer, numBytesNeeded, i, error);
			}
			else
			{
				// Add all the wave forms up into a single wave form for the channel.
				WaveForm mixedWave;
				mixedWave.SumTogether(waveFormListArray[i]);
				for (WaveForm* waveForm : waveFormListArray[i])
					delete waveForm;

				// Convert the aggregated wave form into audio data in the target format.
				mixedWave.ConvertToAudioBuffer(this->audioStreamOut->GetFormat(), mixedAudioBuffer, numBytesNeeded, i, error);
			}
		}

		delete[] waveFormListArray;
	}

	// Write the generated audio data to the output stream.
	this->audioStreamOut->WriteBytesToStream(mixedAudioBuffer, numBytesNeeded);

	// Free any remaining memory we used.
	delete[] mixedAudioBuffer;

	// Lastly, cull any input audio streams that have been depleted.
	uint32_t i = 0;
	while (i < this->audioStreamInArray->size())
	{
		AudioStream* audioStreamIn = (*this->audioStreamInArray)[i];
		uint64_t audioStreamInSize = audioStreamIn->GetSize();
		if (audioStreamInSize > 0)
			i++;
		else
		{
			if (this->ownsStreamMemory)
				delete audioStreamIn;
			uint32_t j = this->audioStreamInArray->size() - 1;
			if (i != j)
				(*this->audioStreamInArray)[i] = (*this->audioStreamInArray)[j];
			this->audioStreamInArray->pop_back();
		}
	}
}

void AudioSink::AddAudioInput(AudioStream* AudioStream)
{
	this->audioStreamInArray->push_back(AudioStream);
}