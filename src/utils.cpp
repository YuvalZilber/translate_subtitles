//
// Created by Yuval Zilber on 09/02/2023.
//

#include <wait.h>
#include <unistd.h>
#include <cstdio>  /* defines FILENAME_MAX */
#include <thread>
#include <array>
#include <chrono>
#include <regex>
#include "../headers/MkvFile.h"
#include "../headers/font.h"
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include "../headers/utils.h"

#define ever (;;)
using namespace std;
namespace utils {
    map<int, FILE *> files;

//region split
    vector<string> split(const string &s, const string &delimiter) {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        string token;
        vector<string> res;

        while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
            token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back(token);
        }

        res.push_back(s.substr(pos_start));
        return res;
    }

    [[maybe_unused]] vector<string> split(const string &s, int limit, const string &delimiter) {
        if (limit > 0) return lsplit(s, limit, delimiter);
        if (limit < 0) return rsplit(s, -limit, delimiter);
        return split(s, delimiter);
    }

    vector<string> rsplit(const string &s, size_t limit, const string &delimiter) {
        size_t pos_start, pos_end = s.length(), delim_len = delimiter.length();
        string token;
        vector<string> res;

        while ((res.size() < limit - 1) &&
               ((pos_start = s.rfind(delimiter, pos_end - delim_len) + delim_len) != string::npos)) {
            token = s.substr(pos_start, pos_end - pos_start);
            pos_end = pos_start - delim_len;
            res.push_back(token);
        }
        res.push_back(s.substr(0, pos_end));
        std::reverse(res.begin(), res.end());
        return res;
    }

    vector<string> lsplit(const string &s, size_t limit, const string &delimiter) {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        string token;
        vector<string> res;

        while ((res.size() < limit - 1) && ((pos_end = s.find(delimiter, pos_start)) != string::npos)) {
            token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            res.push_back(token);
        }

        res.push_back(s.substr(pos_start));
        return res;
    }

//endregion

    bool replace(string &str, const string &from, const string &to) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    int unblock(int fd, int flags = -1) {
        if (flags == -1)
            flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        return flags;
    }

    int block(int fd, int flags = -1) {
        if (flags == -1)
            flags = fcntl(fd, F_GETFL);
        fcntl(fd, F_SETFL, flags & !O_NONBLOCK);
        return flags;
    }

    void BlockRead(int fd) {
        vector<int> fds({fd});
        BlockRead(fds);
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    void BlockRead(const vector<int> &fds) {
        for ever {
            for (auto fd: fds)
                MyFlush(fd, "block");
        }
    }

#pragma clang diagnostic pop

    string MyFlush(int fd, const string &prefix, bool do_block) {
        ostringstream all;
        string line;
        auto process_line = [&all, &prefix](const string &s) {
            if (s != "\3" && s != "\3\n") {
                all << s << "\n";
                if (!prefix.empty())
                    cout << "[" << prefix << "] " << s << endl;
            }
        };

        int flags = -1;
        if (do_block) {
            flags = block(fd);
            process_line(getUnboundedLine(fd));
        }

        unblock(fd, flags);
        while ((line = getUnboundedLine(fd)) != "\3") {
            process_line(line);
        }
        fcntl(fd, F_SETFL, flags); // restore
        return all.str();
    }

    string getUnboundedLine(FILE *file) {
        char *buffer = nullptr;
        size_t line_cap = 0;
        int k = (int) getline(&buffer, &line_cap, file);

        if (k == -1) {

            free(buffer);
            return "\3";
        }
        string line;
        if (buffer != nullptr) {
            line = buffer;
            free(buffer);
        }
        while (line.ends_with("\n"))
            line = line.substr(0, line.length() - 1);
        return line;
    }

    string getUnboundedLine(int fd) {
        FILE *file = fdopen(fd, "r");
        string res = getUnboundedLine(file);
        return res;
    }

    FILE *OpenFile(int fd) {
        auto iter = files.lower_bound(fd);
        if (iter == files.end() || fd < iter->first) {    // not found
            FILE *file = fdopen(fd, "r");
            files.insert(iter, make_pair(fd, file));     // hinted insertion
        }
        return files[fd];
    }

    template<size_t N>
    char *TrimStart(char *sp, array<char *, N> &toTrim) {
        size_t lengths[N];
        for (int i = 0; i < N; ++i) {
            lengths[i] = strlen(toTrim[i]);
        }

        bool started = true;
        size_t s_len = strlen(sp);
        while (started) {
            started = false;
            for (int i = 0; i < N; ++i) {
                size_t ttLength = lengths[i];
                while (ttLength <= s_len && strncmp(sp, toTrim[i], ttLength) == 0) {
                    started = true;
                    sp += ttLength;
                    s_len -= ttLength;
                }
            }
        }
        return sp;
    }

    template<size_t N>
    void TrimStart(string &s, array<string, N> &toTrim) {
        char *cs = new char[s.length()];
        strcpy(cs, s.c_str());

        array<char *, N> &toTrimC{};
        for (int i = 0; i < N; ++i) {
            toTrimC[i] = new char[toTrim[i].length()];
            strcpy(toTrimC[i], toTrim[i]);
        }
        s = TrimStart(cs, toTrimC);
        delete[] cs;
        for (int i = 0; i < N; ++i) {
            delete[] toTrimC[i];
        }

    }

    template<size_t N>
    void TrimEnd(string &s, array<string, N> &toTrim) {
        size_t lengths[N];
        for (string &trim: toTrim)
            std::reverse(trim.begin(), trim.end());

        std::reverse(s.begin(), s.end());

        TrimStart(s, toTrim);
        std::reverse(s.begin(), s.end());
    }

    template<size_t N>
    void trim(string &s, array<string, N> &toTrim) {
        TrimStart(s, toTrim);
        TrimEnd(s, toTrim);
    }

    template<class T>
    size_t length(T *arr[]) {
        size_t length = 0;
        for (; *arr != nullptr; arr++) {
            length++;
        }
        return length;
    }


    void load(size_t dots, size_t max_dots) {
        cout << "\rloading";
        for (size_t i = 0; i < max_dots; i++)
            cout << (i < dots ? "." : " ");
        fflush(stdout);
    }

    void EndLoad(size_t max_dots) {
        cout << "\r        ";
        for (size_t i = 0; i < max_dots; i++)
            cout << " ";
        cout << endl;
        fflush(stdout);
    }

    int to_int(const string &c) {
        //debug << "try to parse '" << c << "'" << endl;
        fflush(stdout);
        return stoi(c);
    }

    regex GetPatternByTitle(vector<string> &format, const string &title) {
        int text_index = (int) find_index(format, title);
        if (text_index < 0)
            return {};
        string c = ".";
        string parts_after = to_string(format.size() - text_index - 1);
        if (strcasecmp(title.c_str(), "text") != 0) {
            c = "[^,]";
            parts_after += ",";
        }

        string pattern =
                "^(Dialogue: ([^,]*,){" + to_string(text_index) + "})(" + c + "*)((,[^,]*){" + parts_after + "})$";
        return regex(pattern);
    }

    size_t find_index(vector<string> &v, const string &element) {
        unsigned long index = find(v.begin(), v.end(), element) - v.begin();
        if (index >= v.size())
            return -1;
        return index;
    }

    smatch GetRegex(const string &line, const regex &text_pattern) {
        smatch sm_text;
        regex_match(line, sm_text, text_pattern);
        return sm_text;
    }

    void error(const string &msg, int code) {
        cerr << "[ERR] " << msg << endl;
        cout << "[ERR] " << msg << endl;
        if (code != 0)
            exit(code);
    }

    bool endsWithPunctuation(const string &line) {

        return line.find_last_of(punctuation, line.length()) == line.length() - 1;
    }

    string getPunctuationAtEnd(const string &line) {
        size_t pos = line.length();
        while (pos >0 && punctuation.contains(line[pos-1]))
            pos--;
        return line.substr(pos);

    }
}
