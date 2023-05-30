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
#include "Vlc.h"
#include "MkvFile.h"
#include "font.h"
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include "utils.h"

#define ever (;;)


using namespace std;
namespace utils {
    using namespace base_utils;

    //region strings
    bool replace(string &str, const string &from, const string &to) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    //region split
    vector<string> split(const string &s, const string &delimiter, bool do_trim) {
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        string token;
        vector<string> res;

        while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
            token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            if (do_trim) {
                token = trim(token);
            }
            res.push_back(token);
        }

        string token_last = s.substr(pos_start);
        if (do_trim) {
            token_last = trim(token_last);
        }
        res.push_back(token_last);
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
    //region Trim
    inline std::string trim(std::string &str) {
        str.erase(str.find_last_not_of(' ') + 1);         //suffixing spaces
        str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
        return str;
    }

    string trimRegex(const string &s, const string &pattern) {
        string result = regex_replace(s, regex("(^" + pattern + ")|(" + pattern + "$)"), "");
        return result;
    }

    template<size_t N>
    char *trimStart(char *sp, array<char *, N> &toTrim) {
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
    void trimStart(string &s, array<string, N> &toTrim) {
        char *cs = new char[s.length()];
        strcpy(cs, s.c_str());

        array<char *, N> &toTrimC{};
        for (int i = 0; i < N; ++i) {
            toTrimC[i] = new char[toTrim[i].length()];
            strcpy(toTrimC[i], toTrim[i]);
        }
        s = trimStart(cs, toTrimC);
        delete[] cs;
        for (int i = 0; i < N; ++i) {
            delete[] toTrimC[i];
        }

    }

    template<size_t N>
    void trimEnd(string &s, array<string, N> &toTrim) {
        size_t lengths[N];
        for (string &trim: toTrim)
            std::reverse(trim.begin(), trim.end());

        std::reverse(s.begin(), s.end());

        trimStart(s, toTrim);
        std::reverse(s.begin(), s.end());
    }

    template<size_t N>
    void trim(string &s, array<string, N> &toTrim) {
        trimStart(s, toTrim);
        trimEnd(s, toTrim);
    }
    //endregion

    bool endsWithPunctuation(const string &line) {
//        return punctuation.find_first_of(line[line.length()-1])!=-1;
        return line.find_last_of(punctuation, line.length()) == line.length() - 1;
    }

    string getPunctuationAtEnd(const string &line) {
        size_t pos = line.length();
        while (pos > 0 && punctuation.contains(line[pos - 1]))
            pos--;
        return line.substr(pos);

    }

    char *string_to_char_array(string str) {
        char *arr = new char[str.length() + 1];
        std::strcpy(arr, str.c_str());
        return arr;
    }

    // endregion
    //region files & streams
    map<int, FILE *> files;

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

    FILE *open_file_timestamp(const string &filename) {
        string timestamp = timeStamp("%Y-%m-%d_%H-%M-%S");
        //    myString.assign(buffer);
        ofstream myfile;
        myfile.open(timestamp);
        if (myfile.is_open()) {
            cout << "created log file" << endl;
        }
        myfile.close();
        return nullptr;
    }


    void blockRead(int fd) {
        vector<int> fds({fd});
        blockRead(fds);
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

    void blockRead(const vector<int> &fds) {
        for ever {
            for (auto fd: fds)
                flushTo(fd, "block");
        }
    }

#pragma clang diagnostic pop

    string flushTo(int fd, const string &prefix, bool do_block) {
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

    FILE *openFile(int fd) {
        auto iter = files.lower_bound(fd);
        if (iter == files.end() || fd < iter->first) {    // not found
            FILE *file = fdopen(fd, "r");
            files.insert(iter, make_pair(fd, file));     // hinted insertion
        }
        return files[fd];
    }

    void load(size_t dots, size_t max_dots) {
        cout << "\rloading";
        for (size_t i = 0; i < max_dots; i++)
            cout << (i < dots ? "." : " ");
        fflush(stdout);
    }

    void endLoad(size_t max_dots) {
        cout << "\r        ";
        for (size_t i = 0; i < max_dots; i++)
            cout << " ";
        cout << endl;
        fflush(stdout);
    }

    //endregion

    template<class T>
    size_t length(T *arr[]) {
        size_t length = 0;
        for (; *arr != nullptr; arr++) {
            length++;
        }
        return length;
    }

    int to_int(const string &c) {
        //debug << "try to parse '" << c << "'" << endl;
        fflush(stdout);
        return stoi(c);
    }

    regex getPatternByTitle(vector<string> &format, const string &title) {
//        if (title=="Text"){
//            size_t n = format.size()-1;
//            string pattern ="^(([^,]*,){" + to_string(n) + "})(.*)";
//            return regex(pattern);
//        }
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

    smatch getRegex(const string &line, const regex &text_pattern) {
        smatch sm_text;
        regex_match(line, sm_text, text_pattern);
        return sm_text;
    }

    vector<string> regexFindAll(const string &text, const string &pattern) {
        vector<string> result;
        std::smatch match;

        string::const_iterator searchStart(text.cbegin());
        std::regex p{pattern};
        while (std::regex_search(searchStart, text.cend(), match, p)) {
            result.push_back(match[0].str());
            searchStart = match.suffix().first;
        }
        return result;
    }

    void error(const string &msg, int code) {
        cerr << "[ERR] " << msg << endl;
        cout << "[ERR] " << msg << endl;
        if (code != 0)
            exit(code);
    }

    string pwd() {
        string cwd(get_cwd());
        printf("[%d] PWD:%s\n", getpid(), cwd.c_str());
        return cwd;
    }

    string get_cwd() {
        char c_current_path[FILENAME_MAX];

        if (!GetCurrentDir(c_current_path, sizeof(c_current_path))) {
            exit(99);
        }

        c_current_path[sizeof(c_current_path) - 1] = '\0';
        string cwd(c_current_path);
        return cwd;
    }


}
