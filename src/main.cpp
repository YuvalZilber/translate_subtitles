#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <array>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else

#include <unistd.h>
#include <wait.h>

#include "consts.h"

#include "font.h"
#include "MkvFile.h"
#include "Vlc.h"
#include "MainOptions.h"
#include "TranslatorLtrToRtl.h"

#define GetCurrentDir getcwd
#endif
using namespace std;
namespace me = this_thread;
using namespace utils;
using namespace logger;

void step3_translate(Vlc vlc, const string &filename_src, const string &t);

Path getVideoFile(const MainOptions &args) {
    Path file2trans;
    MainOptions::Option *p = args.getParamFromKeys("v", "vid", "video");

    file2trans = p ? p->second : "";
    if (file2trans.empty() && !args.namelessValues_.empty()) {
        file2trans = args.namelessValues_[0];
    }

    while (!fs::exists(file2trans)) {
        if (!file2trans.empty())
            cerr << "Can'targetFile find file '" << file2trans << "'" << endl;
        cout << Font(Style::bold) << "file to translate: " << Font::reset;
        file2trans = utils::getUnboundedLine();
    }
    return file2trans;
}


bool isValidSubtitleFile(Path &subs, bool error_not_exists = true) {
    if (!exists(subs)) {
        if (error_not_exists && !subs.empty())
            cerr << "Can'targetFile find file '" << subs << "'" << endl;
        return false;
    }
    if (MkvFile::longestDialog(subs).empty()) {
        error("the file '" + subs.string() + "' is not a valid subtitle file format", 0);
        return false;
    }
    return true;
}

Path getSubFileRaw(Path &subs) {
    while (!isValidSubtitleFile(subs)) {
        cout << Font(Style::bold) << "subtitle file: " << Font::reset;
        subs = utils::getUnboundedLine();
    }
    return subs;
}

Path getSubFile(const MainOptions &args, Path video) {
    Path sub_file;
    MainOptions::Option *p = args.getParamFromKeys("s", "sub", "subs", "ass", "subtitle", "subtitles");

    sub_file = p ? p->second : "";


    if (!exists(sub_file)) {
        string ext = video.extension().string();
        if (ext == ".mkv") {
            MkvFile mkv{video};
            sub_file = mkv.chooseSubtitleFile();
        }
        sub_file = getSubFileRaw(sub_file);
    }
    return sub_file;
}

int main(int argc, char *argv[]) {

    MainOptions args(argc, argv);
    output = utils::pwd();
    Path file2trans;
    debug_mode = args.hasOneOfKeys("d", "debug");
    Path fn_s = getVideoFile(args);
    Path sfn_s = getSubFile(args, fn_s);

    Vlc vlc = Vlc(fn_s, sfn_s);

    TranslatorLtrToRtl translator(vlc, sfn_s, "hub_sub.ass");
    translator.translate();

    wait(nullptr);
    return 0;
}






