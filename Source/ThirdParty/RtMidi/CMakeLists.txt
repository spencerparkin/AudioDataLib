# CMakeLists.txt file for RtMidi library.

set(RTMIDI_SOURCES
    RtMidi.cpp
    RtMidi.h
)

source_group("Sources" TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${RTMIDI_SOURCES})

add_library(RtMidi STATIC
    ${RTMIDI_SOURCES}
)

if(WIN32)
    target_compile_definitions(RtMidi PUBLIC __WINDOWS_MM__)
    target_link_libraries(RtMidi PUBLIC Winmm)
endif()

target_include_directories(RtMidi PUBLIC ".")

target_compile_features(RtMidi PUBLIC cxx_std_17)