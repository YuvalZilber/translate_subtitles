//
// Created by yuvalzilber on 5/16/23.
//

#include "../headers/Vlc.h"


Vlc::Vlc(const string &videoFile, const string &subtitle_file) :
        InteractiveShell("vlc -I rc --extraintf qt --sub-file \"" + subtitle_file + "\" \"" + videoFile+"\"",
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