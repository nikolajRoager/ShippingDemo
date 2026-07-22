//
// Created by nikolaj on 2/25/26.
//

#include "control.h"

control::control() {
    x0_ = 0;
    y0_ = 0;
    width_=height_=0.0;
    depth_=0;
}


void control::update(const InputData& userInputs, int screenWidth, int screenHeight, bool /*covered*/, SDL_Rect& clip){
    hoverInherited_ =
        (userInputs.mouseXPx>x0_ && userInputs.mouseYPx>y0_ && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<height_+y0_);
}

