//
// Created by Yuval Zilber on 09/02/2023.
//

#include "../headers/utils.h"

#define ever (;;)
using namespace std;
map<int, FILE *> utils::files;

//region split
vector<string> utils::split(const string &s, const string &delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

vector<string> utils::split(const string &s, int limit, const string &delimiter) {
    if (limit > 0) return utils::lsplit(s, limit, delimiter);
    if (limit < 0) return utils::rsplit(s, -limit, delimiter);
    return utils::split(s, delimiter);
}

vector<string> utils::rsplit(const string &s, size_t limit, const string &delimiter) {
    size_t pos_start, pos_end = s.length(), delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((res.size() < limit - 1) &&
           ((pos_start = s.rfind(delimiter, pos_end - delim_len) + delim_len) != string::npos)) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_end = pos_start - delim_len;
        res.push_back(token);
    }
    res.push_back(s.substr(0, pos_end));
    std::reverse(res.begin(), res.end());
    return res;
}

vector<string> utils::lsplit(const string &s, size_t limit, const string &delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((res.size() < limit - 1) && ((pos_end = s.find(delimiter, pos_start)) != string::npos)) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

//endregion

bool utils::replace(string &str, const string &from, const string &to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

int unblock(int fd, int flags = -1) {
    if (flags == -1)
        flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return flags;
}

int block(int fd, int flags = -1) {
    if (flags == -1)
        flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags & !O_NONBLOCK);
    return flags;
}

void utils::blockRead(int fd) {
    vector<int> fds({fd});
    blockRead(fds);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

void utils::blockRead(const vector<int> &fds) {
    for ever {
        for (auto fd: fds)
            myFlush(fd, "block");
    }
}

#pragma clang diagnostic pop

string utils::myFlush(int fd, const string &prefix, bool do_block) {
    ostringstream all;
    string line;
    auto process_line = [&all, &prefix](const string &s) {
        all << s << "\n";
        if (!prefix.empty())
            cout << "[" << prefix << "] " << s << endl;
    };

    int flags = -1;
    if (do_block) {
        flags = block(fd);
        process_line(getUnboundedLine(fd));
    }

    unblock(fd, flags);
    while ((line = getUnboundedLine(fd)) != "\3") {
        process_line(line);
    }
    fcntl(fd, F_SETFL, flags); // restore
    return all.str();
}

string utils::getUnboundedLine(FILE *file) {
    char *buffer = nullptr;
    size_t line_cap = 0;
    int k = getline(&buffer, &line_cap, file);
    if (k == -1) {
        delete(buffer);
        return "\3";
    }
    string line = buffer;
    delete (buffer);
    while (line.ends_with("\n"))
        line = line.substr(0, line.length() - 1);
    return line;
}

string utils::getUnboundedLine(int fd) {
    FILE *file = openFile(fd);
    return getUnboundedLine(file);
}

FILE *utils::openFile(int fd) {
    auto iter = files.lower_bound(fd);
    if (iter == files.end() || fd < iter->first) {    // not found
        FILE *file = fdopen(fd, "r");
        files.insert(iter, make_pair(fd, file));     // hinted insertion
    }
    return files[fd];
}
