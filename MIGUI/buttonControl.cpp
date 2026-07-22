//
// Created by nikolaj on 2/28/26.
//

#include "buttonControl.h"

#include <memory>

buttonControl::buttonControl(const std::shared_ptr<control> &content, Uint8 R, Uint8 G, Uint8 B, bool expandWidth): control() {
    content_=content;
    R_ = R;
    G_ = G;
    B_ = B;
    width_=content_->getWidth();
    height_=content_->getHeight();
    expandWidth_=expandWidth;
}


void buttonControl::update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) {
    int w = content_->getWidth();
    int h = content_->getHeight();
    if (!expandWidth_)
    {
        if (w!=width_) {
            width_=w;
            widthChanged_ = true;
        }
    }
    if (h!=height_) {
        height_=h;
        heightChanged_ = true;
    }

    hover_ =(userInputs.mouseXPx>x0_ && userInputs.mouseYPx>y0_ && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<height_+y0_);
    hoverInherited_ = hover_ || content_->getHoverInherited();
    clicked_ = !covered && hover_ && ((userInputs.leftMouseDown && !userInputs.prevLeftMouseDown) || (userInputs.rightPressed && !userInputs.prevRightMouseDown));

    content_->update(userInputs,screenWidth,screenHeight, covered, clip);
}

void buttonControl::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (depth==depth_) {
        SDL_Rect backgroundRect= {x0_,y0_,width_,height_};


        // --- Clipping logic ---
        // Find how many pixels are clipped off each edge of the destination rect,
        // then adjust the source rect proportionally so we only sample the
        // texels that correspond to the visible portion of the destination.

        // Pixels clipped from each side
        int clipLeft   = std::max(0, clip.x - backgroundRect.x);
        int clipTop    = std::max(0, clip.y - backgroundRect.y);
        int clipRight  = std::max(0, (backgroundRect.x + backgroundRect.w) - (clip.x + clip.w));
        int clipBottom = std::max(0, (backgroundRect.y + backgroundRect.h) - (clip.y + clip.h));

        // If the quad is entirely outside the clip rect, nothing to draw
        if ( !(clipLeft + clipRight >= backgroundRect.w || clipTop + clipBottom >= backgroundRect.h)) {

            // Adjust destination rect
            backgroundRect.x += clipLeft;
            backgroundRect.y += clipTop;
            backgroundRect.w -= clipLeft + clipRight;
            backgroundRect.h -= clipTop  + clipBottom;

            if (hover_)
                SDL_SetRenderDrawColor(renderer,100,100,100,255);
            else
                SDL_SetRenderDrawColor(renderer,150,150,150,255);
            SDL_RenderFillRect(renderer,&backgroundRect);
            SDL_SetRenderDrawColor(renderer,255,255,255,255);
            SDL_RenderDrawRect(renderer,&backgroundRect);
        }

    }
    content_->render(renderer, screenWidth, screenHeight, depth,clip,hover_?R_:r,hover_?G_: g,hover_?B_: b,a);
}
