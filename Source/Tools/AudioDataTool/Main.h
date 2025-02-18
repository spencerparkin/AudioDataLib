#pragma once

#include "AudioDataLib/FileFormats/MidiFileFormat.h"
#include "AudioDataLib/FileFormats/WaveFileFormat.h"
#include "SDLAudio.h"
#include "AudioDataLib/ErrorSystem.h"
#include "AudioDataLib/Timer.h"
#include "CmdLineParser.h"
#include "AudioDataLib/MIDI/MidiMsgLogDestination.h"

bool PlayMidiData(AudioDataLib::MidiData* midiData, bool logMidiMessages);
bool PlayAudioData(AudioDataLib::AudioData* audioData, CmdLineParser& parser);
bool MixAudio(const std::vector<std::string>& sourceFileArray, const std::string& destinationFile);
bool DumpInfo(const std::string& filePath, bool csv);
bool Unpack(const std::string& filePath);
bool PlayWithKeyboard(CmdLineParser& parser);
bool AddReverb(const std::string& inFilePath, const std::string& outFilePath);

class StdoutLogDestination : public AudioDataLib::MidiMsgLogDestination
{
public:
	virtual void LogMessage(const std::string& logMessage) override
	{
		printf("MIDI LOG: %s\n", logMessage.c_str());
	}
};