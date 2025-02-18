#include "AudioDataLib/AudioSink.h"
#include "AudioDataLib/WaveForm.h"
#include "AudioDataLib/ErrorSystem.h"

using namespace AudioDataLib;

AudioSink::AudioSink()
{
	this->audioStreamInArray = new std::vector<std::shared_ptr<AudioStream>>();
	this->audioStreamOut = new std::shared_ptr<AudioStream>();
}

/*virtual*/ AudioSink::~AudioSink()
{
	delete this->audioStreamInArray;
	delete this->audioStreamOut;
}

void AudioSink::Clear()
{
	this->audioStreamInArray->clear();
	this->audioStreamOut->reset();
}

void AudioSink::SetAudioOutput(std::shared_ptr<AudioStream> audioStreamOut)
{
	*this->audioStreamOut = audioStreamOut;
}

void AudioSink::GenerateAudio(double desiredSecondsAvailable, double minSecondsAddedPerMix)
{
	// There's nothing for us to do if we don't have a place to put generated audio.
	if (!this->audioStreamOut)
		return;

	// Early out here if there is already enough data available.
	uint64_t numDesiredBytesAvailable = (*this->audioStreamOut)->GetFormat().BytesFromSeconds(desiredSecondsAvailable);
	uint64_t numActualBytesAvailable = (*this->audioStreamOut)->GetSize();
	if (numDesiredBytesAvailable <= numActualBytesAvailable)
		return;

	// How much data do we need to add to the output stream?
	uint64_t numBytesNeeded = numDesiredBytesAvailable - numActualBytesAvailable;
	uint64_t minBytesAddedPerMix = (*this->audioStreamOut)->GetFormat().BytesFromSeconds(minSecondsAddedPerMix);
	if (numBytesNeeded < minBytesAddedPerMix)
		numBytesNeeded = minBytesAddedPerMix;
	numBytesNeeded = (*this->audioStreamOut)->GetFormat().RoundUpToNearestFrameMultiple(numBytesNeeded);

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
            (*this->audioStreamOut)->WriteBytesToStream(silenceBuffer, numBytesNeeded);
            delete[] silenceBuffer;
        }
		return;
	}

	// Are all the formats the same?
	bool allSameFormat = true;
	for (auto& audioStreamIn : *this->audioStreamInArray)
	{
		if (audioStreamIn->GetFormat() != (*this->audioStreamOut)->GetFormat())
		{
			allSameFormat = false;
			break;
		}
	}

	// Allocate space for the audio data we're going to geneate.
	uint8_t* generatedAudioBuffer = new uint8_t[(uint32_t)numBytesNeeded];
	::memset(generatedAudioBuffer, 0, (size_t)numBytesNeeded);

	// Handle this case specifically, because it's easy and fast.
	if (allSameFormat)
	{
		uint64_t numSamples = numBytesNeeded / (*this->audioStreamOut)->GetFormat().BytesPerSample();
		uint64_t bytesPerSample = (*this->audioStreamOut)->GetFormat().BytesPerSample();
		for (uint64_t i = 0; i < numSamples; i++)
		{
			uint8_t* sampleBuffer = &generatedAudioBuffer[i * bytesPerSample];

			if ((*this->audioStreamOut)->GetFormat().sampleType == AudioData::Format::SIGNED_INTEGER)
			{
				switch ((*this->audioStreamOut)->GetFormat().bitsPerSample)
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
			else if ((*this->audioStreamOut)->GetFormat().sampleType == AudioData::Format::UNSIGNED_INTEGER)
			{
				switch ((*this->audioStreamOut)->GetFormat().bitsPerSample)
				{
					case 8:
					{
						uint8_t netSample = this->CalcNetSample<uint8_t>();
						::memcpy(sampleBuffer, (const void*)&netSample, sizeof(uint8_t));
						break;
					}
					case 16:
					{
						uint16_t netSample = this->CalcNetSample<uint16_t>();
						::memcpy(sampleBuffer, (const void*)&netSample, sizeof(uint16_t));
						break;
					}
					case 32:
					{
						uint32_t netSample = this->CalcNetSample<uint32_t>();
						::memcpy(sampleBuffer, (const void*)&netSample, sizeof(uint32_t));
						break;
					}
				}
			}
			else if ((*this->audioStreamOut)->GetFormat().sampleType == AudioData::Format::FLOAT)
			{
				switch ((*this->audioStreamOut)->GetFormat().bitsPerSample)
				{
					case 32:
					{
						float netSample = this->CalcNetSample<float>();
						::memcpy(sampleBuffer, (const void*)&netSample, sizeof(float));
						break;
					}
					case 64:
					{
						double netSample = this->CalcNetSample<double>();
						::memcpy(sampleBuffer, (const void*)&netSample, sizeof(double));
						break;
					}
				}
				
			}
		}
	}
	else
	{
		// Grab audio buffers from all the inputs and transform them into wave form space.
		double secondsNeeded = (*this->audioStreamOut)->GetFormat().BytesToSeconds(numBytesNeeded);
		auto waveFormListArray = new std::list<WaveForm*>[(*this->audioStreamOut)->GetFormat().numChannels];
		for (auto& audioStreamIn : *this->audioStreamInArray)
		{
			uint64_t audioBufferSize = audioStreamIn->GetFormat().BytesFromSeconds(secondsNeeded);
			audioBufferSize = audioStreamIn->GetFormat().RoundUpToNearestFrameMultiple(audioBufferSize);
			uint8_t* audioBuffer = new uint8_t[(size_t)audioBufferSize];
			uint64_t numBytesRead = audioStreamIn->ReadBytesFromStream(audioBuffer, audioBufferSize);
			for (uint64_t i = numBytesRead; i < audioBufferSize; i++)
				audioBuffer[i] = 0;

			for (uint32_t i = 0; i < (*this->audioStreamOut)->GetFormat().numChannels; i++)
			{
				WaveForm* waveForm = new WaveForm();
				if(!waveForm->ConvertFromAudioBuffer(audioStreamIn->GetFormat(), audioBuffer, audioBufferSize, i))
					waveForm->MakeSilence(audioStreamIn->GetFormat().framesPerSecond, secondsNeeded);
				waveFormListArray[i].push_back(waveForm);
			}

			delete[] audioBuffer;
		}

		// Build each channel in the audio output stream.
		for (uint32_t i = 0; i < (*this->audioStreamOut)->GetFormat().numChannels; i++)
		{
			// If we're not mixing audio, just convert/resample.
			if (waveFormListArray[i].size() == 1)
			{
				const WaveForm* waveForm = *waveFormListArray[i].begin();
				waveForm->ConvertToAudioBuffer((*this->audioStreamOut)->GetFormat(), generatedAudioBuffer, numBytesNeeded, i);
			}
			else
			{
				// Add all the wave forms up into a single wave form for the channel.
				WaveForm mixedWave;
				mixedWave.SumTogether(waveFormListArray[i]);
				for (WaveForm* waveForm : waveFormListArray[i])
					delete waveForm;

				// Convert the aggregated wave form into audio data in the target format.
				mixedWave.ConvertToAudioBuffer((*this->audioStreamOut)->GetFormat(), generatedAudioBuffer, numBytesNeeded, i);
			}
		}

		delete[] waveFormListArray;
	}

	// Write the generated audio data to the output stream.
	(*this->audioStreamOut)->WriteBytesToStream(generatedAudioBuffer, numBytesNeeded);

	// Free any remaining memory we used.
	delete[] generatedAudioBuffer;

	// Lastly, cull any input audio streams that have been depleted.
	uint32_t i = 0;
	while (i < this->audioStreamInArray->size())
	{
		AudioStream* audioStreamIn = (*this->audioStreamInArray)[i].get();
		uint64_t audioStreamInSize = audioStreamIn->GetSize();
		if (audioStreamInSize > 0)
			i++;
		else
		{
			uint32_t j = uint32_t(this->audioStreamInArray->size()) - 1;
			if (i != j)
				(*this->audioStreamInArray)[i] = (*this->audioStreamInArray)[j];
			this->audioStreamInArray->pop_back();
		}
	}
}

void AudioSink::AddAudioInput(std::shared_ptr<AudioStream> audioStream)
{
	this->audioStreamInArray->push_back(audioStream);
}