//
// Created by Yuval Zilber on 10/03/2023.
//

#include <experimental/socket>
#include "NullBuffer.h"

namespace logger {

    NullBuffer::NullBuffer(const std::string &pre, const std::string &filename) :
            std::streambuf(),
            last_bl(true),
            prefix("[" + base_utils::timeStamp() + "] " + "[" + pre + "] "),
//            log_file(fopen(filename.c_str(), "w+")),
            log_file(filename) {
        FILE *f = fopen(filename.c_str(), "w+");
        if (!filename.empty() && !f)
            cerr << "Couldn't open file '" + filename + "'";
        else if(f)
            fclose(f);
    }


    std::streamsize NullBuffer::xsputn(const char *s, std::streamsize n) {
        string s2 = s;
        size_t rn = n;
        std::streamsize written = 0;
        if (last_bl) {
            s2 = prefix + s;
            rn += prefix.length();
        }

        if (debug_mode) {
            if (old_buf)
                old_buf->sputn(s2.c_str(), (int) rn);
            else
                fprintf(stderr, "%.*s", (int) rn, s2.c_str());
            cerr.flush();
            cout.flush();
        }
        FILE *f = fopen(log_file.c_str(), "a");

        if (f) {
            fprintf(f, "%.*s", (int) rn, s2.c_str());
            fflush(f);
            std::fclose(f);
        }
        std::string br = "\n\r";
        last_bl = (br.contains(s[n - 1]));
        return n;
    }

    NullBuffer::int_type NullBuffer::overflow(int_type c) {
        if (!streambuf::traits_type::eq_int_type(c, streambuf::traits_type::eof())) {
            char_type ch = traits_type::to_char_type(c);
            if (debug_mode) {
                if (old_buf) {
                    old_buf->sputc(ch);
                } else {
                    std::cerr << ch;
                }
                cerr.flush();
                cout.flush();
            }
            FILE *f = fopen(log_file.c_str(), "a");

            if (f) {
                fprintf(f, "%c", ch);
                fflush(f);
                fclose(f);
            }
            std::string br = "\n\r";
            last_bl = (br.contains(ch));
        }

        return c;
    }

}
