#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include "../headers/font.h"
#include "../headers/MkvFile.h"
#include <regex>
#include <chrono>
#include <array>
#include <thread>
#include <cstdio>  /* defines FILENAME_MAX */

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else

#include <unistd.h>

#define GetCurrentDir getcwd
#endif
using namespace std;
namespace me = this_thread;
using namespace chrono;
using namespace utils;
using namespace logger;

#define BUF_SIZE 1<<10

Path file2trans;
Path sub_file;

void step1_extract_all_tracks_of_sample(const string &path);

vector<Path> get_eng_sub_paths(const char *src);

void step2(const vector<Path> &paths, const char *track_num);

Path MkvExtract(const Path &path, size_t si);

Path MkvExtract(const Path &path, const string &si);

void step3_translate(const string &t);

std::string string_to_hex(const std::string &input);

void process_break_line(const string &origin, string &translation);

bool endsWithPunctuation(const string &line);

template<class T>
void print_vector(const vector<T> &hhmmss_stop);


const std::ofstream devnull("/dev/null", std::ofstream::out | std::ofstream::app);
FILE *devnull_f;


vector<int> GetVector(const string &line, regex &time_pattern);

string getRegexS(const string &line, const regex &text_pattern);

string *SendCommand(const string &cmd, string *response = nullptr);

int vlc_get_time();

string flipPunctuationIfNeeded(size_t n, const vector<string> &lines);

int vlc_get_time() {
    string cur_s;
    SendCommand("get_time", &cur_s);
    cur_s = regex_replace(cur_s, regex("^\\D+"), "");
    if (cur_s.empty())
        return -1;
    return to_int(cur_s);
}

int pipe_to_vlc[2];
int pipe_out_vlc[2];


const std::ostream &debug1() {
    if (debug_mode)
        return cerr << "[DEBUG] ";
    return devnull;
}


string pwd() {
    char c_current_path[FILENAME_MAX];

    if (!GetCurrentDir(c_current_path, sizeof(c_current_path))) {
        exit(99);
    }

    c_current_path[sizeof(c_current_path) - 1] = '\0';
    printf("[%d] PWD:%s", getpid(), c_current_path);
    string cwd(c_current_path);
    return cwd;
}

void WriteLine(const string &line) {
    write(pipe_to_vlc[WRITE], (line + "\n").c_str(), line.length() + 1);
}

int getDialogLine(Path &p) {
    ifstream src(p);
    string line;
    bool has_dialog = false;
    size_t match = 0;
    size_t max = 0;
    std::regex const eng("[a-zA-Z]");
    std::regex const text(R"(([^0-9:,.\]\\: '\"]))");
    while (getline(src, line)) {
        if (line[0] < 0 && line[0] != -17)
            return 0;
        if (line.starts_with("Dialogue: ")) {
            has_dialog = true;

            std::ptrdiff_t const c_eng(std::distance(
                    std::sregex_iterator(line.begin(), line.end(), eng),
                    std::sregex_iterator()));
            std::ptrdiff_t const c_tex(std::distance(
                    std::sregex_iterator(line.begin(), line.end(), text),
                    std::sregex_iterator()));
            if (c_eng == c_tex)
                match++;
            max++;
        }
    }
    fs::remove(p);
    if (!has_dialog)
        return 0;
    return (double) match * 1000 / max;
}

int rateEngSub(Path &p) {
    ifstream src(p);
    string line;
    bool has_dialog = false;
    size_t match = 0;
    size_t max = 0;
    std::regex const eng("[a-zA-Z]");
    std::regex const text(R"([^0-9:,.\]\\: '\"])");
    while (getline(src, line)) {
        if (line[0] < 0 && line[0] != -17)
            return 0;
        if (line.starts_with("Dialogue: ")) {
            has_dialog = true;

            std::ptrdiff_t const c_eng(std::distance(
                    std::sregex_iterator(line.begin(), line.end(), eng),
                    std::sregex_iterator()));
            std::ptrdiff_t const c_tex(std::distance(
                    std::sregex_iterator(line.begin(), line.end(), text),
                    std::sregex_iterator()));
            if (c_eng == c_tex)
                match++;
            max++;
        }
    }
    fs::remove(p);
    if (!has_dialog)
        return 0;
    return (double) match * 1000 / max;
}


char *to_system_cmd(char *args[]) {
    size_t sum_size = 0;
    for (int i = 0; args[i] != nullptr; ++i) {
        sum_size += strlen(args[i]);
        sum_size++;
    }
    char *cmd = new char[sum_size];
    char *p = cmd;
    for (int i = 0; args[i] != nullptr; ++i) {
        char *arg = args[i];
        size_t size = strlen(arg);
        memcpy(p, arg, size);
        p += size;
        *(p++) = ' ';
    }
    *(p - 1) = '\0';
    return cmd;
}

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
    if (MkvFile::LongestDialog(sub_file).empty()) {
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
            sub_file = mkv.ChooseSubtitleFile();

        }
        getSubFileRaw();
    }
    return sub_file;
}

FILE *open_file_timestamp(string filename) {
    time_t t = time(0);   // get time now
    struct tm *now = localtime(&t);

    char buffer[80];
    size_t length = strftime(buffer, 80, "%Y-%m-%d_%H-%M-%S", now);
    string full_filename = buffer;
//    myString.assign(buffer);
    std::ofstream myfile;
    myfile.open(buffer);
    if (myfile.is_open()) {
        std::cout << "created log file" << std::endl;
    }
    myfile.close();
    return 0;
}

int main(int argc, char *argv[]) {
    output = pwd();
    devnull_f = fopen("/dev/null", "w+");
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
            }
            else if (MkvFile::LongestDialog(sub_file).empty())
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
    }
//    if(!debug_mode)
//        debug =
    pipe(pipe_to_vlc);
    pipe(pipe_out_vlc);
    string fn_s = getVideoFile();
    string sfn_s = getSubFile();

    pid_t pid = fork();
    FILE *vlc_stderr = fopen("vlc_stderr.log", "w+");

    if (pid == 0) { //child
        debug << "pid child: " << getpid() << endl;

        fs::current_path("/");
        char filename[fn_s.length() + 1];
        strcpy(filename, fn_s.c_str());

        char subFile_c[sfn_s.length() + 1];
        strcpy(subFile_c, sfn_s.c_str());
        // vlc -I rc --extraintf macosx
//        string cmd = "vlc -I rc --extraintf macosx --sub-file "+sfn_s+" "+fn_s;
//        string all_cmd = (R"(tell app "Terminal" to do script ")"+cmd+R"(")");
//        char all_cmd_c[all_cmd.length() + 1];
//        strcpy(all_cmd_c, all_cmd.c_str());
//        char *args[] = {(char*)"osascript", (char*)"-e", all_cmd_c,nullptr};
        char *args[] = {(char *) "vlc", (char *) "-I", (char *) "rc", (char *) "--extraintf",
                        (char *) "macosx",
                        (char *) "--sub-file", subFile_c, filename, nullptr};
        debug << "$$$$$$$$$$ " << subFile_c << endl;


        close(pipe_to_vlc[WRITE]);
        close(pipe_out_vlc[READ]);
        int fd_in = pipe_to_vlc[READ];
        int fd_out = pipe_out_vlc[WRITE];
        int fd_err = fileno(vlc_stderr);
        close(STDERR_FILENO);
        dup(fd_err);
        close(STDIN_FILENO);
        dup(fd_in);
        close(STDOUT_FILENO);
        dup(fd_out);
        cout << to_system_cmd(args) << endl;
        execvp(args[0], args);
    }
    else {//parent
        debug << "pid parent: " << getpid() << endl;
        close(pipe_to_vlc[READ]);
        close(pipe_out_vlc[WRITE]);
        int fd_vlc_out = pipe_out_vlc[READ];
        sleep(3);
        utils::MyFlush(fd_vlc_out, "OUT");
        string response;

        SendCommand("help", &response);
        cout << response << endl;
        step3_translate("heb_sub.ass");

        wait(nullptr);
    }
    return 0;
}

string *SendCommand(const string &cmd, string *response) {
    WriteLine(cmd);
    fflush(stderr);
    CommandsLogger << "> " << cmd << endl;
    if (response == nullptr) {
        utils::MyFlush(pipe_out_vlc[READ], "", false);
        return nullptr;
    }
    *response = utils::MyFlush(pipe_out_vlc[READ]);
    *response = regex_replace(*response, regex("\r"), "");
    *response = regex_replace(*response, regex("\n+$"), "");
    CommandsLogger << "-------------------------------------------" << endl;
    CommandsLogger << *response << endl;
    CommandsLogger << "-------------------------------------------" << endl;
    return response;
}

void step3_translate(const string &t) {
    const string filename_src = getSubFile();
    const string &filename_trg = t;

    cout << filename_trg << endl;
    ifstream src(filename_src);
    fstream trg;
    bool was_exist = false;
    if (fs::exists(filename_trg)) {
        cout << "found file from last run." << endl << "loading..." << endl << endl;
        trg.open(filename_trg, fstream::out | fstream::in);

        was_exist = true;
    }
    else
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
        bool stop = false;
        int lines_num = 0;
        vector<size_t> line_starts_target;
        vector<size_t> line_starts_source;
        ostream *tell2_ptr;
        if (was_exist) {
            while (getline(trg, line)) {
                lines_num++;
                getline(src, line);
                line_starts_target.push_back(trg.tellg());
                line_starts_source.push_back(src.tellg());
                if (line.starts_with("Format: ")) {
                    string sformat = utils::split(line)[1];
                    format = utils::split(sformat, ",");
                    text_pattern = GetPatternByTitle(format, "Text");
                    time_pattern = GetPatternByTitle(format, "Start");
                    stop_pattern = GetPatternByTitle(format, "End");
                }
            }
            line_starts_target.pop_back();
            trg.close();
            trg.open(filename_trg, ios::out | ios::app);
        }

        debug << "loaded " << lines_num << " lines" << endl;
//        auto *tell_ptr = file_with_timestamp("tell");
//        auto &tell = *tell_ptr;
        while (getline(src, line)) {
            line_starts_target.push_back(trg.tellg());
            line_starts_source.push_back(src.tellg());

            if (!stop) {
                if (line.starts_with("[")) {
                }
                else if (line.starts_with("Format: ")) {
                    string sformat = utils::split(line)[1];
                    format = utils::split(sformat, ",");
                    text_pattern = GetPatternByTitle(format, "Text");
                    time_pattern = GetPatternByTitle(format, "Start");
                    stop_pattern = GetPatternByTitle(format, "End");
                }
                else if (line.starts_with("Dialogue: ")) {
                    smatch sm_text = GetRegex(line, text_pattern);
                    vector<int> hhmmss_start = GetVector(line, time_pattern);

                    vector<int> hhmmss_end = GetVector(line, stop_pattern);
                    int cur;
                    int time_to_play_to_2 = hhmmss_end[2] + hhmmss_end[1] * 60 + hhmmss_end[0] * 60 * 60;
                    int time_to_play_to_1 = hhmmss_start[2] + hhmmss_start[1] * 60 + hhmmss_start[0] * 60 * 60;
                    string translation;
                    TRANSLATE:
                    cur = vlc_get_time();

                    cout << Font(Color::yellow) << "[" << GetRegex(line, time_pattern)[3] << "] - ";
                    cout << "[" << GetRegex(line, stop_pattern)[3] << "]" << Font::reset << endl;
                    cout << Font(Color::cyan) << "[# - exit, $ - replay, ^ - undo last line]" << Font::reset << endl;
                    cout << sm_text[3] << endl;
                    if ((!translation.starts_with("$")) && (time_to_play_to_1 < cur || cur + 12 < time_to_play_to_1))
                        SendCommand("seek " + to_string(time_to_play_to_1));
//                    this_thread::sleep_for(milliseconds(250));

                    cur = vlc_get_time();
                    SendCommand("play");
                    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                    debug << "sleep for " << (time_to_play_to_2 - cur) << "s" << endl;
                    sleep(time_to_play_to_2 - cur);
                    if (hhmmss_start[2] == hhmmss_end[2]) {
                        this_thread::sleep_for(milliseconds((hhmmss_start[3] + 1) * 10));
                    }
                    SendCommand("pause");
                    //sendCommand("seek " + to_string(time_to_play_to_2));

                    translation = utils::getUnboundedLine();
                    if (translation.starts_with("$")) {
                        cur = vlc_get_time();

                        int requested = atoi(translation.substr(1).c_str());
                        int time_to_rewind = requested ? requested : 10;
                        time_to_rewind = min(time_to_rewind, cur);
                        SendCommand("seek " + to_string(cur - time_to_rewind));
                        goto TRANSLATE;
                    }
                    else if (translation.starts_with("#")) {
                        stop = true;
                        break;
                    }
                    else if (translation.empty()) {
                        line_starts_source.pop_back();
                        debug << line_starts_source.back() << endl;
                        src.seekg(line_starts_source.back(), ios::beg);
                        continue;
                    }
                    else if (translation.starts_with("^")) {
                        line_starts_source.pop_back();
                        line_starts_source.pop_back();
                        debug << line_starts_source.back() << endl;
                        src.seekg(line_starts_source.back(), ios::beg);

                        line_starts_target.pop_back();
                        trg.close();
                        truncate(filename_trg.c_str(), line_starts_target.back());
                        line_starts_target.pop_back();
                        trg.open(filename_trg, ios::out | ios::app);

                        continue;
                    }
                    else {
                        process_break_line(line, translation);

                        line = string(sm_text[1]) + translation + string(sm_text[5]);
                    }
//                    tell << trg.tellp() << "-" << trg.tellg() << endl;
                }
                //trg.seekp(0, ios::end);
                trg << line << endl;
                trg.flush();
            }

        }
        trg.close();
        src.close();
    }
    else {
        if (!src.is_open())
            cerr << "couldn't open src '" << filename_src << "'" << endl;
    }
    SendCommand("quit");
    cout << "bye!" << endl;
}

string getRegexS(const string &line, const regex &text_pattern) {
    smatch sm_text;
    regex_match(line, sm_text, text_pattern);
    string s;
    for (auto p: sm_text)
        s += p.str();
    return s;
}


vector<int> GetVector(const string &line, regex &time_pattern) {
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

void process_break_line(const string &origin, string &translation) {
    translation = regex_replace(translation, regex(R"(\s*@\s*)"), " \\N");
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
        translation += origin[origin.length() - 1];

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

            n = line.length();
            if (endsWithPunctuation(line)) {
                line = line[n - 1] + line.substr(0, n - 1);
            }
            res += line;
            if (i < lines.size() - 1)
                res += " \\N";
        }
    }
    return res;
}

bool endsWithPunctuation(const string &line) {
    string punctuation = ",.!?:;\"\\/'";

    return line.find_last_of(punctuation, line.length()) == line.length() - 1;
}




