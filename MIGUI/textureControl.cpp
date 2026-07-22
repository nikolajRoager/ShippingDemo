//
// Created by nikolaj on 2/27/26.
//

#include "textureControl.h"

#include <utility>

textureControl::textureControl(std::shared_ptr<const TexWrap> tex, bool background) : control(), texture_(std::move(tex)) {
    background_ = background;
    width_=texture_->getWidth();
    height_=texture_->getHeight();
};

void textureControl::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (depth==depth_) {
        if (background_) {
            SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
            SDL_Rect bgr {x0_,y0_,width_,height_};
            SDL_RenderFillRect(renderer, &bgr);
        }

        texture_->render(x0_,y0_,r,g,b,a,renderer,clip);
    }
}

