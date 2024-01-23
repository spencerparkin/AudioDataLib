# CMakeLists.txt file for RtMidi library.

set(RTMIDI_SOURCES
    RtMidi.cpp
    RtMidi.h
)

add_library(RtMidi STATIC
    ${RTMIDI_SOURCES}
)

if(WIN32)
    target_compile_definitions(RtMidi PUBLIC __WINDOWS_MM__)
endif()