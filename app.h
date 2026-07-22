//
// Created by nikolaj on 6/1/26.
//

#ifndef DUNGEONSKETCH_APP_H
#define DUNGEONSKETCH_APP_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "musicWrap.h"
#include "scene.h"

class App {
public:
    App();
    ~App();
    void run();

private:

    std::vector<musicWrap> music_;
    int selectedMusicTrack_;

    //The window we'll be rendering to
    SDL_Window* window_;

    //The window renderer
    SDL_Renderer* renderer_;

    int smallFontSize_;
    int midFontSize_;
    int largeFontSize_;

    TTF_Font* smallFont_;
    TTF_Font* midFont_;
    TTF_Font* largeFont_;

    Scene* currentScene_;

    int windowWidthPx_;
    int windowHeightPx_;
};


#endif //DUNGEONSKETCH_APP_H