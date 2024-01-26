# CMakeLists.txt file the AudioDataLib library.

#file(GLOB_RECURSE LIBRARY_SOURCES CONFIGURE_DEPENDS
#    "*.cpp"
#    "*.h"
#)

set(LIBRARY_SOURCES
    FileDatas/AudioData.cpp
    FileDatas/AudioData.h
    FileDatas/FileData.cpp
    FileDatas/FileData.h
    FileDatas/MidiData.cpp
    FileDatas/MidiData.h
    FileDatas/SoundFontData.cpp
    FileDatas/SoundFontData.h
    FileFormats/FileFormat.cpp
    FileFormats/FileFormat.h
    FileFormats/MidiFileFormat.cpp
    FileFormats/MidiFileFormat.h
    FileFormats/SoundFontFormat.cpp
    FileFormats/SoundFontFormat.h
    FileFormats/WaveFileFormat.cpp
    FileFormats/WaveFileFormat.h
    Math/ComplexNumber.cpp
    Math/ComplexNumber.h
    Math/ComplexVector.cpp
    Math/ComplexVector.h
    MIDI/LogSynth.cpp
    MIDI/LogSynth.h
    MIDI/MidiPlayer.cpp
    MIDI/MidiPlayer.h
    MIDI/MidiSynth.cpp
    MIDI/MidiSynth.h
    MIDI/RecorderSynth.cpp
    MIDI/RecorderSynth.h
    MIDI/SimpleSynth.cpp
    MIDI/SimpleSynth.h
    MIDI/WaveTableSynth.cpp
    MIDI/WaveTableSynth.h
    MIDI/SubtractiveSynth.cpp
    MIDI/SubtractiveSynth.h
    MIDI/SampleBasedSynth.cpp
    MIDI/SampleBasedSynth.h
    SynthModules/OscillatorModule.cpp
    SynthModules/OscillatorModule.h
    SynthModules/MixerModule.cpp
    SynthModules/MixerModule.h
    SynthModules/SynthModule.cpp
    SynthModules/SynthModule.h
    SynthModules/FilterModule.cpp
    SynthModules/FilterModule.h
    SynthModules/SoundFontModule.cpp
    SynthModules/SoundFontModule.h
    SynthModules/ArticulationModule.cpp
    SynthModules/ArticulationModule.h
    AudioSink.cpp
    AudioSink.h
    ByteStream.cpp
    ByteStream.h
    ByteSwapper.cpp
    ByteSwapper.h
    ChunkParser.cpp
    ChunkParser.h
    Common.h
    Error.cpp
    Error.h
    FrequencyGraph.cpp
    FrequencyGraph.h
    Mutex.cpp
    Mutex.h
    Timer.cpp
    Timer.h
    WaveForm.cpp
    WaveForm.h
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LIBRARY_SOURCES})

add_library(AudioDataLib SHARED
    ${LIBRARY_SOURCES}
)

target_compile_definitions(AudioDataLib PRIVATE
    AUDIO_DATA_LIB_EXPORT
)

target_include_directories(AudioDataLib PUBLIC
    "FileDatas"
    "FileFormats"
    "Math"
    "MIDI"
    "SynthModules"
    "."
)