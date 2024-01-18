#include "CmdLineParser.h"

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

	if (parser.ArgGiven("--play"))
	{
		//...
	}

	return 0;
}
