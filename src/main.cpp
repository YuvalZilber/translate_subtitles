#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <cstdio>
#include "../headers/utils.h"
#include "../headers/font.h"
#include <regex>
#include <spawn.h>
#include <chrono>
#include <thread>
#include <stdio.h>  /* defines FILENAME_MAX */

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else

#include <unistd.h>

#define GetCurrentDir getcwd
#endif
using namespace std;
namespace fs = __fs::filesystem;
namespace me = this_thread;
using namespace chrono;

#define BUF_SIZE 1<<10
#define Path fs::path
#define READ 0
#define WRITE 1
bool debug_mode;
Path file2trans;
int trackNum = -1;
Path subFile;

void step1_extract_all_tracks_of_sample(const string &path);

vector<Path> get_eng_sub_paths(const char *src);

void step2(const vector<Path> &paths, const char *track_num);

Path mkvExtract(const Path &path, size_t si);

Path mkvExtract(const Path &path, const string &si);

void step3_translate(const string &t);

unsigned long find_index(vector<string> &v, const string &element);

regex foo(vector<string> &format, const string &title, bool);

regex foo(vector<string> &format, const string &title);

std::string string_to_hex(const std::string &input);


void process_break_line(const string &origin, string &translation);

bool endsWithPisuk(const string &line);

template<class T>
void print_vector(const vector<T> &hhmmss_stop);

class Logger : public std::ostream {
    class NullBuffer : public std::streambuf {
    private:
        bool last_bl = true;
    public:
        NullBuffer(const string &pre) : std::streambuf(), prefix(pre) {

        }

        std::streamsize xsputn(const char *s, std::streamsize n) override {
            if (!debug_mode)
                return 0;
            if (last_bl)
                cerr << "[" << prefix << "] ";
            cerr << s;
            last_bl = (s[n - 1] == '\n');
            return n;
        }

        int_type overflow(int_type c) override {
            if (!debug_mode)
                return c;
            if (c == traits_type::eof()) {
                return traits_type::eof();
            }
            else {
                char_type ch = traits_type::to_char_type(c);
                //this->xsputn(&ch, 1);
                cerr << ch;
                last_bl = (ch == '\n');
                return c;
            }
        }

        string prefix;
    } m_nb;

public:
    explicit Logger(const string &name = "LOG") : m_nb(name), std::ostream(&m_nb) {}
};

std::ofstream devnull("/dev/null", std::ofstream::out | std::ofstream::app);
FILE *devnull_f;
Logger debug("DEBUG");
Logger commandsLogger("CMD");

int to_int(const string &c) {
    debug << "try to parse '" << c << "'" << endl;
    fflush(stdout);
    return stoi(c);
};

vector<int> getVector(const string &line, regex &time_pattern);

string getRegexS(const string &line, const regex &text_pattern);

smatch getRegex(const string &line, const regex &text_pattern);

string *sendCommand(const string &cmd, string *response = nullptr);

int vlc_get_time();

int vlc_get_time() {
    string cur_s;
    sendCommand("get_time", &cur_s);
    cur_s = regex_replace(cur_s, regex("^\\D+"), "");
    if (cur_s.empty())
        return -1;
    return to_int(cur_s);
}

auto &cerr_backup = cerr;
auto &cout_backup = cout;

int pipe_to_vlc[2];
int pipe_out_vlc[2];

void debug_old(const string &msg) {
    if (debug_mode)
        cerr << "[DEBUG] " << msg << endl;
}


std::ostream &debug1() {
    if (debug_mode)
        return cerr << "[DEBUG] ";
    return devnull;
}

void error(const string &msg, int code = -1) {
    cerr << "[ERR] " << msg << endl;
    if (code != 0)
        exit(code);
}

string pwd() {
    char cCurrentPath[FILENAME_MAX];

    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath))) {
        exit(99);
    }

    cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
    printf("[%d] PWD:%s", getpid(), cCurrentPath);
    string cwd(cCurrentPath);
    return cwd;
}

void writeLine(const string &line) {
    write(pipe_to_vlc[WRITE], (line + "\n").c_str(), line.length() + 1);
}

int getDialogLine(Path &p) {
    ifstream src(p);
    string line;
    bool hasDialog = false;
    size_t match = 0;
    size_t max = 0;
    std::regex const eng("[a-zA-Z]");
    std::regex const text("([^0-9:,.\\]\\\\: '\\\"])");
    while (getline(src, line)) {
        if (line[0] < 0 && line[0] != -17)
            return 0;
        if (line.starts_with("Dialogue: ")) {
            hasDialog = true;

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
    if (!hasDialog)
        return 0;
    return match * 1000 / max;
}

int rateEngSub(Path &p) {
    ifstream src(p);
    string line;
    bool hasDialog = false;
    size_t match = 0;
    size_t max = 0;
    std::regex const eng("[a-zA-Z]");
    std::regex const text(R"([^0-9:,.\]\\: '\"])");
    while (getline(src, line)) {
        if (line[0] < 0 && line[0] != -17)
            return 0;
        if (line.starts_with("Dialogue: ")) {
            hasDialog = true;

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
    if (!hasDialog)
        return 0;
    return match * 1000 / max;
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

template<typename T>
class KeyValueProperty {
public:
    T value = T();
    std::string key = "";

    KeyValueProperty(const std::string &key) : key(key) {}

    T &operator=(const T &i) { return value = i; };

    operator const T &() { return value; };
};

Path getVideoFile() {
    while (!fs::exists(file2trans)) {
        if (!file2trans.empty())
            cerr << "Can't find file " << file2trans << endl;
        cout << Font(Style::bold) << "file to translate: " <<   Font::reset;
        file2trans = utils::getUnboundedLine();
    }
    return file2trans;
}


string longestDialog(Path &p) {
    ifstream src(p);
    string line;
    string max_dialog;
    vector<string> format;
    regex text_pattern;

    while (getline(src, line)) {
        if (line[0] < 0 && line[0] != -17)
            break;
        if (line.starts_with("Format: ")) {
            string sformat = utils::split(line)[1];
            format = utils::split(sformat, ",");
            text_pattern = foo(format, "Text", true);
        }
        else if (line.starts_with("Dialogue: ")) {
            string dialog = getRegex(line, text_pattern)[3];
            dialog = regex_replace(dialog, regex(R"(\{\\[^}]*\})"), "");
            if (dialog.contains("\\N") || dialog.contains("<"))
                continue;
            if (dialog.length() > max_dialog.length()) {
                max_dialog = dialog;
            }
        }
    }
    return max_dialog;
}

typedef struct {
    size_t index;
    string dialog;
} trackOption;

void load(size_t dots, size_t max_dots = 3) {
    cout << "\rloading";
    for (size_t i = 0; i < max_dots; i++)
        cout << (i < dots ? "." : " ");
    fflush(stdout);
}
void endLoad(size_t max_dots = 3) {
    cout << "\r        ";
    for (size_t i = 0; i < max_dots; i++)
        cout << " ";
    cout<<endl;
    fflush(stdout);
}

int getTrackNum() {
    if (trackNum != -1 || !subFile.empty())
        return trackNum;
    Path file = getVideoFile();

    cout << "\nloading...";
    Path curSubFile("/");
    vector<trackOption> options;
    for (size_t i = 0; !curSubFile.empty(); i++) {
        load(i % 3 + 1);
        curSubFile = mkvExtract(file, i);
        string max_dialog = longestDialog(curSubFile);
        remove(curSubFile);
        if (!max_dialog.empty()) {
            options.push_back({i, max_dialog});
        }
    }
    endLoad();
    debug << "[" << getpid() << "] " << "finished mkvExtract(...)" << endl;
    cout << Font(Style::bold) << "Choose language or enter a subtitle filename" <<   Font::reset << endl;
    for (const auto &option: options) {
        cout << "O " << Font(Style::bold) << "[" << option.index << "] " <<   Font::reset << option.dialog << endl;
    }
    while (trackNum == -1) {
        cout << Font(Style::bold) << "your choice: " <<   Font::reset;
        string line = utils::getUnboundedLine();
        if (exists(Path(line))) {
            subFile = line;
            return -1;
        }
        bool found = false;
        try {
            trackNum = to_int(line);
            for (const auto &option: options) {
                if (option.index == trackNum) {
                    found = true;
                    break;
                }
            }
        }
        catch (exception &e) {
            cout << "-------------------------------------------------------------------" << endl;
            cout << "it is not a number, please enter the number of the wanted language:" << endl;
            cout << Font(Style::bold) << "Choose language or enter a subtitle filename" <<   Font::reset << endl;
            for (const auto &option: options) {
                cout << "[" << option.index << "] " << option.dialog << endl;
            }
        }

        if (!found)
            trackNum = -1;
    }
    return trackNum;
}

bool isValidSubtitleFile(const Path &subs, bool errorNotExists = true) {
    if (!exists(subs)) {
        if (errorNotExists && !subs.empty())
            cerr << "Can't find file '" << subFile << "'" << endl;
        return false;
    }
    if (longestDialog(subFile).empty()) {
        error("the file '" + subFile.string() + "' is not a valid subtitle file format", 0);
        return false;
    }
    return true;
}

Path getSubFileRaw() {
    while (!isValidSubtitleFile(subFile)) {
        cout << Font(Style::bold) << "subtitle file: " <<   Font::reset;
        subFile = utils::getUnboundedLine();
    }
    return subFile;
}

Path getSubFile() {
    Path file = getVideoFile();
    if (!exists(subFile)) {
        file = getVideoFile();
        string ext = file.extension().string();
        if (ext == ".mkv") {
            trackNum = getTrackNum();
            if (trackNum != -1) {
                subFile = mkvExtract(file, trackNum);
            }
        }
        getSubFileRaw();
    }
    return subFile;
}

int main(int argc, char *argv[]) {
    devnull_f = fopen("/dev/null", "w+");
    for (int i = 0; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-d" || arg == "--debug")
            debug_mode = true;
        if (arg == "-s" || arg == "--sub-file") {
            if (argc < i + 1)
                error("expected subtitle file after " + arg + " option");
            subFile = argv[++i];
            if (!fs::exists(subFile))
                error("Couldn't find file '" + subFile.string() + "'");
            if (longestDialog(subFile).empty())
                error("the file '" + subFile.string() + "' is not a valid subtitle file format");
        }
        if (arg == "-v" || arg == "--vid-file") {
            if (argc < i + 1)
                error("expected video file after " + arg + " option");
            file2trans = argv[++i];
            if (!fs::exists(file2trans))
                error("Couldn't find file '" + file2trans.string() + "'");
            else
                cout << "[SUC] Chose file '" << file2trans << "'" << endl;
        }
    }

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
        char *args[] = {(char *) "bash", (char *) "vlc", (char *) "-I", (char *) "rc", (char *) "--extraintf",
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
        utils::myFlush(fd_vlc_out, "OUT");
        string response;

        sendCommand("help", &response);
        cout << response << endl;
        step3_translate("heb_sub.ass");

        wait(nullptr);
    }
    return 0;
}

string myReplace(string s, string f, string t) {
    return regex_replace(s, regex(f), t);
}

string myReplace(char *s, char *f, char *t) {
    string ss = s;
    string ff = f;
    string tt = t;
    return myReplace(ss, ff, tt);
}

string *sendCommand(const string &cmd, string *response) {
    writeLine(cmd);
    fflush(stderr);
    commandsLogger << "> " << cmd << endl;
    if (response == nullptr) {
        utils::myFlush(pipe_out_vlc[READ], "", false);
        return nullptr;
    }
    *response = utils::myFlush(pipe_out_vlc[READ]);
    *response = regex_replace(*response, regex("\r"), "");
    *response = regex_replace(*response, regex("\n+$"), "");
    commandsLogger << "-------------------------------------------" << endl;
    commandsLogger << *response << endl;
    commandsLogger << "-------------------------------------------" << endl;
    return response;
}


template<class T>
void print_vector(const vector<T> &hhmmss_stop) {
    for (T i: hhmmss_stop) {
        cout << i << "->";
    }
    cout << "." << endl;
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
        vector<size_t> lineStartsTarget;
        vector<size_t> lineStartsSource;
        ostream *tell2_ptr;
        if (was_exist) {
            while (getline(trg, line)) {
                lines_num++;
                getline(src, line);
                lineStartsTarget.push_back(trg.tellg());
                lineStartsSource.push_back(src.tellg());
                if (line.starts_with("Format: ")) {
                    string sformat = utils::split(line)[1];
                    format = utils::split(sformat, ",");
                    text_pattern = foo(format, "Text", true);
                    time_pattern = foo(format, "Start");
                    stop_pattern = foo(format, "End");
                }
            }
            lineStartsTarget.pop_back();
            trg.close();
            trg.open(filename_trg, ios::out | ios::app);
        }

        debug << "loaded " << lines_num << " lines" << endl;
//        auto *tell_ptr = file_with_timestamp("tell");
//        auto &tell = *tell_ptr;
        while (getline(src, line)) {
            lineStartsTarget.push_back(trg.tellg());
            lineStartsSource.push_back(src.tellg());
            debug << "aaaaaa:" << src.tellg() << endl;

            if (!stop) {
                if (line.starts_with("[")) {
                }
                else if (line.starts_with("Format: ")) {
                    string sformat = utils::split(line)[1];
                    format = utils::split(sformat, ",");
                    text_pattern = foo(format, "Text", true);
                    time_pattern = foo(format, "Start");
                    stop_pattern = foo(format, "End");
                }
                else if (line.starts_with("Dialogue: ")) {
                    smatch sm_text = getRegex(line, text_pattern);
                    vector<int> hhmmss_start = getVector(line, time_pattern);

                    vector<int> hhmmss_end = getVector(line, stop_pattern);
                    int cur;
                    int time_to_play_to_2 = hhmmss_end[2] + hhmmss_end[1] * 60 + hhmmss_end[0] * 60 * 60;
                    int time_to_play_to_1 = hhmmss_start[2] + hhmmss_start[1] * 60 + hhmmss_start[0] * 60 * 60;
                    string translation;
                    TRANSLATE:
                    cur = vlc_get_time();

                    cout << Font(Color::yellow)<< "[" << getRegex(line, time_pattern)[3] << "] - ";
                    cout << "[" << getRegex(line, stop_pattern)[3] << "]" << Font::reset << endl;
                    cout << Font(Color::cyan) << "[# - exit, $ - replay, ^ - undo last line]" << Font::reset << endl;
                    cout << sm_text[3] << endl;
                    if ((!translation.starts_with("$")) && (time_to_play_to_1 < cur || cur + 12 < time_to_play_to_1))
                        sendCommand("seek " + to_string(time_to_play_to_1));
//                    this_thread::sleep_for(milliseconds(250));

                    cur = vlc_get_time();
                    sendCommand("play");
                    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                    debug << "sleep for " << (time_to_play_to_2 - cur) << "s" << endl;
                    sleep(time_to_play_to_2 - cur);
                    if (hhmmss_start[2] == hhmmss_end[2]) {
                        this_thread::sleep_for(milliseconds((hhmmss_start[3] + 1) * 10));
                    }
                    sendCommand("pause");
                    //sendCommand("seek " + to_string(time_to_play_to_2));

                    translation = utils::getUnboundedLine();
                    if (translation.starts_with("$")) {
                        cur = vlc_get_time();

                        int requested = atoi(translation.substr(1).c_str());
                        int time_to_rewind = requested ? requested : 10;
                        time_to_rewind = min(time_to_rewind, cur);
                        sendCommand("seek " + to_string(cur - time_to_rewind));
                        goto TRANSLATE;
                    }
                    else if (translation.starts_with("#")) {
                        stop = true;
                        break;
                    }
                    else if (translation == "") {
                        debug << "bbbbbbb:" << src.tellg() << endl;
                        lineStartsSource.pop_back();
                        debug << lineStartsSource.back() << endl;
                        src.seekg(lineStartsSource.back(), ios::beg);
                        debug << "ccccccc:" << src.tellg() << endl;
                        continue;
                    }
                    else if (translation.starts_with("^")) {
                        lineStartsSource.pop_back();
                        lineStartsSource.pop_back();
                        debug << lineStartsSource.back() << endl;
                        src.seekg(lineStartsSource.back(), ios::beg);

                        lineStartsTarget.pop_back();
                        trg.close();
                        truncate(filename_trg.c_str(), lineStartsTarget.back());
                        lineStartsTarget.pop_back();
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
            debug << "ggggggg:" << src.tellg() << endl;

        }
        trg.close();
        src.close();
    }
    else {
        if (!src.is_open())
            cerr << "couldn't open src '" << filename_src << "'" << endl;
    }
    sendCommand("quit");
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

smatch getRegex(const string &line, const regex &text_pattern) {
    smatch sm_text;
    regex_match(line, sm_text, text_pattern);
    return sm_text;
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

std::string string_to_hex(const std::string &input) {
    static const char hex_digits[] = "0123456789ABCDEF";

    std::string output;
    output.reserve(input.length() * 3);
    for (unsigned char c: input) {
        output.push_back('x');
        output.push_back(hex_digits[c >> 4]);
        output.push_back(hex_digits[c & 15]);
    }
    return output;
}

void process_break_line(const string &origin, string &translation) {
    utils::replace(translation, "@", " \\N");
    size_t n = translation.length();
    if (translation.contains("\\N")) {
        if (translation.ends_with("\\N"))
            translation = translation.substr(0, n - 2);
    }
    else if (translation.contains(" ")) {
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

    while (translation.find_last_of("\n ") == translation.length() - 1)
        translation = translation.substr(0, translation.length() - 1);

    if (endsWithPisuk(origin) && !endsWithPisuk(translation))
        translation += origin[origin.length() - 1];

    vector<string> lines = utils::split(translation, " \\N");
    string res;

    for (int i = 0; i < lines.size(); i++) {
        string line = lines[i];

        while (line[line.length() - 1] == ' ')
            line = line.substr(0, line.length() - 1);

        n = line.length();
        if (endsWithPisuk(line)) {
            line = line[n - 1] + line.substr(0, n - 1);
        }
        res += line;
        if (i < lines.size() - 1)
            res += " \\N";
    }
    translation = res;
}

bool endsWithPisuk(const string &line) {
    string pisuk = ",.!?:;\"\\/'";

    return line.find_last_of(pisuk, line.length()) == line.length() - 1;
}


regex foo(vector<string> &format, const string &title) {
    return foo(format, title, false);
}

regex foo(vector<string> &format, const string &title, bool include_commas) {
    int text_index = (int) find_index(format, title);
    if (text_index < 0)
        return {};
    string c = ".";
    string parts_after = to_string(format.size() - text_index - 1);
    if (!include_commas) {
        c = "[^,]";
        parts_after = parts_after + ",";
    }

    string pattern = "^(Dialogue: ([^,]*,){" + to_string(text_index) + "})(" + c + "*)((,[^,]*){" + parts_after + "})$";
    return regex(pattern);
}

unsigned long find_index(vector<string> &v, const string &element) {
    unsigned long index = find(v.begin(), v.end(), element) - v.begin();
    if (index >= v.size())
        return -1;
    return index;
}

void step2(const vector<Path> &paths, const char *track_num) {
    for (Path path: paths) {
        mkvExtract(path, track_num);
    }
}

vector<Path> get_eng_sub_paths(const char *src) {
    /**
     * get all tracks of a specific mkv file.
     */
    vector<Path> v;
    for (const auto &entry: fs::directory_iterator(src)) {
        const Path &path = entry.path();
        if (path.extension() == ".mkv")
            v.push_back(path);
    }
    return v;
}

void step1_extract_all_tracks_of_sample(const string &path) {
    Path res("/");
    for (int i = 0; !res.empty(); i++) {
        string si = to_string(i);
        res = mkvExtract(path, si);
    }
}

Path mkvExtract(const Path &path, size_t si) {
    return mkvExtract(path, to_string(si));
}


char *to_system_cmd(string args[]) {
    size_t sum_size = 0;
    for (int i = 0; !args[i].empty(); ++i) {
        sum_size += args[i].length() + 1;
    }
    char *cmd = new char[sum_size];
    char *p = cmd;
    for (int i = 0; !args[i].empty(); ++i) {
        string arg = args[i];
        size_t size = arg.length();
        memcpy(p, arg.c_str(), size);
        p += size;
        *(p++) = ' ';
    }
    *(p - 1) = '\0';
    return cmd;
}

Path mkvExtract(const Path &path, const string &si) {

    int res;

    Path p = path.filename().replace_extension("ass");
    p = si + "_" + p.string();

    string s1 = path.string();
    string s2 = (si + ":" + p.string());
    char ss1[s1.length()];
    strcpy(ss1, s1.c_str());
    char ss2[s2.length()];
    strcpy(ss2, s2.c_str());
    debug << "PATH:" << getenv("PATH") << endl;
    string pathh = "/usr/local/bin/";
    char *scmds[] = {(char *) "mkvextract", ss1, (char *) "tracks", ss2, (char *) "-r", (char *) "debug.log", nullptr};
    //string scmds[] = { "mkvextract", ss1, "tracks", ss2, ">>/dev/null", "2>>/dev/null", ""};
//    string scmds[] = {"mkvextract", s1, "tracks", s2, ""};
    char *cmd = to_system_cmd(scmds);
    string scmd = cmd;
    //res = system(cmd);
#ifdef WIN32
    int success = spawnv(P_WAIT, sCompiler.c_str(), argv);
#else
    pid_t pid;
    char out[] = "mkvextract_stdout.log";
    char err[] = "mkvextract_stderr.log";
    int bu_out;
    int bu_err;
    FILE *n_out, *n_err;
    vector<string> paths;
    debug << "[" << getpid() << "] " << "from pid:" << getpid() << endl;
    switch (pid = fork()) {
        case -1:
            error("Error using fork()");
            break;
        case 0:
            debug << "[" << getpid() << "] " << "#child start " << endl;

            mkstemp(out);
            mkstemp(err);
            bu_out = dup(STDOUT_FILENO);
            bu_err = dup(STDERR_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            n_out = fopen(out, "w");
            n_err = fopen(err, "w");

            execvp(scmds[0], scmds);
            debug << "[" << getpid() << "] " << "#execvp mkvextract ERROR! " << errno << endl;
            debug << "Please, make sure that the mkvextract executable is in one of the following paths:" << endl;
            paths = utils::split(getenv("PATH"), ":");

            for (const auto &path_0: paths) {
                cout << path_0 << endl;
            }
            fclose(n_out);
            fclose(n_err);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            dup(bu_out);
            dup(bu_err);
            close(bu_out);
            close(bu_err);
            debug << "[" << getpid() << "] " << "#execvp mkvextract ERROR! " << errno << endl;
            debug << "Please, make sure that the mkvextract executable is in one of the following paths:" << endl;
            paths = utils::split(getenv("PATH"), ":");

            for (const auto &path_0: paths) {
                debug << path_0 << endl;
            }
            error("mkvextract couldn't run", errno);
            break;
        default:
            debug << "[" << getpid() << "] " << "parent start wait(...)==" << pid << endl;
            int status;
            int waited = wait(&status);
            debug << "[" << getpid() << "] " << "parent end wait(" << status << ")=" << waited;

            if (waited != pid) {
                debug << "[" << getpid() << "] " << "did it?" << endl;
                error("Error using wait(" + to_string(status) + ")");
            }
            debug << "[" << getpid() << "] " << "did it!" << endl;
            res = WEXITSTATUS(status);
            debug << "[" << getpid() << "] " << "res: " << res << endl;

    }
#endif
    delete[] (cmd);
    if (res == 0)
        return p;
    else
        return Path();
}
