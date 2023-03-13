//
// Created by Yuval Zilber on 10/03/2023.
//

#include "../headers/Logger.h"

namespace logger {
    Logger::Logger(const std::string &name) : m_nb(name), std::ostream(&m_nb) {}
}