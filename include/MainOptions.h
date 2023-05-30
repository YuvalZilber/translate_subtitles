//
// Created by yuvalzilber on 5/30/23.
//

#ifndef EXTRACT_SUBTITLES_2_MAINOPTIONS_H
#define EXTRACT_SUBTITLES_2_MAINOPTIONS_H


#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <cassert>
#include "base_utils.h"

class MainOptions {
public:
    typedef std::pair<std::string, std::string> Option;

    MainOptions(int argc, char *argv[]);

    virtual ~MainOptions() = default;

    [[nodiscard]] std::string getAppName() const;

    [[nodiscard]] bool hasKey(const std::string &) const;

    template<typename... Ts>
    [[nodiscard]] bool hasOneOfKeys(Ts... keys) const {
        static_assert((std::is_convertible_v<Ts, std::string> && ...),
                      "All args in the pack must convert to string_view.");
        const auto keysVector = std::vector<std::string>{std::forward<Ts>(keys)...};
        return std::ranges::any_of(keysVector.cbegin(), keysVector.cend(),
                                   [this](const std::string &key) { return hasKey(key); });
    }

    [[nodiscard]] Option *getParamFromKey(const std::string &) const;

    template<typename... Ts>
    [[nodiscard]] Option *getParamFromKeys(Ts... keys) const {
        static_assert((std::is_convertible_v<Ts, std::string> && ...),
                      "All args in the pack must convert to string_view.");
        const auto keysVector = std::vector<std::string>{std::forward<Ts>(keys)...};
        for (const auto &key: keysVector) {
            if (hasKey(key))
                return this->getParamFromKey(key);
        }
        return nullptr;
    }

    void printOptions() const;

    std::vector<std::string> namelessValues_;

private:
    typedef std::map<std::string, std::string> Options;

    void parse();

    void insertOption(Option *option);

    [[nodiscard]] const char *const *begin() const;

    [[nodiscard]] const char *const *end() const;

    [[nodiscard]] const char *const *last() const;

    Options options_;
    int argc_;
    char **argv_;
    std::string appName_;
};


#endif //EXTRACT_SUBTITLES_2_MAINOPTIONS_H
