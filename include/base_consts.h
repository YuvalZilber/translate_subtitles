//
// Created by Yuval Zilber on 09/03/2023.
//

#ifndef EXTRACT_SUBTITLES_2_BASE_CONSTS_H
#define EXTRACT_SUBTITLES_2_BASE_CONSTS_H

#include <iostream>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

#define Path fs::path
#define READ 0
#define WRITE 1
#define flip_punctuation true
extern bool debug_mode;
extern Path output;
extern string punctuation;
extern char* vlc_interface_module;

#endif //EXTRACT_SUBTITLES_2_BASE_CONSTS_H
