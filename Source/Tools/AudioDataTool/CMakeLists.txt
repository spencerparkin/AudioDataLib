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

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TOOL_SOURCES})

add_executable(AudioDataTool ${TOOL_SOURCES})

target_link_libraries(AudioDataTool PRIVATE
    AudioDataLib
    SDL2
    RtMidi
)

#set_target_properties(AudioDataTool PROPERTIES
#    LINK_OPTIONS "/DELAYLOAD:SDL2.dll"
#)

target_include_directories(AudioDataTool PUBLIC "${SDL_ROOT}/include")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(AudioDataTool PUBLIC "${SDL_ROOT}/lib/x64")
    set(AUGMENTED_PATH "PATH=%PATH%;${SDL_ROOT}/lib/x64")
else()
    target_link_directories(AudioDataTool PUBLIC "$(SDL_ROOT}/lib/x86")
    set(AUGMENTED_PATH "PATH=%PATH%;${SDL_ROOT}/lib/x86")
endif()

if(WIN32)
    set_target_properties(AudioDataTool PROPERTIES
        VS_DEBUGGER_ENVIRONMENT "${AUGMENTED_PATH}"
        VS_DEBUGGER_COMMAND_ARGUMENTS "--keyboard \"AKM320 0\" --synth simple --log_midi"
        VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/TestData"
    )
endif()