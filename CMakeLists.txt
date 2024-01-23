# CMakeLists.txt for the AudioDataLib project.

cmake_minimum_required(VERSION 3.28)
project(AudioDataLib)

add_subdirectory(Source/Library)
add_subdirectory(Source/ThirdParty/RtMidi)
add_subdirectory(Source/Tools/AudioDataTool)
#add_subdirectory(Source/Tools/AudioSignalAnalysis)