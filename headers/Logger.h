//
// Created by Yuval Zilber on 10/03/2023.
//

#ifndef EXTRACT_SUBTITLES_2_LOGGER_H
#define EXTRACT_SUBTITLES_2_LOGGER_H

#include "NullBuffer.h"
#include <string>

namespace logger {

    class Logger : public std::ostream {
    protected:
        NullBuffer m_nb;

    public:
        explicit Logger(const std::string &name = "LOG");
    };
}

#endif //EXTRACT_SUBTITLES_2_LOGGER_H
