//
// Created by nikolaj on 6/17/26.
//

#include "numberInputControl.h"

#include <utility>

NumberInputControl::NumberInputControl(const NumberRenderer &numberRenderer, int value, int min, int max, std::shared_ptr<const TexWrap> plusTex, std::shared_ptr<const TexWrap> minusTex, Uint8 R, Uint8 G, Uint8 B) : numberRenderer_(numberRenderer), value_(value), min_(min), max_(max), plusTex_(std::move(plusTex)), minusTex_(std::move(minusTex)) {
    R_ = R; G_ = G; B_ = B;
    numWidth_=numberRenderer_.getWidth(value_);
    width_= numWidth_+std::max(plusTex_->getWidth(),minusTex_->getWidth());
    height_=std::max(numberRenderer_.getHeight(),plusTex_->getHeight()+minusTex_->getHeight());
}



void NumberInputControl::setValue(int value) {
    value_ = std::ranges::clamp(value,min_,max_);
    numWidth_=numberRenderer_.getWidth(value_);
    width_= numWidth_+std::max(plusTex_->getWidth(),minusTex_->getWidth());
    widthChanged_ = true;
}

void NumberInputControl::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect &clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (depth==depth_) {
        numberRenderer_.render(value_,x0_,y0_,r,g,b,a,renderer,clip);
        int x = x0_+numWidth_;
        if (allowIncrement_ && value_<max_)
            plusTex_->render(x,y0_,hoverPlus_? R_ : r,hoverPlus_? G_ : g,hoverPlus_? B_ :b,a,renderer,clip);
        if (value_>min_)
            minusTex_->render(x,y0_+minusTex_->getHeight() ,hoverMinus_?R_: r,hoverMinus_? G_: g,hoverMinus_? B_: b,a,renderer,clip);
    }
}

void NumberInputControl::update(const InputData &userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect &clip) {
    clicked_=false;
    hoverPlus_ =allowIncrement_ && (userInputs.mouseXPx>x0_+numWidth_ && userInputs.mouseYPx>y0_ && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<plusTex_->getHeight()+y0_);
    hoverMinus_ =(userInputs.mouseXPx>x0_+numWidth_ && userInputs.mouseYPx>y0_+plusTex_->getHeight() && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<height_+y0_);
    if (hoverPlus_ && ((userInputs.leftMouseDown && !userInputs.prevLeftMouseDown) || (userInputs.rightPressed && !userInputs.prevRightMouseDown))) {
        setValue(value_+1);
        clicked_=true;
    }
    if (hoverMinus_&& ((userInputs.leftMouseDown && !userInputs.prevLeftMouseDown) || (userInputs.rightPressed && !userInputs.prevRightMouseDown))) {
        setValue(value_-1);
        clicked_=true;
    }

}
