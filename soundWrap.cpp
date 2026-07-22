//
// Created by nikolaj on 1/12/26.
//

#include "soundWrap.h"

#include <iostream>

SoundWrap::SoundWrap(const fs::path &path) {
    sound = Mix_LoadWAV(path.string().c_str());
    if (!sound) {
        throw std::runtime_error("Could not load sound file: " + path.string()+" error: " + std::string(Mix_GetError()));
    }
}

SoundWrap &SoundWrap::operator=(SoundWrap &&other) noexcept {
    sound=other.sound;
    other.sound = nullptr;
    return *this;
}

SoundWrap::SoundWrap(SoundWrap &&other) noexcept {
    sound=other.sound;
    other.sound = nullptr;
}

SoundWrap::~SoundWrap() {
    if (sound!=nullptr) {
        Mix_FreeChunk(sound);
    }
}

void SoundWrap::play(int xScreen, int yScreen, int screenWidth, int screenHeight, double scale,bool silenceAtScaleout) const {


    if (xScreen<0 || xScreen>screenWidth|| yScreen<0 || yScreen>screenHeight || (silenceAtScaleout && scale<SOUND_SCALE_LIMIT)) {
        return;
    }

    if (sound!=nullptr) {
        Mix_PlayChannel(-1, sound, 0);
    }
}

