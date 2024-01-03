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
	waveForm.ConvertFromAudioBuffer(audioDataIn->GetFormat(), audioDataIn->GetAudioBuffer(), audioDataIn->GetAudioBufferSize(), 0);

	AudioData* audioDataOut = new AudioData();
	audioDataOut->SetAudioBufferSize(audioDataIn->GetAudioBufferSize());

	waveForm.ConvertToAudioBuffer(audioDataIn->GetFormat(), audioDataOut->GetAudioBuffer(), audioDataOut->GetAudioBufferSize(), 0);

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

	FileOutputStream outputStream("TestData/TestAudio1_copy.wav");
	bool savedWave = waveFormat.WriteToStream(outputStream, audioDataOut, error);
	assert(savedWave);

	delete audioDataIn;
	delete audioDataOut;

	return true;
}