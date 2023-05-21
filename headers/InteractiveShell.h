//
// Created by yuvalzilber on 5/16/23.
//

#ifndef EXTRACT_SUBTITLES_2_INTERACTIVESHELL_H
#define EXTRACT_SUBTITLES_2_INTERACTIVESHELL_H


#include <cstdio>
#include <string>
#include <regex>
#include <wait.h>

#include "utils.h"
#include "consts.h"

#ifdef WINDOWS
#else

#include <unistd.h>
#include <wait.h>

#define GetCurrentDir getcwd
#endif

class InteractiveShell {
public:
    explicit InteractiveShell(const std::string &cmd, std::string in="", std::string out="", std::string err="");

#pragma clang diagnostic push
#pragma ide diagnostic ignored "google-default-arguments"

    virtual std::string *sendCommand(const std::string &cmd, std::string *response = nullptr);

#pragma clang diagnostic pop

    int shellPid;
    int parentPid;

    int wait();

private:

    void initShell(const string &cmd);

    int *shell_in;
    int *shell_out;
    int *shell_err;

    string file_std_in;
    string file_std_out;
    string file_std_err;

    [[nodiscard]] static char **split_cmd(const string &cmd);

    static void childTransferStdStream(FILE *fd, const int *new_pipe, int side_to_use, const std::string &filename);

    static void closeUnusedSideOfPipe(const int *new_pipe, int side_to_use);
};


#endif //EXTRACT_SUBTITLES_2_INTERACTIVESHELL_H
