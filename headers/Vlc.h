//
// Created by yuvalzilber on 5/16/23.
//

#ifndef EXTRACT_SUBTITLES_2_VLC_H
#define EXTRACT_SUBTITLES_2_VLC_H


#include "InteractiveShell.h"

class Vlc : public InteractiveShell {
public:
    Vlc(const std::string &videoFile, const std::string &subtitle_file);


    int get_time();

    void seek(int time);

    void play();

    void pause();

    void play_for(int time);

    void quit();

    string help();

private:
    std::string *
    sendCommand(const std::string &cmd, std::string *response = nullptr) override; // NOLINT(google-default-arguments)

};


#endif //EXTRACT_SUBTITLES_2_VLC_H
