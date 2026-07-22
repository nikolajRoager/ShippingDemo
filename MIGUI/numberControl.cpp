//
// Created by nikolaj on 6/15/26.
//

#include "numberControl.h"

numberControl::numberControl(const NumberRenderer &numberRenderer, int value): numberRenderer_(numberRenderer) {
    value_ = value;
    width_= numberRenderer_.getWidth(value_);
    height_=numberRenderer_.getHeight();
}

void numberControl::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect &clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (depth==depth_)
        numberRenderer_.render(value_,x0_,y0_,r,g,b,a,renderer,clip);
}

