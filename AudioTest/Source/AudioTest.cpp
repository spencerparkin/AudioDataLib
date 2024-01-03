#include "WaveFormat.h"
#include "ByteStream.h"
#include "AudioSink.h"
#include "WaveForm.h"
#include "AudioTest.h"
#include <iostream>

using namespace AudioDataLib;

int main(int argc, char** argv)
{
	TestWaveFormat();
	//TestWaveForm();
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

		audioSink.AddAudioSource(audioDataA);
		audioSink.AddAudioSource(audioDataB);

		while (audioSink.GetAudioInputCount() > 0)
		{
			audioSink.MixAudio(1.0, 1.0);
		}

		if (!waveFormat.WriteToStream(outputStream, audioSink.GetAudioOutput(), error))
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

	uint64_t audioBufferSize = audioDataIn->GetAudioStream()->GetSize();
	uint8_t* audioBuffer = new uint8_t[(uint32_t)audioBufferSize];
	uint64_t numBytesRead = audioDataIn->GetAudioStream()->ReadBytesFromStream(audioBuffer, audioBufferSize);
	assert(numBytesRead == audioBufferSize);

	WaveForm waveForm;
	waveForm.ConvertFromAudioBuffer(audioDataIn->GetFormat(), audioBuffer, audioBufferSize, 0);

	::memcpy(audioBuffer, 0, (size_t)audioBufferSize);

	waveForm.ConvertToAudioBuffer(audioDataIn->GetFormat(), audioBuffer, audioBufferSize, 0);

	AudioData* audioDataOut = new AudioData();
	uint64_t numBytesWritten = audioDataOut->GetAudioStream()->WriteBytesToStream(audioBuffer, audioBufferSize);
	assert(numBytesWritten == audioBufferSize);

	FileOutputStream outputStream("TestData/TestAudio1_copy.wav");
	bool savedWave = waveFormat.WriteToStream(outputStream, audioDataOut, error);
	assert(savedWave);

	delete audioDataIn;
	delete audioDataOut;
	delete[] audioBuffer;

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
	while (audioDataIn->GetAudioStream()->CanRead())
	{
		uint8_t byte = 0;
		audioDataIn->GetAudioStream()->ReadBytesFromStream(&byte, 1);
		audioDataOut->GetAudioStream()->WriteBytesToStream(&byte, 1);
	}

	FileOutputStream outputStream("TestData/TestAudio1_copy.wav");
	bool savedWave = waveFormat.WriteToStream(outputStream, audioDataOut, error);
	assert(savedWave);

	delete audioDataIn;
	delete audioDataOut;

	return true;
}