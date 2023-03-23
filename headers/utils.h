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
using namespace std;
namespace utils {
    vector<string> split (const string& s, const string& delimiter=" ");

    [[maybe_unused]] [[maybe_unused]] vector<string> split (const string&, int, const string& delimiter=" ");
    vector<string> lsplit (const string&, size_t, const string& delimiter=" ");
    vector<string> rsplit (const string&, size_t, const string& delimiter=" ");
    string MyFlush(int fd, const string &prefix="", bool block=true);
    string getUnboundedLine(int fd);
    string getUnboundedLine(FILE* fd=stdin);
    bool replace(std::string& str, const std::string& from, const std::string& to);
    [[noreturn]] void BlockRead(const vector<int>& fds);
    [[noreturn]] void BlockRead(int fd);
    template<size_t N>
    void TrimStart(string& s, array<string, N>& toTrim);
    template<size_t N>
    void TrimEnd(string& s, array<string, N>& toTrim);
    template<size_t N>
    void trim(string& s, array<string, N>& toTrim);
    int to_int(const string &c);
    void load(size_t dots, size_t max_dots = 3);
    void EndLoad(size_t max_dots = 3);
    regex GetPatternByTitle(vector<string> &format, const string &title); //todo: find a better place
    size_t find_index(vector<string> &v, const string &element);
    smatch GetRegex(const string &line, const regex &text_pattern);
    void error(const string &msg, int code = -1);
    bool endsWithPunctuation(const string &line);
    string getPunctuationAtEnd(const string& line);


    FILE* OpenFile(int fd);
    void SortStrings(char* arr[]);
    template<size_t N>
    char* TrimStart(char *sp, array<char*, N>& toTrim);
    template<size_t N>
    char* TrimEnd(char *sp, array<char[], N>& toTrim);
    template<class T>
    size_t length(T* arr[]);


};


#endif //EXTRACT_SUBTITLES_2_UTILS_H
