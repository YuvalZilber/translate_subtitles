//
// Created by yuvalzilber on 5/30/23.
//

#ifndef EXTRACT_SUBTITLES_2_TRANSLATORLTRTORTL_H
#define EXTRACT_SUBTITLES_2_TRANSLATORLTRTORTL_H


#include "Vlc.h"

class TranslatorLtrToRtl {
public:
    TranslatorLtrToRtl(Vlc &vlc, string filename_src, string t);

    void translate();

private:
    static vector<int> getTimeVector(const string &line, regex &time_pattern);

    static string extractByTitlePattern(const string &line, const regex &title_pattern);

    static void process_break_line(string &origin, string &translation);

    static string flipPunctuationIfNeeded(size_t n, const vector<string> &lines);

    const Vlc &player;
    const Path subtitleFile;
    const Path targetFile;
};


#endif //EXTRACT_SUBTITLES_2_TRANSLATORLTRTORTL_H
