#include "WaveFormat.h"
#include "ByteStream.h"
#include "AudioSink.h"
#include <iostream>

using namespace AudioDataLib;

int main(int argc, char** argv)
{
	WaveFormat waveFormat;

	AudioData* audioDataA = nullptr;
	AudioData* audioDataB = nullptr;
	
	FileInputStream inputStreamA("TestData/TestAudio1.wav");
	FileInputStream inputStreamB("TestData/TestAudio2.wav");
	FileOutputStream outputStream("TestData/TestAudioResult.wav");

	std::string error;
	bool success = false;

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
			audioSink.MixAudio(1024);
		}

		if (!waveFormat.WriteToStream(outputStream, audioSink.GetAudioOutput(), error))
			break;

		success = true;

	} while (false);

	if (error.size() > 0)
		std::cout << error << std::endl;

	delete audioDataA;
	delete audioDataB;

	return success ? 0 : -1;
}