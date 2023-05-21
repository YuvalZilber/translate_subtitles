//
// Created by yuvalzilber on 5/17/23.
//

#ifndef EXTRACT_SUBTITLES_2_BASE_UTILS_H
#define EXTRACT_SUBTITLES_2_BASE_UTILS_H

#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include <iostream>
#include <string>
#include <regex>


#include <wait.h>
#include <unistd.h>
#include <cstdio>  /* defines FILENAME_MAX */
#include <thread>
#include <array>
#include <chrono>
#include <regex>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

namespace base_utils {
    std::string timeStamp(const char*format="%d-%m-%Y %H:%M:%S") ;
    std::string logFilename(const char*base_name);
}


#endif //EXTRACT_SUBTITLES_2_BASE_UTILS_H
