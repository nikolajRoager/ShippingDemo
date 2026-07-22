//
// Created by nikolaj on 12/30/25.
//

#include "NumberRenderer.h"

#include <iostream>
#include <vector>

NumberRenderer::NumberRenderer(int _spacing,TTF_Font* font, SDL_Renderer* renderer):
minus_("-",renderer,font),
digits_{
    TexWrap("0", renderer, font),
    TexWrap("1", renderer, font),
    TexWrap("2", renderer, font),
    TexWrap("3", renderer, font),
    TexWrap("4", renderer, font),
    TexWrap("5", renderer, font),
    TexWrap("6", renderer, font),
    TexWrap("7", renderer, font),
    TexWrap("8", renderer, font),
    TexWrap("9", renderer, font)
}
{
    spacing_ = _spacing;
}

int NumberRenderer::getHeight() const {
    return digits_[0].getHeight();
}

int NumberRenderer::getWidth(int number) const {

    int width = 0;
    if (number<0)
        width+=minus_.getWidth();
    std::vector<int> numberDigits;
    if (number==0)
        width+=digits_[0].getWidth();
    number=std::abs(number);
    for (;number>0;number/=10) {
        width +=digits_[number%10].getWidth()+spacing_;
    }

    return width;
}

int NumberRenderer::render(int number, double x, double y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Renderer *renderer, SDL_Rect& clip, double scale, bool centerX, bool centerY, bool flip, unsigned int frames, unsigned int frame) const {
    int width = 0;
    if (number<0) {
        minus_.render(x,y,r,g,b,a,renderer, clip, scale, centerX, centerY, flip, frames, frame);
        width +=minus_.getWidth()+spacing_;
        x+=minus_.getWidth()+spacing_;
    }

    number = std::abs(number);

    std::vector<int> numberDigits;
    if (number==0) {
        numberDigits.push_back(0);
    }
    else
        for (;number>0;number/=10) {
            numberDigits.push_back(number % 10);
        }

    for (int i = numberDigits.size()-1; i >= 0; i--) {
        digits_[numberDigits[i]].render(x,y,r,g,b,a,renderer,clip, scale, centerX, centerY, flip, frames, frame);
        width +=digits_[numberDigits[i]].getWidth()+spacing_;
        x+=digits_[numberDigits[i]].getWidth()+spacing_;
    }
    return width;
}

int NumberRenderer::render(int number, double x, double y, Uint8 r, Uint8 g, Uint8 b, SDL_Renderer *renderer, SDL_Rect& clip, double scale, bool centerX, bool centerY, bool flip, unsigned int frames, unsigned int frame) const {
    return render(number,x,y,r,g,b,255,renderer,clip,scale,centerX,centerY,flip,frames,frame);
}
int NumberRenderer::render(int number, double x, double y, SDL_Renderer *renderer, SDL_Rect& clip, double scale, bool centerX, bool centerY, bool flip, unsigned int frames, unsigned int frame) const {
    return render(number,x,y,255,255,255,255,renderer,clip,scale,centerX,centerY,flip,frames,frame);
}



