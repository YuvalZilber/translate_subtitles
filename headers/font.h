//
// Created by Yuval Zilber on 07/03/2023.
//

#ifndef EXTRACT_SUBTITLES_2_FONT_H
#define EXTRACT_SUBTITLES_2_FONT_H


#include <ostream>
#include <string>
enum Style {
    bold=1, underline = 4, inverse = 7
};//off = +20
enum Color {
    black, red, green, yellow, blue, magenta, cyan, white
};//BG = +10

class Font {
private:
    Font(std::string value);
protected:
    std::string value;
public:
    Font(int value);
    Font(Color color);

    operator const char*();

    std::ostream &operator()(std::ostream &);
    static Font background(Color color);
    static Font reset;
};


#endif //EXTRACT_SUBTITLES_2_FONT_H
