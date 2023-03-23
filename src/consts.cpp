//
// Created by Yuval Zilber on 09/03/2023.
//

#include "../headers/consts.h"
#include "../headers/Logger.h"

using namespace std;
using namespace logger;

bool debug_mode = false;
Logger debug("DEBUG");
Logger CommandsLogger("CMD");
Path output;
string punctuation = ",.!?:;\"\\/'";

#if defined(__linux__) // Or #if __linux__
    char* vlc_interface_module = (char*)"qt";
#elif __FreeBSD__
    char* vlc_interface_module = (char*)"qt";
#elif __ANDROID__
    char* vlc_interface_module = (char*)"qt";
#elif __APPLE__
    char* vlc_interface_module = (char*)"macosx";
#elif _WIN32
    char* vlc_interface_module = (char*)"qt";
#else
    char* vlc_interface_module = (char*)"qt";
#endif

