//
// Created by Yuval Zilber on 27/02/2023.
//

#ifndef EXTRACT_SUBTITLES_2_UTILS_H
#define EXTRACT_SUBTITLES_2_UTILS_H


//#include <string>
#include <vector>
#include <iostream>
#include <fcntl.h>
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
#include "MkvFile.h"
#include "font.h"
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "base_utils.h"
using namespace std;
namespace utils {
    using namespace base_utils;
    inline std::string trim(std::string& str);
    vector<string> split (const string& s, const string& delimiter=" ", bool trim=true);

    FILE *open_file_timestamp(const string& filename);
    [[maybe_unused]] [[maybe_unused]] vector<string> split (const string&, int, const string& delimiter=" ");
    vector<string> lsplit (const string&, size_t, const string& delimiter=" ");
    vector<string> rsplit (const string&, size_t, const string& delimiter=" ");
    string flushTo(int fd, const string &prefix="", bool block=true);
    string getUnboundedLine(int fd);
    string getUnboundedLine(FILE* fd=stdin);
    bool replace(std::string& str, const std::string& from, const std::string& to);
    [[noreturn]] void blockRead(const vector<int>& fds);
    [[noreturn]] void blockRead(int fd);
    template<size_t N>
    void trimStart(string& s, array<string, N>& toTrim);
    template<size_t N>
    void trimEnd(string& s, array<string, N>& toTrim);
    template<size_t N>
    void trim(string& s, array<string, N>& toTrim);
    string trimRegex(const string& s, const string& pattern);
    int to_int(const string &c);
    void load(size_t dots, size_t max_dots = 3);
    void endLoad(size_t max_dots = 3);
    size_t count(const string& s, char needle);
    regex getPatternByTitle(vector<string> &format, const string &title); //todo: find a better place
    size_t find_index(vector<string> &v, const string &element);
    smatch getRegex(const string &line, const regex &text_pattern);
    vector<string> regexFindAll(const string& text, const string& pattern);
    void error(const string &msg, int code = -1);
    bool endsWithPunctuation(const string &line);
    string getPunctuationAtEnd(const string& line);
    char* string_to_char_array(string str);


    FILE* openFile(int fd);
    void sortStrings(char* arr[]);
    template<size_t N>
    char* trimStart(char *sp, array<char*, N>& toTrim);
    template<size_t N>
    char* trimEnd(char *sp, array<char[], N>& toTrim);
    template<class T>
    size_t length(T* arr[]);
    string pwd();
    string get_cwd();

};


#endif //EXTRACT_SUBTITLES_2_UTILS_H
