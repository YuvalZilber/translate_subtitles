//
// Created by yuvalzilber on 5/30/23.
//

#include "MainOptions.h"

#include <iostream>

using namespace std;

MainOptions::MainOptions(int argc, char *argv[]) :
        argc_(argc),
        argv_(argv) {
    appName_ = argv_[0];
    this->parse();
}


std::string MainOptions::getAppName() const {
    return appName_;
}

void MainOptions::parse() {
    Option option;
    string &sec = option.second;
    for (const char *const *i = this->begin() + 1; i != this->end(); i++) {
        const string p = *i;

        if (p[0] == '-') {
            insertOption(&option);

            option.first = p;
        } else {
            if (!sec.empty())
                sec += " ";
            sec += p;
        }
    }
    insertOption(&option);
}

void MainOptions::printOptions() const {
    auto m = options_.begin();
    int i = 0;
    if (options_.empty() && namelessValues_.empty()) {
        cout << "No parameters" << endl;
    }
    for (; m != options_.end(); m++, ++i) {
        cout << "Parameter [" << i << "] [" << (*m).first << "]->[" << (*m).second << "]" << endl;
    }
    for (const auto &value: namelessValues_) {
        cout << "Nameless value: [" << value << "]" << endl;
    }
}

const char *const *MainOptions::begin() const {
    return argv_;
}

const char *const *MainOptions::end() const {
    return argv_ + argc_;
}

const char *const *MainOptions::last() const {
    return argv_ + argc_ - 1;
}

bool MainOptions::hasKey(const std::string &key) const {
    return options_.find(key) != options_.end();
}

MainOptions::Option *MainOptions::getParamFromKey(const std::string &key) const {
    auto i = options_.find(key);
    if (i != options_.end()) {
        return ((MainOptions::Option *) (&*i));
    }
    return nullptr;
}

void MainOptions::insertOption(MainOptions::Option *option) {
    if (!option->first.empty()) {
        base_utils::tolower(option->first);
        const char *c = option->first.c_str();
        while (c[0] == '-')
            c++;
        option->first = string(c);

        options_.insert(*option);
    } else if (!option->second.empty())
        namelessValues_.push_back(option->second);

    *option = Option();
}
