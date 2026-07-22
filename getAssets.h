//
// Created by nikolaj on 6/1/26.
//

#ifndef DUNGEONSKETCH_GETASSETS_H
#define DUNGEONSKETCH_GETASSETS_H

#include <filesystem>

namespace fs = std::filesystem;

///Path where assets is stored, defined in a seperate header so we can easily move it elsewhere
inline fs::path assetsPath() {
    return fs::path("assets");
}

#endif //DUNGEONSKETCH_GETASSETS_H