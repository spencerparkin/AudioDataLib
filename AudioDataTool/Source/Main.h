#pragma once

#include "MidiFileFormat.h"
#include "WaveFileFormat.h"
#include "RtMidiPlayer.h"
#include "Error.h"
#include "Timer.h"

bool PlayMidiFile(AudioDataLib::MidiData* midiData, AudioDataLib::Error& error);
bool PlayAudioFile(AudioDataLib::AudioData* audioData, AudioDataLib::Error& error);