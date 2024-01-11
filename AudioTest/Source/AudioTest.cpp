#include "WaveFileFormat.h"
#include "MidiFileFormat.h"
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
	//TestAudioConvertFormat();
	TestMidiSongLength();

	return 0;
}

bool TestMidiSongLength()
{
	FileInputStream inputStream("TestData/GGMOZH.MID");

	FileData* fileData = nullptr;
	std::string error;
	MidiFileFormat midiFileFormat;
	midiFileFormat.ReadFromStream(inputStream, fileData, error);

	MidiData* midiData = dynamic_cast<MidiData*>(fileData);

	for (uint32_t i = 1; i < midiData->GetNumTracks(); i++)
	{
		double timeSeconds = 0.0;
		midiData->CalculateTrackLengthInSeconds(1, timeSeconds, error);
		printf("Track %d: %f seconds\n", i, timeSeconds);
	}

	MidiData::Destroy(midiData);
	return true;
}

bool TestAudioSink()
{
	bool success = false;
	std::string error;

	WaveFileFormat waveFileFormat;

	FileData* fileDataA = nullptr;
	FileData* fileDataB = nullptr;

	FileInputStream inputStreamA("TestData/TestAudio1_converted.wav");
	FileInputStream inputStreamB("TestData/TestAudio2.wav");
	FileOutputStream outputStream("TestData/TestAudioResult.wav");

	do
	{
		if (!waveFileFormat.ReadFromStream(inputStreamA, fileDataA, error))
			break;

		if (!waveFileFormat.ReadFromStream(inputStreamB, fileDataB, error))
			break;

		auto audioDataA = dynamic_cast<AudioData*>(fileDataA);
		auto audioDataB = dynamic_cast<AudioData*>(fileDataB);

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

	delete fileDataA;
	delete fileDataB;

	return success;
}

bool TestWaveForm()
{
	WaveFileFormat waveFileFormat;

	FileInputStream inputStream("TestData/TestAudio1.wav");

	FileData* fileData = nullptr;
	std::string error;
	bool loadedWave = waveFileFormat.ReadFromStream(inputStream, fileData, error);
	assert(loadedWave);

	auto audioDataIn = dynamic_cast<AudioData*>(fileData);

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

	FileInputStream inputStream("TestData/TestAudio1.wav");

	FileData* fileData = nullptr;
	std::string error;
	bool loadedWave = waveFileFormat.ReadFromStream(inputStream, fileData, error);
	assert(loadedWave);

	auto audioDataIn = dynamic_cast<AudioData*>(fileData);

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

	FileData* fileDataA = nullptr;
	FileData* fileDataB = nullptr;

	FileInputStream inputStreamA("TestData/TestAudio1.wav");
	FileInputStream inputStreamB("TestData/TestAudio2.wav");

	std::string error;

	waveFileFormat.ReadFromStream(inputStreamA, fileDataA, error);
	waveFileFormat.ReadFromStream(inputStreamB, fileDataB, error);

	auto audioDataA = dynamic_cast<AudioData*>(fileDataA);
	auto audioDataB = dynamic_cast<AudioData*>(fileDataB);

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

	FileInputStream inputStream("TestData/TestAudio1.wav");

	FileData* fileData = nullptr;
	std::string error;
	bool loadedWave = waveFileFormat.ReadFromStream(inputStream, fileData, error);
	assert(loadedWave);

	auto audioDataIn = dynamic_cast<AudioData*>(fileData);

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