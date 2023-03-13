//
// Created by Yuval Zilber on 10/03/2023.
//

#include "../headers/NullBuffer.h"
namespace logger {

    NullBuffer::NullBuffer(std::string pre) : std::streambuf(), prefix(std::move(pre)), log_file(){

    }

    std::streamsize NullBuffer::xsputn(const char *s, std::streamsize n) {

        if (last_bl)
            std::cerr << "[" << prefix << "] ";
        std::cerr << s;
        last_bl = (s[n - 1] == '\n');
        return n;
    }

    NullBuffer::int_type NullBuffer::overflow(int_type c) {
        if (c == traits_type::eof()) {
            return traits_type::eof();
        }
        else {
            char_type ch = traits_type::to_char_type(c);
            std::cerr << ch;
            last_bl = (ch == '\n');
            return c;
        }
    }

}
