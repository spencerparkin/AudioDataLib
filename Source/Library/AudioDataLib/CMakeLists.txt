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
    FileDatas/WaveTableData.cpp
    FileDatas/WaveTableData.h
    FileFormats/FileFormat.cpp
    FileFormats/FileFormat.h
    FileFormats/MidiFileFormat.cpp
    FileFormats/MidiFileFormat.h
    FileFormats/SoundFontFormat.cpp
    FileFormats/SoundFontFormat.h
    FileFormats/WaveFileFormat.cpp
    FileFormats/WaveFileFormat.h
    FileFormats/AiffFileFormat.cpp
    FileFormats/AiffFileFormat.h
    FileFormats/DownloadableSoundFormat.cpp
    FileFormats/DownloadableSoundFormat.h
    Codecs/Codec.cpp
    Codecs/Codec.h
    Codecs/ALawCodec.cpp
    Codecs/ALawCodec.h
    Codecs/uLawCodec.cpp
    Codecs/uLawCodec.h
    Math/ComplexNumber.cpp
    Math/ComplexNumber.h
    Math/ComplexVector.cpp
    Math/ComplexVector.h
    Math/Function.cpp
    Math/Function.h
    MIDI/MidiMsgLogDestination.cpp
    MIDI/MidiMsgLogDestination.h
    MIDI/MidiPlayer.cpp
    MIDI/MidiPlayer.h
    MIDI/MidiSynth.cpp
    MIDI/MidiSynth.h
    MIDI/MidiMsgSource.cpp
    MIDI/MidiMsgSource.h
    MIDI/MidiMsgDestination.cpp
    MIDI/MidiMsgDestination.h
    MIDI/MidiMsgRecorderDestination.cpp
    MIDI/MidiMsgRecorderDestination.h
    MIDI/SimpleSynth.cpp
    MIDI/SimpleSynth.h
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
    SynthModules/ArticulationModule.cpp
    SynthModules/ArticulationModule.h
    SynthModules/PitchShiftModule.cpp
    SynthModules/PitchShiftModule.h
    SynthModules/LoopedAudioModule.cpp
    SynthModules/LoopedAudioModule.h
    SynthModules/InterpolationModule.cpp
    SynthModules/InterpolationModule.h
    SynthModules/AttenuationModule.cpp
    SynthModules/AttenuationModule.h
    SynthModules/ReverbModule.cpp
    SynthModules/ReverbModule.h
    SynthModules/DuplicationModule.cpp
    SynthModules/DuplicationModule.h
    SynthModules/DelayModule.cpp
    SynthModules/DelayModule.h
    AudioSink.cpp
    AudioSink.h
    ByteStream.cpp
    ByteStream.h
    ByteSwapper.cpp
    ByteSwapper.h
    ChunkParser.cpp
    ChunkParser.h
    Common.h
    ErrorSystem.cpp
    ErrorSystem.h
    FrequencyGraph.cpp
    FrequencyGraph.h
    Mutex.cpp
    Mutex.h
    Timer.cpp
    Timer.h
    WaveForm.cpp
    WaveForm.h
    RecursiveFilter.cpp
    RecursiveFilter.h
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LIBRARY_SOURCES})

add_library(AudioDataLib SHARED
    ${LIBRARY_SOURCES}
)

target_compile_definitions(AudioDataLib PRIVATE
    AUDIO_DATA_LIB_EXPORT
)

target_include_directories(AudioDataLib PUBLIC
    ".."
)