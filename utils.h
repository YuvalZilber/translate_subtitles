//
// Created by Yuval Zilber on 27/02/2023.
//

#ifndef EXTRACT_SUBTITLES_2_UTILS_H
#define EXTRACT_SUBTITLES_2_UTILS_H


#include <string>
#include <vector>
#include <iostream>
#include <fcntl.h>
#include <sstream>
#include <map>
using namespace std;
class utils {
public:
    static vector<string> split (const string& s, const string& delimiter=" ");
    static vector<string> split (const string&, int, const string& delimiter=" ");
    static vector<string> lsplit (const string&, size_t, const string& delimiter=" ");
    static vector<string> rsplit (const string&, size_t, const string& delimiter=" ");
    static string myFlush(int fd, const string &prefix="", bool block=true);
    static string getUnboundedLine(int fd);
    static string getUnboundedLine(FILE* fd=stdin);
    static bool replace(std::string& str, const std::string& from, const std::string& to);
    [[noreturn]] static void blockRead(const vector<int>& fds);
    [[noreturn]] static void blockRead(int fd);
private:
    static map<int, FILE*> files;
    static FILE* openFile(int fd);
};


#endif //EXTRACT_SUBTITLES_2_UTILS_H
