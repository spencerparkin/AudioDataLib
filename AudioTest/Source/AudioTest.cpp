#include "WaveFileFormat.h"
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
	//TestWaveFormAdd();
	//TestAudioSink();
	TestAudioConvertFormat();

	return 0;
}

bool TestAudioSink()
{
	bool success = false;
	std::string error;

	WaveFileFormat waveFileFormat;

	AudioData* audioDataA = nullptr;
	AudioData* audioDataB = nullptr;

	FileInputStream inputStreamA("TestData/TestAudio1_converted.wav");
	FileInputStream inputStreamB("TestData/TestAudio2.wav");
	FileOutputStream outputStream("TestData/TestAudioResult.wav");

	do
	{
		if (!waveFileFormat.ReadFromStream(inputStreamA, audioDataA, error))
			break;

		if (!waveFileFormat.ReadFromStream(inputStreamB, audioDataB, error))
			break;

		AudioSink audioSink(true);
		audioSink.SetAudioOutput(new AudioStream(audioDataA->GetFormat()));

		auto audioStreamA = new AudioStream(audioDataA);
		auto audioStreamB = new AudioStream(audioDataB);

		audioSink.AddAudioInput(audioStreamA);
		audioSink.AddAudioInput(audioStreamB);

		AudioStream mixedAudioStream(audioDataA->GetFormat());

		while (audioSink.GetAudioInputCount() > 0)
		{
			audioSink.GenerateAudio(1.0, 1.0);

			// Artificially drain the audio from the sink.  In practice, this
			// would be done by a callback for the audio device.
			while (audioSink.GetAudioOutput()->CanRead())
			{
				uint8_t byte = 0;
				audioSink.GetAudioOutput()->ReadBytesFromStream(&byte, 1);
				mixedAudioStream.WriteBytesToStream(&byte, 1);
			}
		}

		AudioData generatedAudioData;
		generatedAudioData.GetFormat() = mixedAudioStream.GetFormat();
		generatedAudioData.SetAudioBufferSize(mixedAudioStream.GetSize());
		mixedAudioStream.ReadBytesFromStream(generatedAudioData.GetAudioBuffer(), generatedAudioData.GetAudioBufferSize());

		if (!waveFileFormat.WriteToStream(outputStream, &generatedAudioData, error))
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
	WaveFileFormat waveFileFormat;

	AudioData* audioDataIn = nullptr;
	FileInputStream inputStream("TestData/TestAudio1.wav");

	std::string error;
	bool loadedWave = waveFileFormat.ReadFromStream(inputStream, audioDataIn, error);
	assert(loadedWave);

	WaveForm waveForm;
	waveForm.ConvertFromAudioBuffer(audioDataIn->GetFormat(), audioDataIn->GetAudioBuffer(), audioDataIn->GetAudioBufferSize(), 0, error);

	AudioData* audioDataOut = new AudioData();
	audioDataOut->SetAudioBufferSize(audioDataIn->GetAudioBufferSize());
	audioDataOut->GetFormat() = audioDataIn->GetFormat();

	waveForm.ConvertToAudioBuffer(audioDataOut->GetFormat(), audioDataOut->GetAudioBuffer(), audioDataOut->GetAudioBufferSize(), 0, error);

	FileOutputStream outputStream("TestData/TestAudio1_copy.wav");
	bool savedWave = waveFileFormat.WriteToStream(outputStream, audioDataOut, error);
	assert(savedWave);

	delete audioDataIn;
	delete audioDataOut;

	return true;
}

bool TestWaveFormat()
{
	WaveFileFormat waveFileFormat;

	AudioData* audioDataIn = nullptr;
	FileInputStream inputStream("TestData/TestAudio1.wav");

	std::string error;
	bool loadedWave = waveFileFormat.ReadFromStream(inputStream, audioDataIn, error);
	assert(loadedWave);

	AudioData* audioDataOut = new AudioData();
	audioDataOut->SetAudioBufferSize(audioDataIn->GetAudioBufferSize());
	::memcpy(audioDataOut->GetAudioBuffer(), audioDataIn->GetAudioBuffer(), (size_t)audioDataOut->GetAudioBufferSize());
	audioDataOut->GetFormat() = audioDataIn->GetFormat();

	FileOutputStream outputStream("TestData/TestAudio1_copy.wav");
	bool savedWave = waveFileFormat.WriteToStream(outputStream, audioDataOut, error);
	assert(savedWave);

	delete audioDataIn;
	delete audioDataOut;

	return true;
}

bool TestWaveFormAdd()
{
	WaveFileFormat waveFileFormat;

	AudioData* audioDataA = nullptr;
	AudioData* audioDataB = nullptr;

	FileInputStream inputStreamA("TestData/TestAudio1.wav");
	FileInputStream inputStreamB("TestData/TestAudio2.wav");

	std::string error;

	waveFileFormat.ReadFromStream(inputStreamA, audioDataA, error);
	waveFileFormat.ReadFromStream(inputStreamB, audioDataB, error);

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
	waveFileFormat.WriteToStream(outputStream, audioDataOut, error);

	delete audioDataA;
	delete audioDataB;
	delete audioDataOut;

	return true;
}

bool TestAudioConvertFormat()
{
	WaveFileFormat waveFileFormat;

	AudioData* audioDataIn = nullptr;
	FileInputStream inputStream("TestData/TestAudio1.wav");

	std::string error;
	bool loadedWave = waveFileFormat.ReadFromStream(inputStream, audioDataIn, error);
	assert(loadedWave);

	double clipLengthSeconds = audioDataIn->GetTimeSeconds();

	AudioData* audioDataOut = new AudioData();
	AudioData::Format& format = audioDataOut->GetFormat();
	format.bitsPerSample = 32;
	format.numChannels = 2;
	format.framesPerSecond = 48000;
	format.sampleType = AudioData::Format::FLOAT;

	uint64_t numBytes = format.BytesFromSeconds(clipLengthSeconds);
	audioDataOut->SetAudioBufferSize(numBytes);

	AudioSink audioSink(true);
	audioSink.AddAudioInput(new AudioStream(audioDataIn));
	audioSink.SetAudioOutput(new AudioStream(audioDataOut->GetFormat()));
	audioSink.GenerateAudio(clipLengthSeconds, 0.0);

	audioSink.GetAudioOutput()->ReadBytesFromStream(audioDataOut->GetAudioBuffer(), audioDataOut->GetAudioBufferSize());

	FileOutputStream outputStream("TestData/TestAudio1_converted.wav");
	bool savedWave = waveFileFormat.WriteToStream(outputStream, audioDataOut, error);
	assert(savedWave);

	delete audioDataIn;
	delete audioDataOut;

	return true;
}