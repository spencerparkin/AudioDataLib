#include "CmdLineParser.h"
#include "Main.h"
#include <memory>

using namespace AudioDataLib;

int main(int argc, char** argv)
{
	CmdLineParser parser;
	parser.SetSynopsis("This is just a basic command-line tool meant to exercise the AudioDataLib shared library.");

	parser.RegisterArg("play", 1, "Play the given file.  This can be a WAV or MIDI file.");
	parser.RegisterArg("keyboard", 1, "Receive MIDI input from the given MIDI keyboard device.");
	parser.RegisterArg("synth", 0, "Synthesize MIDI input to the sound-card.");
	parser.RegisterArg("record_midi", 1, "Record MIDI input to the given MIDI file.");
	parser.RegisterArg("record_wave", 1, "Record microphone input to the given WAV file.");
	parser.RegisterArg("mix", 3, "Mix the two given WAV files into a single WAV file, the third given output file.");
	parser.RegisterArg("concat", 3, "Concatinate the two given WAV files into a single WAV file, the third given output file.");
	parser.RegisterArg("trim_start", 2, "Trim the start of the given WAV file by the given number of seconds.  This modifies the given file.");
	parser.RegisterArg("trim_end", 2, "Truncate the given WAV file by the given number of seconds.  This modifies the given file.");
	
	std::string error;
	if (!parser.Parse(argc, argv, error))
	{
		fprintf(stderr, ("Error: " + error).c_str());
		parser.PrintUsage(stderr);
		return -1;
	}

	if (parser.ArgGiven("play"))
	{
		const std::string& filePath = parser.GetArgValue("play", 0);
		FileFormat* fileFormat = FileFormat::CreateForFile(filePath);
		if (!fileFormat)
		{
			fprintf(stderr, "File format for file \"%s\" not recognized.", filePath.c_str());
			return -1;
		}

		FileInputStream inputStream(filePath.c_str());
		if (!inputStream.IsOpen())
		{
			fprintf(stderr, "Could not open file: %s", filePath.c_str());
			delete fileFormat;
			return -1;
		}

		Error error;
		FileData* fileData = nullptr;
		if (!fileFormat->ReadFromStream(inputStream, fileData, error))
		{
			fprintf(stderr, ("Error: " + error.GetMessage()).c_str());
			delete fileFormat;
			return -1;
		}

		int retCode = 0;

		auto midiData = dynamic_cast<MidiData*>(fileData);
		auto audioData = dynamic_cast<AudioData*>(fileData);

		if (midiData)
		{
			if (!PlayMidiFile(midiData, error))
				retCode = -1;
		}
		else if (audioData)
		{
			if (!PlayAudioFile(audioData, error))
				retCode = -1;
		}
		else
		{
			error.Add("Failed to cast file data!");
			retCode = -1;
		}

		if (error)
			fprintf(stderr, "Error: %s", error.GetMessage().c_str());

		delete fileData;
		delete fileFormat;
		return retCode;
	}

	return 0;
}

bool PlayMidiFile(AudioDataLib::MidiData* midiData, AudioDataLib::Error& error)
{
	SystemClockTimer timer;
	RtMidiPlayer player(&timer);

	player.SetMidiData(midiData);
	std::set<uint32_t> playableTrackSet;

	player.GetSimultaneouslyPlayableTracks(playableTrackSet);
	player.SetTimeSeconds(0.0);
	if (!player.BeginPlayback(playableTrackSet, error))
		return false;

	while (!player.NoMoreToPlay())
	{
		if (!player.ManagePlayback(error))
			return false;

		// TODO: Maybe watch for key-presses here?  Should be able to pause, rewind, fastforward, and just stop.
		// TODO: Print playback time periodically, but can you do it without introducing a newline each time?
	}

	if (!player.EndPlayback(error))
		return false;

	return true;
}

bool PlayAudioFile(AudioDataLib::AudioData* audioData, AudioDataLib::Error& error)
{
	bool success = false;
	SDLAudioPlayer player;

	do
	{
		if (!player.Setup(error))
			break;

		if (!player.PlayAudio(audioData, error))
			break;

		while (player.IsPlayingSomething())
		{
			if (!player.ManagePlayback(error))
				break;

			// TODO: Maybe listen for key-presses here and let the user quit if desired?
		}

		if (error)
			break;

		success = true;
	} while (false);
	
	player.Shutdown(error);

	return success;
}