//
// Created by yuvalzilber on 5/17/23.
//

#include "base_utils.h"

namespace base_utils {
    std::string timeStamp(const char *format) {
        time_t t = time(nullptr);   // get time now
        struct tm *now = localtime(&t);
        char buffer[80];
        strftime(buffer, 80, format, now);
        std::string time_stamp = buffer;
        return time_stamp;
    }

    std::string logFilename(const char *base_name) {
        std::string s = base_name;
        return "logs/" + s + ".log";
    }

    void tolower(std::string &s) {
        transform(s.begin(), s.end(), s.begin(), ::tolower);
    }
}