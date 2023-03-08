//
// Created by Yuval Zilber on 07/03/2023.
//

#include "../headers/font.h"

#include <utility>

Font Font::reset(0);

Font::Font(std::string sValue) : value(std::move(sValue)) {}

Font::Font(int value) : Font("\e[" + std::to_string(value) + "m") {}

Font::Font(Color color) : Font(color + 30) {}

std::ostream &Font::operator()(std::ostream &os) {
    return os << std::string(*this);
}

Font Font::background(Color color) {
    return {color + 40};
}

Font::operator const char *() {
    return this->value.c_str();
}


