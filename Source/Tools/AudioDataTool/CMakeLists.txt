# CMakeLists.txt for AudioDataTool program.

set(TOOL_SOURCES
    CmdLineParser.cpp
    CmdLineParser.h
    Keyboard.cpp
    Keyboard.h
    Main.cpp
    Main.h
    RtMidiPlayer.cpp
    RtMidiPlayer.h
    RtMidiSynth.cpp
    RtMidiSynth.h
    SDLAudioPlayer.cpp
    SDLAudioPlayer.h
)

set(SDL_ROOT "C:/ENG_DEV/driver/trunk/Main_8/apis/SDL2-2.28.5")

add_executable(AudioDataTool ${TOOL_SOURCES})

target_link_libraries(AudioDataTool PRIVATE
    AudioDataLib
    SDL2
    RtMidi
)

target_include_directories(AudioDataTool PUBLIC "${SDL_ROOT}/include")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(AudioDataTool PUBLIC "${SDL_ROOT}/lib/x64")
else()
    target_link_directories(AudioDataTool PUBLIC "$(SDL_ROOT}/lib/x86")
endif()