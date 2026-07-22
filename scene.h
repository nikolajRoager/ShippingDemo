//
// Created by nikolaj on 6/1/26.
//

#ifndef DUNGEONSKETCH_SCENE_H
#define DUNGEONSKETCH_SCENE_H
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>

#include "inputData.h"

struct SceneOutput {
    std::string stringData_;
    int intData_=0;
    bool boolData_=false;

    explicit SceneOutput(std::string str,int i, bool b): stringData_(std::move(str)),intData_(i),boolData_(b) {}
    explicit SceneOutput(std::string str): stringData_(std::move(str)) {}
    explicit SceneOutput(int i): intData_(i) {}
};

class Scene {
public:
    enum SceneInfo {
        QUIT_GAME,
        START_GAME,
        RELOAD_FONTS,
        START_TYPING,
        STOP_TYPING,
        QUIT_TO_MENU,
        SET_MUSIC,
        SET_MUSIC_VOLUME,
        SET_VOLUME
    };

    Scene()=default;
    virtual ~Scene()=default;

    virtual void render(SDL_Renderer* renderer, int screenWidth, int screenHeight,const InputData& userInputs, unsigned int millis, unsigned int pmillis) const=0;
    ///Returns what scene we should transition to, and the arguments to that
    virtual std::optional<std::pair<SceneInfo,SceneOutput>> update(SDL_Renderer* renderer, int screenWidth, int screenHeight,const InputData& userInputs,  unsigned int millis, unsigned int dmillis, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont)=0;
};


#endif //DUNGEONSKETCH_SCENE_H