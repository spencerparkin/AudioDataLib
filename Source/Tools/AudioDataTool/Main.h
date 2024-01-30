#pragma once

#include "MidiFileFormat.h"
#include "WaveFileFormat.h"
#include "RtMidiPlayer.h"
#include "SDLAudioPlayer.h"
#include "Error.h"
#include "Timer.h"
#include "CmdLineParser.h"

bool PlayMidiData(AudioDataLib::MidiData* midiData, AudioDataLib::Error& error);
bool PlayAudioData(AudioDataLib::AudioData* audioData, AudioDataLib::Error& error);
bool MixAudio(const std::vector<std::string>& sourceFileArray, const std::string& destinationFile, AudioDataLib::Error& error);
bool DumpInfo(const std::string& filePath, bool csv, AudioDataLib::Error& error);
bool UnpackSoundFont(const std::string& filePath, AudioDataLib::Error& error);
bool PlayWithKeyboard(CmdLineParser& parser, AudioDataLib::Error& error);