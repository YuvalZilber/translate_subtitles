//
// Created by Yuval Zilber on 09/03/2023.
//

#ifndef EXTRACT_SUBTITLES_2_CONSTS_H
#define EXTRACT_SUBTITLES_2_CONSTS_H

#include <iostream>
#include "Logger.h"
#include <filesystem>

using namespace std;
using namespace logger;
namespace fs = filesystem;

#define Path fs::path
#define READ 0
#define WRITE 1
#define flip_punctuation true
extern bool debug_mode;
extern Logger debug;
extern Logger CommandsLogger;
extern Path output;
extern string punctuation;
extern char* vlc_interface_module;

#endif //EXTRACT_SUBTITLES_2_CONSTS_H
