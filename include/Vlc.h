//
// Created by yuvalzilber on 5/16/23.
//

#ifndef EXTRACT_SUBTITLES_2_VLC_H
#define EXTRACT_SUBTITLES_2_VLC_H


#include "InteractiveShell.h"

class Vlc : public InteractiveShell {
public:
    Vlc(const std::string &videoFile, const std::string &subtitle_file);

    int get_time() const;

    void seek(int time) const;

    void play() const;

    void pause() const;

    void play_for(int time) const;

    void quit() const;

    string help() const;

private:
    std::string *
    sendCommand(const std::string &cmd,
                std::string *response = nullptr) const override; // NOLINT(google-default-arguments)

};


#endif //EXTRACT_SUBTITLES_2_VLC_H
