#pragma once

#include "AudioDataLib/FileFormats/MidiFileFormat.h"
#include "AudioDataLib/FileFormats/WaveFileFormat.h"
#include "SDLAudio.h"
#include "AudioDataLib/Error.h"
#include "AudioDataLib/Timer.h"
#include "CmdLineParser.h"
#include "AudioDataLib/MIDI/MidiMsgLogDestination.h"

bool PlayMidiData(AudioDataLib::MidiData* midiData, bool logMidiMessages, AudioDataLib::Error& error);
bool PlayAudioData(AudioDataLib::AudioData* audioData, CmdLineParser& parser, AudioDataLib::Error& error);
bool MixAudio(const std::vector<std::string>& sourceFileArray, const std::string& destinationFile, AudioDataLib::Error& error);
bool DumpInfo(const std::string& filePath, bool csv, AudioDataLib::Error& error);
bool Unpack(const std::string& filePath, AudioDataLib::Error& error);
bool PlayWithKeyboard(CmdLineParser& parser, AudioDataLib::Error& error);
bool AddReverb(const std::string& inFilePath, const std::string& outFilePath, AudioDataLib::Error& error);

class StdoutLogDestination : public AudioDataLib::MidiMsgLogDestination
{
public:
	virtual void LogMessage(const std::string& logMessage) override
	{
		printf("MIDI LOG: %s\n", logMessage.c_str());
	}
};