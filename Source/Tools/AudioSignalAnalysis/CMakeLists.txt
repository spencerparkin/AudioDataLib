# CMakeLists.txt file for AudioSignalAnalsys application.

set(APP_SOURCES
    App.cpp
    App.h
    Audio.cpp
    Audio.h
    AudioListControl.cpp
    AudioListControl.h
    Canvas.cpp
    Canvas.h
    Frame.cpp
    Frame.h
    Math2D.cpp
    Math2D.h
)

set(WX_WIDGETS_ROOT "E:/wxWidgets")

if(WIN32)
    add_executable(AudioSignalAnalysis WIN32 ${APP_SOURCES})
    target_compile_definitions(AudioSignalAnalysis PUBLIC __WXMSW__ _UNICODE)
else()
    add_executable(AudioSignalAnalysis ${APP_SOURCES})
endif()

target_link_libraries(AudioSignalAnalysis PRIVATE
    AudioDataLib
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_link_directories(AudioSignalAnalysis PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_x64_lib"
    )
else()
    target_link_directories(AudioSignalAnalysis PRIVATE
        "${WX_WIDGETS_ROOT}/lib/vc_lib"
    )
endif()

target_include_directories(AudioSignalAnalysis PRIVATE
    "${WX_WIDGETS_ROOT}/include"
    "${WX_WIDGETS_ROOT}/include/msvc"
)
