#pragma once

#include "MidiFileFormat.h"
#include "WaveFileFormat.h"
#include "SDLAudio.h"
#include "Error.h"
#include "Timer.h"
#include "CmdLineParser.h"
#include "MidiMsgLogDestination.h"

bool PlayMidiData(AudioDataLib::MidiData* midiData, bool logMidiMessages, AudioDataLib::Error& error);
bool PlayAudioData(AudioDataLib::AudioData* audioData, CmdLineParser& parser, AudioDataLib::Error& error);
bool MixAudio(const std::vector<std::string>& sourceFileArray, const std::string& destinationFile, AudioDataLib::Error& error);
bool DumpInfo(const std::string& filePath, bool csv, AudioDataLib::Error& error);
bool UnpackSoundFont(const std::string& filePath, AudioDataLib::Error& error);
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