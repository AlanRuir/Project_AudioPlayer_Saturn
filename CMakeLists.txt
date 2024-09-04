cmake_minimum_required(VERSION 3.5)

project(AudioPlayer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Qml Quick Multimedia Widgets REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(CUDA REQUIRED)
find_package(yaml-cpp REQUIRED)

add_executable(AudioPlayer 
    src/main.cpp 
    src/audio_display_widget.cpp
    src/audio_player_controller.cpp
    src/main.qrc)

target_include_directories(AudioPlayer PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(AudioPlayer PRIVATE Qt5::Core Qt5::Gui Qt5::Qml Qt5::Quick Qt5::Multimedia Qt5::Widgets avcodec avformat avutil swscale yaml-cpp)