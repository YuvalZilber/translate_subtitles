//
// Created by Yuval Zilber on 10/03/2023.
//

#ifndef EXTRACT_SUBTITLES_2_NULLBUFFER_H
#define EXTRACT_SUBTITLES_2_NULLBUFFER_H

#include <iostream>
#include <string>
#include "../headers/base_consts.h"
#include "../headers/base_utils.h"

namespace logger {
    class NullBuffer : public std::streambuf {

    private:
        bool last_bl = true;
        std::string prefix;
        std::string log_file;

        std::streambuf *old_buf;
    public:
        explicit NullBuffer(const std::string &pre, const std::string &filename);

        std::streamsize xsputn(const char *s, std::streamsize n) override;

        int_type overflow(int_type c) override;
    };

} // logger

#endif //EXTRACT_SUBTITLES_2_NULLBUFFER_H
