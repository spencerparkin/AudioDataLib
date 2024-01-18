#pragma once

#include "MidiFileFormat.h"
#include "WaveFileFormat.h"
#include "RtMidiPlayer.h"
#include "SDLAudioPlayer.h"
#include "Error.h"
#include "Timer.h"

bool PlayMidiData(AudioDataLib::MidiData* midiData, AudioDataLib::Error& error);
bool PlayAudioData(AudioDataLib::AudioData* audioData, AudioDataLib::Error& error);