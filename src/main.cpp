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

#include "../headers/consts.h"

#include "../headers/font.h"
#include "../headers/MkvFile.h"
#include "../headers/Vlc.h"

#define GetCurrentDir getcwd
#endif
using namespace std;
namespace me = this_thread;
using namespace utils;
using namespace logger;

Path file2trans;
Path sub_file;

void step3_translate(Vlc vlc, const string &t);

void process_break_line(string &origin, string &translation);

vector<int> getVector(const string &line, regex &time_pattern);

string flipPunctuationIfNeeded(size_t n, const vector<string> &lines);

Path getVideoFile() {
    while (!fs::exists(file2trans)) {
        if (!file2trans.empty())
            cerr << "Can't find file " << file2trans << endl;
        cout << Font(Style::bold) << "file to translate: " << Font::reset;
        file2trans = utils::getUnboundedLine();
    }
    return file2trans;
}


bool isValidSubtitleFile(const Path &subs, bool error_not_exists = true) {
    if (!exists(subs)) {
        if (error_not_exists && !subs.empty())
            cerr << "Can't find file '" << sub_file << "'" << endl;
        return false;
    }
    if (MkvFile::longestDialog(sub_file).empty()) {
        error("the file '" + sub_file.string() + "' is not a valid subtitle file format", 0);
        return false;
    }
    return true;
}

Path getSubFileRaw() {
    while (!isValidSubtitleFile(sub_file)) {
        cout << Font(Style::bold) << "subtitle file: " << Font::reset;
        sub_file = utils::getUnboundedLine();
    }
    return sub_file;
}

Path getSubFile() {
    Path file = getVideoFile();
    if (!exists(sub_file)) {
        file = getVideoFile();
        string ext = file.extension().string();
        if (ext == ".mkv") {
            MkvFile mkv{file};
            sub_file = mkv.chooseSubtitleFile();

        }
        getSubFileRaw();
    }
    return sub_file;
}


int main(int argc, char *argv[]) {
    output = utils::pwd();
    for (int i = 0; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-d" || arg == "--debug")
            debug_mode = true;
        if (arg == "-s" || arg == "--sub-file") {
            if (argc < i + 1)
                error("expected subtitle file after " + arg + " option");
            sub_file = argv[++i];
            if (!fs::exists(sub_file)) {
                error("Couldn't find subtitles file '" + sub_file.string() + "'", 0);
                sub_file = "";
            } else if (MkvFile::longestDialog(sub_file).empty())
                error("the file '" + sub_file.string() + "' is not a valid subtitle file format");
        }
        if (arg == "-v" || arg == "--vid-file") {
            if (argc < i + 1)
                error("expected video file after " + arg + " option");
            file2trans = argv[++i];
            if (!fs::exists(file2trans))
                error("Couldn't find video file '" + file2trans.string() + "'");
            else
                cout << "[SUC] Chose file " << file2trans << endl;
        }
        if (arg == "-p") {
            if (fs::exists("heb_sub1.ass")) {
                if (fs::exists("try.ass"))
                    fs::remove("try.ass");
                fs::copy("heb_sub1.ass", "try.ass");
            } else if (fs::exists("heb_sub1.ass")) {
                if (fs::exists("try.ass"))
                    fs::remove("try.ass");
                fs::copy("heb_sub1.ass", "heb_sub1.ass");
                fs::rename("heb_sub1.ass", "try.ass");
            }
            sub_file = "try.ass";
        }
    }


    string fn_s = getVideoFile();
    string sfn_s = getSubFile();

    Vlc vlc = Vlc(fn_s, sfn_s);

    int pidp = getpid();
    debug << "pid parent: " << to_string(pidp) << "\n";
    sleep(3);
    string response = vlc.help();
    cout << response << endl;
    step3_translate(vlc, "heb_sub.ass");

    wait(nullptr);
    return 0;
}


void step3_translate(Vlc vlc, const string &t) {
    const string filename_src = getSubFile();
    const string &filename_trg = t;

    cout << filename_trg << endl;
    ifstream src(filename_src);
    fstream trg;
    bool target_already_exists = fs::exists(filename_trg);
    if (target_already_exists) {
        cout << "found file from last run." << endl << "loading..." << endl << endl;
        trg.open(filename_trg, fstream::out | fstream::in);
    } else
        trg.open(filename_trg, fstream::out);
    if (!trg) {
        cerr << "Can't open target file: '" << filename_trg << "'" << endl;
        cerr << "cwd+filename: '" << pwd() << "/" << filename_trg << "'" << endl;
    }


    string str;


    if (src.is_open() && trg.is_open()) {
        string line;
        vector<string> format;
        regex text_pattern, time_pattern, stop_pattern;
        int lines_num = 0;
        vector<long> line_starts_target;
        vector<long> line_starts_source;
        if (target_already_exists) {
            while (getline(trg, line)) {
                lines_num++;
                getline(src, line);
                line_starts_target.push_back(trg.tellg());
                line_starts_source.push_back(src.tellg());
                if (line.starts_with("Format: ")) {
                    string sformat = line.substr(line.find(' '));
                    format = utils::split(sformat, ",");
                    text_pattern = getPatternByTitle(format, "Text");
                    time_pattern = getPatternByTitle(format, "Start");
                    stop_pattern = getPatternByTitle(format, "End");
                }
            }
            line_starts_target.pop_back();
            trg.close();
            trg.open(filename_trg, ios::out | ios::app);
        }

        debug << "loaded " << to_string(lines_num) << " lines" << endl;
//        auto *tell_ptr = file_with_timestamp("tell");
//        auto &tell = *tell_ptr;
        while (getline(src, line)) {
            line_starts_target.push_back(trg.tellg());
            line_starts_source.push_back(src.tellg());

            if (line.starts_with("[")) {
            } else if (line.starts_with("Format: ")) {
                string sformat = line.substr(line.find(' '));
                format = utils::split(sformat, ",");
                text_pattern = getPatternByTitle(format, "Text");
                time_pattern = getPatternByTitle(format, "Start");
                stop_pattern = getPatternByTitle(format, "End");
            } else if (line.starts_with("Dialogue: ")) {
                smatch sm_text = getRegex(line, text_pattern);
                vector<int> hhmmss_start = getVector(line, time_pattern);

                vector<int> hhmmss_end = getVector(line, stop_pattern);
                int cur;
                int time_to_play_to_2 = hhmmss_end[2] + hhmmss_end[1] * 60 + hhmmss_end[0] * 60 * 60;
                int time_to_play_to_1 = hhmmss_start[2] + hhmmss_start[1] * 60 + hhmmss_start[0] * 60 * 60;
                if (time_to_play_to_2 > time_to_play_to_1 + 1)
                    time_to_play_to_2--;
                string translation;
                TRANSLATE:
                cur = vlc.get_time();

                cout << Font(Color::yellow) << "[" << getRegex(line, time_pattern)[3] << "] - ";
                cout << "[" << getRegex(line, stop_pattern)[3] << "]" << Font::reset << endl;
                cout << Font(Color::cyan) << "[# - exit, $ - replay, ^ - undo last line]" << Font::reset << endl;
                cout << sm_text[3] << endl;
                if ((!translation.starts_with("$")) && (time_to_play_to_1 < cur || cur + 12 < time_to_play_to_1))
                    vlc.seek(time_to_play_to_1);
//                    this_thread::sleep_for(milliseconds(250));

                cur = vlc.get_time();
                int play_time = (time_to_play_to_2 - cur) * 1000;
                if (hhmmss_start[2] == hhmmss_end[2]) {
                    play_time += (hhmmss_start[3] + 1) * 10;
                }
                vlc.play_for(play_time);

                translation = utils::getUnboundedLine();
                if (translation.starts_with("$")) {
                    cur = vlc.get_time();

                    int requested = atoi(translation.substr(1).c_str());
                    int time_to_rewind = requested ? requested : 10;
                    time_to_rewind = min(time_to_rewind, cur);
                    vlc.seek(cur - time_to_rewind);
                    goto TRANSLATE;
                } else if (translation.starts_with("#")) {
                    break;
                } else if (translation.empty()) {
                    line_starts_source.pop_back();
                    debug << to_string(line_starts_source.back()) << endl;
                    src.seekg(line_starts_source.back(), ios::beg);
                    continue;
                } else if (translation.starts_with("^")) {
                    line_starts_source.pop_back();
                    line_starts_source.pop_back();
                    debug << to_string(line_starts_source.back()) << endl;
                    src.seekg(line_starts_source.back(), ios::beg);

                    line_starts_target.pop_back();
                    trg.close();
                    truncate(filename_trg.c_str(), line_starts_target.back());
                    line_starts_target.pop_back();
                    trg.open(filename_trg, ios::out | ios::app);

                    continue;
                } else {
                    string origin = sm_text[3];
                    process_break_line(origin, translation);

                    line = string(sm_text[1]) + translation + string(sm_text[5]);
                }
//                    tell << trg.tellp() << "-" << trg.tellg() << endl;
            }
            //trg.seekp(0, ios::end);
            trg << line << endl;
            trg.flush();

        }
        trg.close();
        src.close();
    } else {
        if (!src.is_open())
            cerr << "couldn't open src '" << filename_src << "'" << endl;
        if (!trg.is_open())
            cerr << "couldn't open trg '" << filename_trg << "'" << endl;
    }
    vlc.quit();
    cout << "bye!" << endl;
}

vector<int> getVector(const string &line, regex &time_pattern) {
    smatch sm_time;
    regex_match(line, sm_time, time_pattern);
    string time_s = sm_time[3].str();
    if (!time_s.starts_with("0:")) {
        cerr << "don't handle hours yet" << endl;
        exit(1);
    }
    time_s[time_s.find('.')] = ':';
    vector<string> hhmmss_start_s = utils::split(time_s, ":");
    vector<int> hhmmss_start;
    transform(hhmmss_start_s.begin(), hhmmss_start_s.end(), back_inserter(hhmmss_start), to_int);
    return hhmmss_start;
}

void process_break_line(string &origin, string &translation) {
    translation = regex_replace(translation, regex(R"(\s*@\s*)"), " \\N");
    auto styles = regex(R"((\{(([^}])|(\}\{))*\}))");

    origin = regex_replace(origin, styles, "");
    size_t n = translation.length();
    if (!translation.contains("\\N")) {
        if (translation.contains(" ")) {
            if (origin.contains("\\N") || origin.contains("\\n") || origin.contains("<br>")) {
                vector<string> words = utils::split(translation);
                size_t sum = 0;
                string res;
                bool spaced = false;
                for (auto &word: words) {
                    sum += word.length() + 1;
                    res += word + " ";
                    if (!spaced && (sum >= n / 2)) {
                        res += "\\N";
                        spaced = true;
                    }
                }
                translation = res;

            }
        }
    }

    string to_trim[] = {"\n", " ", "\\N"};
    translation = regex_replace(translation, regex(R"((^(\s|\\N)+)|((\s|\\N)+$))"), "");

    if (endsWithPunctuation(origin) && !endsWithPunctuation(translation))
        translation += getPunctuationAtEnd(origin);

    vector<string> lines = utils::split(translation, " \\N");
    string res = flipPunctuationIfNeeded(n, lines);
    translation = res;
}

string flipPunctuationIfNeeded(size_t n, const vector<string> &lines) {
    string res;
    if (flip_punctuation) {
        for (int i = 0; i < lines.size(); i++) {
            string line = lines[i];

            line = regex_replace(line, regex(R"((^\s+)|(\s+$))"), "");

            string prefix;
            while (line.starts_with('-') || line.starts_with('"')) {
                prefix += line[0];
                line = line.substr(1);
            }
            n = line.length();
            if (endsWithPunctuation(line)) {
                string punc = getPunctuationAtEnd(line);
                std::reverse(punc.begin(), punc.end());
                line = punc.append(line.substr(0, n - punc.length()));
            }
            res += line + prefix;
            if (i < lines.size() - 1)
                res += " \\N";
        }
    }
    return res;
}




