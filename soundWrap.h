//
// Created by nikolaj on 1/12/26.
//

#ifndef COUNTRYBRAWL_SOUNDWRAP_H
#define COUNTRYBRAWL_SOUNDWRAP_H

///At what scale does sounds become inaudible
#define SOUND_SCALE_LIMIT 0.015625

#include <filesystem>
#include <SDL2/SDL_mixer.h>

namespace fs = std::filesystem;

class SoundWrap {
public:
    explicit SoundWrap(const fs::path& path);
    SoundWrap(SoundWrap&& other) noexcept ;
    SoundWrap& operator=(SoundWrap&& other) noexcept ;
    ~SoundWrap();

    //Play a sound, if the source is on screen
    void play(int x=512, int y=512, int screenWidth=1024, int screenHeight=1024, double scale=1.0,bool silenceAtScaleout=true) const ;

    [[nodiscard]] Mix_Chunk* getSound() const {return sound;}

    void attenuate(double factor) {
        factor = factor * factor * (3.0f - 2.0f * factor);
        Mix_VolumeChunk(sound, std::max(0,std::min( static_cast<int>(factor * MIX_MAX_VOLUME),MIX_MAX_VOLUME)));
    }
private:
    Mix_Chunk* sound;
};


#endif //COUNTRYBRAWL_SOUNDWRAP_H