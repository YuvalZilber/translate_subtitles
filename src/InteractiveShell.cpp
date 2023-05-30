//
// Created by yuvalzilber on 5/16/23.
//

#include <utility>
#include <strstream>

#include "InteractiveShell.h"

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

    pid_t pid = fork();
    if (pid == -1) {
        utils::error("couldn't fork:" + cmd);
    }
    if (pid == 0) {
        char **cmd_array = split_cmd(cmd);
        debug << "pid child: " << to_string(getpid()) << endl;
        shellPid = getpid();
        childTransferStdStream(stdin, shell_in, READ, file_std_in);
        childTransferStdStream(stdout, shell_out, WRITE, file_std_out);
        childTransferStdStream(stderr, shell_err, WRITE, file_std_err);

        int res = execvp(cmd_array[0], cmd_array);
        debug << "ERROR!!! " << res << endl;


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
    string pattern = R"(("[^"]*")|('[^']*')|([^ ]+))";
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
InteractiveShell::sendCommand(const string &cmd, std::string *response) const { // NOLINT(google-default-arguments)
    write(shell_in[WRITE], (cmd + "\n").c_str(), cmd.length() + 1);
    fflush(stderr);
    if (response == nullptr) {
        utils::flushTo(shell_out[READ], "", false);
        return nullptr;
    }
    *response = utils::flushTo(shell_out[READ]);
    return response;
}

int InteractiveShell::wait() const {
    if (getpid() != parentPid)
        utils::error("Illegel state! only the parent of the shell is allowed to wait for it");
    int res;

    debug << "[" << to_string(getpid()) << "] " << "parent start wait(...)==" << to_string(shellPid) << endl;
    int status;
    int waited = ::wait(&status);
    debug << "[" << to_string(getpid()) << "] " << "parent end wait(" << to_string(status) << ")=" << to_string(waited)
          << endl;

    if (waited != shellPid) {
        debug << "[" << to_string(getpid()) << "] " << "did it?" << endl;
        utils::error("Error using wait(" + to_string(status) + ")");
    }
    debug << "[" << to_string(getpid()) << "] " << "did it!" << endl;
    res = WEXITSTATUS(status);
    debug << "[" << to_string(getpid()) << "] " << "res: " << to_string(res) << endl;
    return res;
}

InteractiveShell::~InteractiveShell() {
    freeArray(shell_in);
    freeArray(shell_out);
    freeArray(shell_err);
}

void InteractiveShell::freeArray(int *&arr) {
    if (arr) {
        delete[] arr;
        arr = nullptr;
    }
}


InteractiveShell::InteractiveShell(InteractiveShell &&other) noexcept:
        InteractiveShell() {
    swap(other);
}

InteractiveShell &InteractiveShell::operator=(InteractiveShell other) {
    swap(*this, other);  // trade our resources for x's
    return *this;    // our (old) resources get destroyed with x
}

InteractiveShell &InteractiveShell::operator=(InteractiveShell &&other) noexcept {
    swap(*this, other);  // trade our resources for x's
    return *this;
}

void InteractiveShell::swap(InteractiveShell &other) {
    std::swap(shell_in, other.shell_in);
    std::swap(shell_out, other.shell_out);
    std::swap(shell_err, other.shell_err);
    std::swap(shellPid, other.shellPid);
    std::swap(parentPid, other.parentPid);
    std::swap(file_std_in, other.file_std_in);
    std::swap(file_std_out, other.file_std_out);
    std::swap(file_std_err, other.file_std_err);
}

InteractiveShell::InteractiveShell() : shellPid(),
                                       parentPid(),
                                       shell_in(),
                                       shell_out(),
                                       shell_err(),
                                       file_std_in(),
                                       file_std_out(),
                                       file_std_err() {

}



