//
// Created by Yuval Zilber on 09/03/2023.
//

#include "MkvFile.h"
#include "InteractiveShell.h"

using namespace utils;

MkvFile::MkvFile(Path &filepath) : filepath_(filepath) {
}

Path MkvFile::chooseSubtitleFile() {
    if (sub_file_.empty()) {
        string trackNum = this->getTrackNum();
        if (!trackNum.empty()) {
            return mkvExtract(trackNum);
        }
    }
    return sub_file_;
}

typedef struct {
    size_t index;
    string dialog;
} track_option;

string MkvFile::getTrackNum() {
    Path file{filepath_};

    cout << "\nloading...";
    Path curSubFile("/");
    vector<track_option> options;
    for (size_t i = 0; !curSubFile.empty(); i++) {
        utils::load(i % 3 + 1);
        curSubFile = mkvExtract(to_string(i));
        string maxDialog = longestDialog(curSubFile);
        remove(curSubFile);
        if (!maxDialog.empty()) {
            options.push_back({i, maxDialog});
        }
    }
    utils::endLoad();
    debug << "[" << to_string(getpid()) << "] " << "finished mkvExtract(...)" << endl;
    if (options.empty()) {
        cerr << "no subtitle file found" << endl;
        exit(1);
    }
    if (options.size() == 1) {
        return to_string(options[0].index);
    }
    cout << Font(Style::bold) << "Choose language or enter a subtitle filename" << Font::reset << endl;
    for (const auto &option: options) {
        cout << "O " << Font(Style::bold) << "[" << option.index << "] " << Font::reset << option.dialog << endl;
    }
    string line;
    int trackNum = -1;
    while (trackNum == -1) {
        cout << Font(Style::bold) << "your choice: " << Font::reset;
        line = utils::getUnboundedLine();
        if (exists(Path(line))) {
            return line;
        }
        bool found = false;
        try {
            //todo: fix [1] case (with brackets).
            if (regex_match(line.c_str(), regex(R"(^\[?(\d+)\]?$)"))) {
                trackNum = to_int(line);
                for (const auto &option: options) {
                    if (option.index == trackNum) {
                        found = true;
                        break;
                    }
                }
            }
        }
        catch (exception &e) {
            cout << "-------------------------------------------------------------------" << endl;
            cout << "it is not a number, please enter the number of the wanted language:" << endl;
            cout << Font(Style::bold) << "Choose language or enter a subtitle filename" << Font::reset << endl;
            for (const auto &option: options) {
                cout << "[" << to_string(option.index) << "] " << option.dialog << endl;
            }
        }

        if (!found)
            trackNum = -1;
    }
    return line;
}


string MkvFile::longestDialog(Path &p) {
    ifstream src(p);
    string line;
    string maxDialog;
    vector<string> format;
    regex textPattern;

    while (getline(src, line)) {
        if (line[0] < 0 && line[0] != -17)
            break;
        if (line.starts_with("Format: ")) {
            string sformat = line.substr(line.find(' '));
            format = split(sformat, ",");
            textPattern = getPatternByTitle(format, "Text");
        } else if (line.starts_with("Dialogue: ")) {
            auto reg = getRegex(line, textPattern);
            auto k = reg[3];
            string dialog{k};
            dialog = regex_replace(dialog, regex(R"(\{\\[^}]*\})"), "");
            if (dialog.contains("\\N") || dialog.contains("<"))
                continue;
            if (dialog.length() > maxDialog.length()) {
                maxDialog = dialog;
            }
        }
    }
    return maxDialog;
}


Path MkvFile::mkvExtract(const string &si) {
    int res = -1;

    Path p = filepath_.filename().replace_extension("ass");
    p = si + "_" + p.string();
#ifdef WIN32
    int success = spawnv(P_WAIT, s_compiler.c_str(), argv);
#else
    string out = utils::logFilename("mkvextract_stdout");
    string err = utils::logFilename("mkvextract_stderr");
    vector<string> paths;
    debug << "[" << to_string(getpid()) << "] " << "from pid:" << to_string(getpid()) << endl;
    const string &logFile = utils::logFilename("debug");
    const string &cmd = "mkvextract \"" + filepath_.string() + "\" tracks " + si + ":" + p.string() + " -r " + logFile;
    InteractiveShell shell(cmd, "", out, err);
    pid_t pid = shell.shellPid;
    if (getpid() != shell.shellPid) {
        // parent
        res = shell.wait();
    } else {
        // child ERROR
        debug_mode = true;
        debug << "[" << to_string(getpid()) << "] " << "#execvp mkvextract ERROR! " << to_string(errno) << endl;
        debug << "[ERR] Please,, make sure that the 'mkvextract' executable is in one of the following paths:"
              << endl;
        paths = utils::split(getenv("PATH"), ":");

        for (const auto &path_0: paths) {
            debug << path_0 << endl;
        }

        debug << "[" << to_string(getpid()) << "] " << "#execvp mkvextract ERROR! " << to_string(errno) << endl;
        error("mkvextract couldn't run", errno);
    }

#endif
//    delete[] (cmd);
    if (res == 0)
        return output / p;
    else
        return Path();
}

string MkvFile::getDigitsOfChoice(const string &choice) {
    regex pattern{R"(^\[?(\d+)\]?$)"};
    smatch match;
    if (regex_match(choice, match, pattern))
        return match[0];

    return "";
}
