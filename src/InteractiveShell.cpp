//
// Created by yuvalzilber on 5/16/23.
//

#include <utility>
#include <strstream>

#include "../headers/InteractiveShell.h"


InteractiveShell::InteractiveShell(const std::string &cmd, std::string in, std::string out, std::string err) :
        shellPid(-1),
        parentPid(getpid()),
        shell_in(new int[2]),
        shell_out(new int[2]),
        shell_err(new int[2]),
        file_std_in(std::move(in)),
        file_std_out(std::move(out)),
        file_std_err(std::move(err)) {
    pipe(shell_in);
    pipe(shell_out);
    pipe(shell_err);
    initShell(cmd);
}

void InteractiveShell::initShell(const string &cmd) {
    debug << "InteractiveShell - create: " << cmd << endl;
    debug << "cwd: " << utils::get_cwd() << endl;
    char **cmd_array = split_cmd(cmd);

    pid_t pid = fork();
    if(pid==-1){
        utils::error("couldn't fork:"+cmd);
    }
    if (pid == 0) {
        debug << "pid child: " << to_string(getpid()) << endl;
        shellPid = getpid();
        childTransferStdStream(stdin, shell_in, READ, file_std_in);
        childTransferStdStream(stdout, shell_out, WRITE, file_std_out);
        childTransferStdStream(stderr, shell_err, WRITE, file_std_err);

        int res = execvp(cmd_array[0], cmd_array);
        debug << "ERROR!!!" << res;
    } else {
        debug << "pid parent: " << to_string(getpid()) << endl;
        shellPid = pid;

        closeUnusedSideOfPipe(shell_in, WRITE);
        closeUnusedSideOfPipe(shell_out, READ);
        closeUnusedSideOfPipe(shell_err, READ);
    }
}

void InteractiveShell::childTransferStdStream(FILE *stream, const int *new_pipe, int side_to_use,
                                              const std::string &filename) {
    if (filename.empty()) {
        closeUnusedSideOfPipe(new_pipe, side_to_use);
        int new_fd = new_pipe[side_to_use];
        dup2(new_fd, fileno(stream));
    } else {
        const string modes[] = {"r", "w+"};
        std::freopen(filename.c_str(), modes[side_to_use].c_str(), stream);
    }
}

void InteractiveShell::closeUnusedSideOfPipe(const int *new_pipe, int side_to_use) {
    close(new_pipe[WRITE + READ - side_to_use]);
}

char **InteractiveShell::split_cmd(const string &cmd) {
    string pattern = "(\"[^\"]*\")|('[^']*')|([^ ]+)";
    smatch match;

    vector<string> cmd_vector = utils::regexFindAll(cmd, pattern);
    size_t n_args = cmd_vector.size();
    char **cmd_array = new char *[n_args + 1];
    cmd_array[n_args] = nullptr;
    size_t i = 0;
    for (const string &arg: cmd_vector) {
        string na = utils::trimRegex(arg, "['\"]");
        cmd_array[i++] = utils::string_to_char_array(na);
    }
    return cmd_array;
}

std::string *
InteractiveShell::sendCommand(const string &cmd, std::string *response) { // NOLINT(google-default-arguments)
    write(shell_in[WRITE], (cmd + "\n").c_str(), cmd.length() + 1);
    fflush(stderr);
    if (response == nullptr) {
        utils::flushTo(shell_out[READ], "", false);
        return nullptr;
    }
    *response = utils::flushTo(shell_out[READ]);
    return response;
}

int InteractiveShell::wait() {
    if(getpid()!=parentPid)
        utils::error("Illegel state! only the parent of the shell is allowed to wait for it");
    int res;

    debug << "[" << getpid() << "] " << "parent start wait(...)==" << shellPid << endl;
    int status;
    int waited = ::wait(&status);
    debug << "[" << getpid() << "] " << "parent end wait(" << status << ")=" << waited;

    if (waited != shellPid) {
        debug << "[" << getpid() << "] " << "did it?" << endl;
        utils::error("Error using wait(" + to_string(status) + ")");
    }
    debug << "[" << getpid() << "] " << "did it!" << endl;
    res = WEXITSTATUS(status);
    debug << "[" << getpid() << "] " << "res: " << res << endl;
    return res;
}



