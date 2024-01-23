# CMakeLists.txt file the AudioDataLib library.

file(GLOB_RECURSE LIBRARY_SOURCES CONFIGURE_DEPENDS
    "*.cpp"
    "*.h"
)

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
    "."
)