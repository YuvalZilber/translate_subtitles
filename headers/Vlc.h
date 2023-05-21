//
// Created by yuvalzilber on 5/16/23.
//

#ifndef EXTRACT_SUBTITLES_2_VLC_H
#define EXTRACT_SUBTITLES_2_VLC_H


#include "InteractiveShell.h"

class Vlc : public InteractiveShell {
public:
    Vlc(const std::string &videoFile, const std::string &subtitle_file);

    std::string *sendCommand(const std::string &cmd, std::string *response = nullptr) override;
};


#endif //EXTRACT_SUBTITLES_2_VLC_H
