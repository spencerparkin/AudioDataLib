# CMakeLists.txt for the AudioDataLib project.

cmake_minimum_required(VERSION 3.27)
project(AudioDataLib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Source/Library)
add_subdirectory(Source/ThirdParty/RtMidi)
add_subdirectory(Source/Tools/AudioDataTool)
add_subdirectory(Source/Tools/AudioSignalAnalysis)