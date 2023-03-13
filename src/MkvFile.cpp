//
// Created by Yuval Zilber on 09/03/2023.
//

#include "../headers/MkvFile.h"

using namespace utils;

MkvFile::MkvFile(Path &filepath) : filepath_(filepath) {
}

Path MkvFile::ChooseSubtitleFile() {
    if (sub_file_.empty()) {
        string track_num = this->GetTrackNum();
        if (track_num != "") {
            return MkvExtract(track_num);
        }
    }
    return sub_file_;
}

typedef struct {
    size_t index;
    string dialog;
} track_option;

string MkvFile::GetTrackNum() {
    Path file{filepath_};

    cout << "\nloading...";
    Path cur_sub_file("/");
    vector<track_option> options;
    for (size_t i = 0; !cur_sub_file.empty(); i++) {
        utils::load(i % 3 + 1);
        cur_sub_file = MkvExtract(to_string(i));
        string max_dialog = LongestDialog(cur_sub_file);
        remove(cur_sub_file);
        if (!max_dialog.empty()) {
            options.push_back({i, max_dialog});
        }
    }
    utils::EndLoad();
    debug << "[" << getpid() << "] " << "finished mkvExtract(...)" << endl;
    cout << Font(Style::bold) << "Choose language or enter a subtitle filename" << Font::reset << endl;
    for (const auto &option: options) {
        cout << "O " << Font(Style::bold) << "[" << option.index << "] " << Font::reset << option.dialog << endl;
    }
    string line;
    int track_num = -1;
    while (track_num == -1) {
        cout << Font(Style::bold) << "your choice: " << Font::reset;
        line = utils::getUnboundedLine();
        if (exists(Path(line))) {
            return line;
        }
        bool found = false;
        try {
            //todo: fix [1] case (with brackets).
            if (regex_match(line.c_str(), regex(R"(^\[?(\d+)\]?$)"))) {
                track_num = to_int(line);
                for (const auto &option: options) {
                    if (option.index == track_num) {
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
                cout << "[" << option.index << "] " << option.dialog << endl;
            }
        }

        if (!found)
            track_num = -1;
    }
    return line;
}


string MkvFile::LongestDialog(Path &p) {
    ifstream src(p);
    string line;
    string max_dialog;
    vector<string> format;
    regex text_pattern;

    while (getline(src, line)) {
        if (line[0] < 0 && line[0] != -17)
            break;
        if (line.starts_with("Format: ")) {
            string sformat = split(line)[1];
            format = split(sformat, ",");
            text_pattern = GetPatternByTitle(format, "Text");
        }
        else if (line.starts_with("Dialogue: ")) {
            string dialog = GetRegex(line, text_pattern)[3];
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


Path MkvFile::MkvExtract(const string &si) {
    int res = -1;

    Path p = filepath_.filename().replace_extension("ass");
    p = si + "_" + p.string();

    string s1 = filepath_.string();
    string s2 = (si + ":" + p.string());
    char ss1[s1.length()];
    strcpy(ss1, s1.c_str());
    char ss2[s2.length()];
    strcpy(ss2, s2.c_str());
    debug << "PATH:" << getenv("PATH") << endl;
    string pathh = "/usr/local/bin/";
    char *scmds[] = {(char *) "mkvextract", ss1, (char *) "tracks", ss2, (char *) "-r", (char *) "debug.log", nullptr};
//    char *cmd = to_system_cmd(scmds);
//    string scmd = cmd;
#ifdef WIN32
    int success = spawnv(P_WAIT, s_compiler.c_str(), argv);
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
//    delete[] (cmd);
    if (res == 0)
        return consts::output / p;
    else
        return Path();
}

string MkvFile::get_digits_of_choice(const string& choice){
    regex pattern{R"(^\[?(\d+)\]?$)"};
    smatch match;
    if(regex_match(choice, match, pattern))
        return match[0];

    return "";
}
