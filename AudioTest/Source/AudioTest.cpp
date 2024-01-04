#include "WaveFormat.h"
#include "ByteStream.h"
#include "AudioSink.h"
#include "WaveForm.h"
#include "AudioTest.h"
#include <iostream>

using namespace AudioDataLib;

int main(int argc, char** argv)
{
	//TestWaveFormat();
	//TestWaveForm();
	TestWaveFormAdd();
	//TestWaveAddManual();
	//TestAudioSink();

	return 0;
}

bool TestAudioSink()
{
	bool success = false;
	std::string error;

	WaveFormat waveFormat;

	AudioData* audioDataA = nullptr;
	AudioData* audioDataB = nullptr;

	FileInputStream inputStreamA("TestData/TestAudio1.wav");
	FileInputStream inputStreamB("TestData/TestAudio2.wav");
	FileOutputStream outputStream("TestData/TestAudioResult.wav");

	do
	{
		if (!waveFormat.ReadFromStream(inputStreamA, audioDataA, error))
			break;

		if (!waveFormat.ReadFromStream(inputStreamB, audioDataB, error))
			break;

		AudioSink audioSink;

		auto audioStreamA = new AudioStream(audioDataA);
		auto audioStreamB = new AudioStream(audioDataB);

		audioSink.AddAudioInput(audioStreamA);
		audioSink.AddAudioInput(audioStreamB);

		while (audioSink.GetAudioInputCount() > 0)
		{
			audioSink.MixAudio(1.0, 1.0);
		}

		AudioData generatedAudioData;
		generatedAudioData.SetAudioBufferSize(audioSink.GetAudioOutput()->GetSize());
		audioSink.GetAudioOutput()->ReadBytesFromStream(generatedAudioData.GetAudioBuffer(), generatedAudioData.GetAudioBufferSize());

		if (!waveFormat.WriteToStream(outputStream, &generatedAudioData, error))
			break;

		success = true;

	} while (false);

	if (error.size() > 0)
		std::cout << error << std::endl;

	delete audioDataA;
	delete audioDataB;

	return success;
}

bool TestWaveForm()
{
	WaveFormat waveFormat;

	AudioData* audioDataIn = nullptr;
	FileInputStream inputStream("TestData/TestAudio1.wav");

	std::string error;
	bool loadedWave = waveFormat.ReadFromStream(inputStream, audioDataIn, error);
	assert(loadedWave);

	WaveForm waveForm;
	waveForm.ConvertFromAudioBuffer(audioDataIn->GetFormat(), audioDataIn->GetAudioBuffer(), audioDataIn->GetAudioBufferSize(), 0, error);

	AudioData* audioDataOut = new AudioData();
	audioDataOut->SetAudioBufferSize(audioDataIn->GetAudioBufferSize());
	audioDataOut->GetFormat() = audioDataIn->GetFormat();

	waveForm.ConvertToAudioBuffer(audioDataOut->GetFormat(), audioDataOut->GetAudioBuffer(), audioDataOut->GetAudioBufferSize(), 0, error);

	FileOutputStream outputStream("TestData/TestAudio1_copy.wav");
	bool savedWave = waveFormat.WriteToStream(outputStream, audioDataOut, error);
	assert(savedWave);

	delete audioDataIn;
	delete audioDataOut;

	return true;
}

bool TestWaveFormat()
{
	WaveFormat waveFormat;

	AudioData* audioDataIn = nullptr;
	FileInputStream inputStream("TestData/TestAudio1.wav");

	std::string error;
	bool loadedWave = waveFormat.ReadFromStream(inputStream, audioDataIn, error);
	assert(loadedWave);

	AudioData* audioDataOut = new AudioData();
	audioDataOut->SetAudioBufferSize(audioDataIn->GetAudioBufferSize());
	::memcpy(audioDataOut->GetAudioBuffer(), audioDataIn->GetAudioBuffer(), (size_t)audioDataOut->GetAudioBufferSize());
	audioDataOut->GetFormat() = audioDataIn->GetFormat();

	FileOutputStream outputStream("TestData/TestAudio1_copy.wav");
	bool savedWave = waveFormat.WriteToStream(outputStream, audioDataOut, error);
	assert(savedWave);

	delete audioDataIn;
	delete audioDataOut;

	return true;
}

bool TestWaveFormAdd()
{
	WaveFormat waveFormat;

	AudioData* audioDataA = nullptr;
	AudioData* audioDataB = nullptr;

	FileInputStream inputStreamA("TestData/TestAudio1.wav");
	FileInputStream inputStreamB("TestData/TestAudio2.wav");

	std::string error;

	waveFormat.ReadFromStream(inputStreamA, audioDataA, error);
	waveFormat.ReadFromStream(inputStreamB, audioDataB, error);

	WaveForm waveFormA, waveFormB;

	waveFormA.ConvertFromAudioBuffer(audioDataA->GetFormat(), audioDataA->GetAudioBuffer(), audioDataA->GetAudioBufferSize(), 0, error);
	waveFormB.ConvertFromAudioBuffer(audioDataB->GetFormat(), audioDataB->GetAudioBuffer(), audioDataB->GetAudioBufferSize(), 0, error);

	WaveForm waveFormSum;
	waveFormSum.SumTogether({ &waveFormA, &waveFormB });

	AudioData* audioDataOut = new AudioData();
	AudioData::Format& format = audioDataOut->GetFormat();
	format = audioDataA->GetFormat();	// TODO: Maybe choose a different format altogether.  Just choose this one for now.

	audioDataOut->SetAudioBufferSize(waveFormSum.GetSizeBytes(format, true));
	waveFormSum.ConvertToAudioBuffer(audioDataOut->GetFormat(), audioDataOut->GetAudioBuffer(), audioDataOut->GetAudioBufferSize(), 0, error);

	FileOutputStream outputStream("TestData/TestAudioMixed.wav");
	waveFormat.WriteToStream(outputStream, audioDataOut, error);

	delete audioDataA;
	delete audioDataB;
	delete audioDataOut;

	return true;
}

bool TestWaveAddManual()
{
	WaveFormat waveFormat;

	AudioData* audioDataA = nullptr;
	AudioData* audioDataB = nullptr;

	FileInputStream inputStreamA("TestData/TestAudio1.wav");
	FileInputStream inputStreamB("TestData/TestAudio2.wav");

	std::string error;

	waveFormat.ReadFromStream(inputStreamA, audioDataA, error);
	waveFormat.ReadFromStream(inputStreamB, audioDataB, error);

	AudioData* audioDataOut = new AudioData();
	audioDataOut->GetFormat() = audioDataA->GetFormat();	// We're assuming both formats are the same, 16-bit mono.
	audioDataOut->SetAudioBufferSize(ADL_MAX(audioDataA->GetAudioBufferSize(), audioDataB->GetAudioBufferSize()));
	
	uint64_t numSamplesA = audioDataA->GetAudioBufferSize() / audioDataA->GetFormat().BytesPerSample();
	uint64_t numSamplesB = audioDataB->GetAudioBufferSize() / audioDataB->GetFormat().BytesPerSample();

	int16_t* sampleBufA = reinterpret_cast<int16_t*>(audioDataA->GetAudioBuffer());
	int16_t* sampleBufB = reinterpret_cast<int16_t*>(audioDataB->GetAudioBuffer());

	uint64_t numSamples = audioDataOut->GetAudioBufferSize() / audioDataOut->GetFormat().BytesPerSample();
	int16_t* sampleBuf = reinterpret_cast<int16_t*>(audioDataOut->GetAudioBuffer());

	constexpr int32_t maxInt = std::numeric_limits<int16_t>::max();
	constexpr int32_t minInt = std::numeric_limits<int16_t>::min();

	for (uint64_t i = 0; i < numSamples; i++)
	{
		int16_t sampleA = (i < numSamplesA) ? sampleBufA[i] : 0;
		int16_t sampleB = (i < numSamplesB) ? sampleBufB[i] : 0;

		int32_t sample = int32_t(sampleA) + int32_t(sampleB);

		if (sample < minInt)
			sample = minInt;
		if (sample > maxInt)
			sample = maxInt;

		sampleBuf[i] = int16_t(sample);
	}

	FileOutputStream outputStream("TestData/TestAudioMixed.wav");
	waveFormat.WriteToStream(outputStream, audioDataOut, error);

	delete audioDataA;
	delete audioDataB;
	delete audioDataOut;

	return true;
}