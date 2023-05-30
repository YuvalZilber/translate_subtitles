cmake_minimum_required(VERSION 3.23)
project(extract_subtitles_2)

set(CMAKE_CXX_STANDARD 23)
include_directories(include)
add_executable(extract_subtitles_2 src/main.cpp src/utils.cpp include/utils.h src/font.cpp include/font.h src/MkvFile.cpp include/MkvFile.h include/consts.h src/consts.cpp include/base_consts.h src/base_consts.cpp src/Logger.cpp include/Logger.h src/NullBuffer.cpp include/NullBuffer.h src/InteractiveShell.cpp include/InteractiveShell.h src/Vlc.cpp include/Vlc.h src/base_utils.cpp include/base_utils.h src/MainOptions.cpp include/MainOptions.h src/TranslatorLtrToRtl.cpp include/TranslatorLtrToRtl.h)
