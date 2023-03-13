//
// Created by Yuval Zilber on 10/03/2023.
//

#ifndef EXTRACT_SUBTITLES_2_NULLBUFFER_H
#define EXTRACT_SUBTITLES_2_NULLBUFFER_H

#include <iostream>
#include <string>

namespace logger {

    class NullBuffer : public std::streambuf {

    private:
        bool last_bl = true;
        std::string prefix;
        FILE* log_file;
    public:
        explicit NullBuffer(std::string pre);
        std::streamsize xsputn(const char *s, std::streamsize n) override;
        int_type overflow(int_type c) override;
    };

} // logger

#endif //EXTRACT_SUBTITLES_2_NULLBUFFER_H
