//
// Created by nikolaj on 3/7/26.
//

#include "mouseOverControl.h"

#include <iostream>

void mouseOverControl::update(const InputData &userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) {
    hover_ =(userInputs.mouseXPx>x0_ && userInputs.mouseYPx>y0_ && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<height_+y0_&&
    userInputs.mouseXPx>clip.x && userInputs.mouseYPx>clip.y && userInputs.mouseXPx<clip.w+clip.x && userInputs.mouseYPx<clip.h+clip.y
    && !covered
    );


    hoverInherited_ = hover_ || content_->getHoverInherited();


    if (hover_) {
        int minX = userInputs.mouseXPx-mouseOverContent_->getWidth()/2;
        int maxX = userInputs.mouseXPx+mouseOverContent_->getWidth()/2;
        int minY = userInputs.mouseYPx;
        int maxY = userInputs.mouseYPx+mouseOverContent_->getHeight();

        if (minX<0) {
            minX=0;
        }
        else if (maxX>screenWidth) {
            minX=screenWidth-mouseOverContent_->getWidth();
        }
        if (maxY>screenHeight) {
            minY=screenHeight-mouseOverContent_->getHeight();
        }
        mouseOverContent_->setX0(minX);
        mouseOverContent_->setY0(minY);
    }

    if (content_->getWidthChanged()) {
        width_ = content_->getWidth();
        widthChanged_ = true;
    }
    if (content_->getHeightChanged()) {
        height_ = content_->getHeight();
        heightChanged_ = true;
    }
}

void mouseOverControl::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    content_->render(renderer, screenWidth, screenHeight, depth,clip, r, g, b, a);
    if (hover_) {
        if (depth==mouseOverContent_->getDepth()) {
            SDL_SetRenderDrawColor(renderer, R_, G_, B_, 255);
            SDL_Rect rect = {mouseOverContent_->getX0(), mouseOverContent_->getY0(), mouseOverContent_->getWidth(), mouseOverContent_->getHeight()};
            SDL_RenderFillRect(renderer, &rect);
        }
        SDL_Rect totalRect = {0,0, screenWidth, screenHeight};
        mouseOverContent_->render(renderer, screenWidth, screenHeight, depth,totalRect,255,255,255,255);
    }
}

mouseOverControl::mouseOverControl(const std::shared_ptr<control> &content, const std::shared_ptr<control> &mouseOverContent, Uint8 R, Uint8 G, Uint8 B) {
    content_=content;
    width_=content_->getWidth();
    height_=content_->getHeight();
    mouseOverContent_=mouseOverContent;
    R_ = R;
    G_ = G;
    B_ = B;
}


