//
// Created by Yuval Zilber on 10/03/2023.
//

#include "Logger.h"

namespace logger {
    Logger::Logger(const std::string &name, const std::string &filename) :
            m_nb(name, filename),
            std::ostream(&m_nb) {}

}