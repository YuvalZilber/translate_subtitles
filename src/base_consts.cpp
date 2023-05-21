//
// Created by Yuval Zilber on 09/03/2023.
//

#include "../headers/base_consts.h"

using namespace std;

bool debug_mode = false;
Path output;
string punctuation = ",.!?:;\"\\/'-";

#if defined(__linux__) // Or #if __linux__
char *vlc_interface_module = (char *) "qt";
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

