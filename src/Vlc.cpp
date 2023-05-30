//
// Created by yuvalzilber on 5/16/23.
//

#include "Vlc.h"


Vlc::Vlc(const string &videoFile, const string &subtitle_file) :
        InteractiveShell("vlc -I rc --extraintf qt --sub-file \"" + subtitle_file + "\" \"" + videoFile + "\"",
                         "", "", base_utils::logFilename("vlc_err")) {
    cout << "who am i? " << getpid() << endl;
}


std::string *Vlc::sendCommand(const string &cmd, std::string *response) {
    commandsLogger << "#############################################" << endl;
    commandsLogger << "> " << cmd << endl;
    InteractiveShell::sendCommand(cmd, response);
    if (response) {
        *response = regex_replace(*response, regex("\r"), "");
        *response = regex_replace(*response, regex("\n+$"), "");
        commandsLogger << " -------------------------------------------" << endl;
        commandsLogger << *response << endl;
        commandsLogger << "#############################################" << endl;
    }
    return response;
}

int Vlc::get_time() {
    string response;
    this->sendCommand("get_time", &response);
    response = regex_replace(response, regex("^\\D+"), "");
    if (response.empty())
        return -1;
    return utils::to_int(response);
}

void Vlc::seek(int time) {
    this->sendCommand("seek " + to_string(time));
}

void Vlc::play() {
    this->sendCommand("play");

}

void Vlc::pause() {
    this->sendCommand("pause");
}

void Vlc::play_for(int milliseconds) {
    this->play();
    this_thread::sleep_for(chrono::milliseconds(milliseconds));
    this->pause();
}

void Vlc::quit() {
    this->sendCommand("quit");
}

string Vlc::help() {
    string response;
    this->sendCommand("help", &response);
    return response;
}
