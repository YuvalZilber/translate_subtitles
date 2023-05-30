//
// Created by Yuval Zilber on 09/03/2023.
//

#ifndef EXTRACT_SUBTITLES_2_MKVFILE_H
#define EXTRACT_SUBTITLES_2_MKVFILE_H
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include "consts.h"
#include "utils.h"
#include "font.h"
#include <fstream>
#include <cstdlib>
#include <cstdlib>
#include <wait.h>
#include <sys/wait.h>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else

#include <unistd.h>

#define GetCurrentDir getcwd
#endif
using namespace std;
namespace fs = filesystem;

class MkvFile {
private:
    Path filepath_;
    Path sub_file_;
    string getTrackNum();
    Path mkvExtract(const string &si);
    static string getDigitsOfChoice(const string& choice);
public:
    MkvFile(Path& filepath);
    Path chooseSubtitleFile();
    static string longestDialog(Path &p);
};


#endif //EXTRACT_SUBTITLES_2_MKVFILE_H
