//
// Created by Yuval Zilber on 09/03/2023.
//

#ifndef EXTRACT_SUBTITLES_2_CONSTS_H
#define EXTRACT_SUBTITLES_2_CONSTS_H
#include <iostream>
#include "Logger.h"

using namespace std;
using namespace logger;
namespace fs = __fs::filesystem;

#define Path fs::path
#define READ 0
#define WRITE 1
#define flip_punctuation true

bool debug_mode;
Logger debug("DEBUG");
Logger CommandsLogger("CMD");
Path output;

#endif //EXTRACT_SUBTITLES_2_CONSTS_H
