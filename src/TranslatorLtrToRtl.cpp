//
// Created by yuvalzilber on 5/30/23.
//

#include "TranslatorLtrToRtl.h"

#include <utility>

TranslatorLtrToRtl::TranslatorLtrToRtl(Vlc &vlc, string filename_src, string t) :
        player(vlc),
        subtitleFile(std::move(filename_src)),
        targetFile(std::move(t)) {
}

void TranslatorLtrToRtl::translate() {
    const string &filename_trg = targetFile;

    cout << filename_trg << endl;
    ifstream src(subtitleFile);
    fstream trg;
    bool target_already_exists = fs::exists(filename_trg);
    if (target_already_exists) {
        cout << "found file from last run." << endl << "loading..." << endl << endl;
        trg.open(filename_trg, fstream::out | fstream::in);
    } else
        trg.open(filename_trg, fstream::out);
    if (!trg) {
        cerr << "Can'targetFile open target file: '" << filename_trg << "'" << endl;
        cerr << "cwd+filename: '" << utils::pwd() << "/" << filename_trg << "'" << endl;
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
                    text_pattern = utils::getPatternByTitle(format, "Text");
                    time_pattern = utils::getPatternByTitle(format, "Start");
                    stop_pattern = utils::getPatternByTitle(format, "End");
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
                text_pattern = utils::getPatternByTitle(format, "Text");
                time_pattern = utils::getPatternByTitle(format, "Start");
                stop_pattern = utils::getPatternByTitle(format, "End");
            } else if (line.starts_with("Dialogue: ")) {
                smatch sm_text = utils::getRegex(line, text_pattern);
                vector<int> hhmmss_start = getTimeVector(line, time_pattern);

                vector<int> hhmmss_end = getTimeVector(line, stop_pattern);
                int cur;
                int time_to_play_to_2 = hhmmss_end[2] + hhmmss_end[1] * 60 + hhmmss_end[0] * 60 * 60;
                int time_to_play_to_1 = hhmmss_start[2] + hhmmss_start[1] * 60 + hhmmss_start[0] * 60 * 60;
                if (time_to_play_to_2 > time_to_play_to_1 + 1)
                    time_to_play_to_2--;
                string translation;
                TRANSLATE:
                cur = player.get_time();

                cout << Font(Color::yellow) << "[" << extractByTitlePattern(line, time_pattern) << "] - ";
                cout << "[" << extractByTitlePattern(line, stop_pattern) << "]" << Font::reset << endl;
                cout << Font(Color::cyan) << "[# - exit, $ - replay, ^ - undo last line]" << Font::reset << endl;
                cout << sm_text[3] << endl;
                if ((!translation.starts_with("$")) && (time_to_play_to_1 < cur || cur + 12 < time_to_play_to_1))
                    player.seek(time_to_play_to_1);
//                    this_thread::sleep_for(milliseconds(250));

                cur = player.get_time();
                int play_time = (time_to_play_to_2 - cur) * 1000;
                if (hhmmss_start[2] == hhmmss_end[2]) {
                    play_time += (hhmmss_start[3] + 1) * 10;
                }
                player.play_for(play_time);

                translation = utils::getUnboundedLine();
                if (translation.starts_with("$")) {
                    cur = player.get_time();

                    int requested = atoi(translation.substr(1).c_str()); // NOLINT(cert-err34-c)
                    int time_to_rewind = requested ? requested : 10;
                    time_to_rewind = min(time_to_rewind, cur);
                    player.seek(cur - time_to_rewind);
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
            cerr << "couldn'targetFile open src '" << subtitleFile << "'" << endl;
        if (!trg.is_open())
            cerr << "couldn'targetFile open trg '" << filename_trg << "'" << endl;
    }
    player.quit();
    cout << "bye!" << endl;
}

vector<int> TranslatorLtrToRtl::getTimeVector(const string &line, regex &time_pattern) {
    smatch sm_time;
    regex_match(line, sm_time, time_pattern);
    string time_s = sm_time[3].str();
    if (!time_s.starts_with("0:")) {
        cerr << "don'targetFile handle hours yet" << endl;
        exit(1);
    }
    time_s[time_s.find('.')] = ':';
    vector<string> hhmmss_start_s = utils::split(time_s, ":");
    vector<int> hhmmss_start;
    transform(hhmmss_start_s.begin(), hhmmss_start_s.end(), back_inserter(hhmmss_start), utils::to_int);
    return hhmmss_start;
}

string TranslatorLtrToRtl::extractByTitlePattern(const string &line, const regex &title_pattern) {
    return utils::getRegex(line, title_pattern)[3];
}

void TranslatorLtrToRtl::process_break_line(string &origin, string &translation) {
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

    translation = regex_replace(translation, regex(R"((^(\s|\\N)+)|((\s|\\N)+$))"), "");

    if (utils::endsWithPunctuation(origin) && !utils::endsWithPunctuation(translation))
        translation += utils::getPunctuationAtEnd(origin);

    vector<string> lines = utils::split(translation, " \\N");
    string res = flipPunctuationIfNeeded(n, lines);
    translation = res;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"

string TranslatorLtrToRtl::flipPunctuationIfNeeded(size_t n, const vector<string> &lines) {
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
            if (utils::endsWithPunctuation(line)) {
                string punc = utils::getPunctuationAtEnd(line);
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

#pragma clang diagnostic pop