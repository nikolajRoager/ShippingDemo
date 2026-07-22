//
// Created by nikolaj on 5/15/26.
//

#ifndef LCCRENDERINGDEMO_EMPTYCONTROL_H
#define LCCRENDERINGDEMO_EMPTYCONTROL_H
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <iostream>

#include "control.h"

/// An empty control with no content
class emptyControl : public control{
public:
    emptyControl() = default;
    ~emptyControl() override = default;


    ///Update how much width is available, the control might or might not  care depending on the class
    void setAvailableWidth(int width) override {width_ = width;};
    ///Update how much width is available, the control might or might not care depending on the class
    void setAvailableHeight(int height) override {height_ = height;};

    void render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override {};
};


#endif //LCCRENDERINGDEMO_EMPTYCONTROL_H