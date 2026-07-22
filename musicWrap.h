//
// Created by nikolaj on 1/6/26.
//

#ifndef COUNTRYBRAWL_MUSICWRAP_H
#define COUNTRYBRAWL_MUSICWRAP_H

#include <filesystem>
#include <SDL2/SDL_mixer.h>

namespace fs = std::filesystem;


///A wrapper class for music, which manages movement and deletion of the track
class musicWrap {
public:
    musicWrap(const fs::path& path, const std::string& name);
    musicWrap(musicWrap&& other) noexcept ;
    musicWrap& operator=(musicWrap&& other) noexcept ;
    ~musicWrap();

    [[nodiscard]] double getLength() const {return length;}
    [[nodiscard]] Mix_Music* getMusic() const {return music;}
    [[nodiscard]] const std::string& getName() const {return name;}

private:
    Mix_Music* music;
    double length;
    std::string name;
};


#endif //COUNTRYBRAWL_MUSICWRAP_H