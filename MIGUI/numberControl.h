//
// Created by nikolaj on 6/15/26.
//

#ifndef DUNGEONSKETCHTWO_NUMBERCONTROL_H
#define DUNGEONSKETCHTWO_NUMBERCONTROL_H
#include <memory>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>

#include "control.h"


class numberControl : public control {
public:
    explicit numberControl(const NumberRenderer& numberRenderer, int value);
    ~numberControl() override = default;

    void render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;

    [[nodiscard]] int getValue() const {return value_;}
    void setValue(int value) {
        value_ = value;
        width_= numberRenderer_.getWidth(value_);
        widthChanged_ = true;
    }

private:
    const NumberRenderer& numberRenderer_;
    int value_;
};


#endif //DUNGEONSKETCHTWO_NUMBERCONTROL_H