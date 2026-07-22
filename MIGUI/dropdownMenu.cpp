//
// Created by nikolaj on 6/18/26.
//

#include "dropdownMenu.h"

#include <iostream>
#include <utility>

DropdownMenu::DropdownMenu(const std::vector<std::shared_ptr<control> >& controls, std::shared_ptr<const TexWrap> expandButtonTexture, Uint8 RHover, Uint8 GHover, Uint8 BHover, Uint8 RSelect, Uint8 GSelect, Uint8 BSelect, Uint8 RBgr, Uint8 GBgr, Uint8 BBgr):controls_(controls),expandButtonTexture_(std::move(expandButtonTexture)) {
    RHover_ = RHover;
    GHover_ = GHover;
    BHover_ = BHover;
    RSelect_ = RSelect;
    GSelect_ = GSelect;
    BSelect_ = BSelect;
    RBgr_ = RBgr;
    GBgr_ = GBgr;
    BBgr_ = BBgr;
    if (controls_.empty())
        throw std::runtime_error("No controls for dropdown menu.");
    selectedIndex_=0;
    width_=controls[selectedIndex_]->getWidth()+expandButtonTexture_->getWidth();
    height_=std::max(controls[selectedIndex_]->getHeight(),expandButtonTexture_->getHeight());
    expandButtonTextureX_ =x0_+(controls[selectedIndex_]->getWidth());

    expanseWidth_=width_;
    for (const auto &control : controls_)
        expanseWidth_=std::max(control->getWidth(),expanseWidth_);
}

int DropdownMenu::getMaxDepth() const {
    int maxDepth=depth_;
    for (const auto& control : controls_)
        maxDepth = std::max(control->getMaxDepth(),maxDepth);
    return maxDepth;
}

void DropdownMenu::setDepth(int depth) {
    depth_ = depth;
    for (auto& control : controls_)
        control->setDepth(depth+10);
    controls_[selectedIndex_]->setDepth(depth+1);
}

void DropdownMenu::setX0(int x0) {
    x0_ = x0;
    expandButtonTextureX_ =x0_+(controls_[selectedIndex_]->getWidth());
    for (auto& control : controls_)
        control->setX0(x0);
}

void DropdownMenu::setY0(int y0) {
    y0_ = y0;
    int y = y0_;
    controls_[selectedIndex_]->setY0(y);

    //Just a guess, we don't have access to the screenheight so we can't tell here
    expandAbove_=false;
    y+=controls_[selectedIndex_]->getHeight();
    for (size_t i = 0; i < controls_.size(); ++i) {
        if (i!=selectedIndex_) {
            controls_[i]->setY0(y);
            y+=controls_[i]->getHeight();
        }
    }

}

void DropdownMenu::update(const InputData &userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect &clip) {
    if (y0_+expanseHeight_>screenHeight) {
        if (!expandAbove_) {
            expandAbove_=true;
        }
    }
    else {//Can expand below, do it
        if (expandAbove_) {
            expandAbove_=false;
        }

    }

    changedSelection_=false;
    controls_[selectedIndex_]->update(userInputs, screenWidth, screenHeight, covered, clip);
    hoverButton_ =(userInputs.mouseXPx>expandButtonTextureX_ && userInputs.mouseYPx>y0_ && userInputs.mouseXPx<expandButtonTextureX_+expandButtonTexture_->getWidth() && userInputs.mouseYPx<height_+y0_);
    if (!expanded_) {
        if (hoverButton_ && ((userInputs.leftMouseDown && !userInputs.prevLeftMouseDown) || (userInputs.rightPressed && !userInputs.prevRightMouseDown))) {
            expanded_ = true;
            recalculateExpansion();
        }
    }

    if (expanded_) {

        int y =expandAbove_?y0_ :  y0_+height_;
        hoveredIndex_=-1;
        SDL_Rect newClip = {x0_,y0_+height_,width_,expanseHeight_};

        bool hoverMenu=(userInputs.mouseXPx>x0_&& userInputs.mouseYPx> (expandAbove_ ?(y0_-expanseHeight_) : y0_) && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<y0_+height_+expanseHeight_);


        for (size_t i = 0; i < controls_.size(); ++i) {
            if (i!=selectedIndex_) {
                if (expandAbove_)
                    y-=controls_[i]->getHeight();
                controls_[i]->setX0(x0_);
                controls_[i]->setY0(y);
                if (userInputs.mouseXPx>x0_ && userInputs.mouseYPx>y && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<y+controls_[i]->getHeight()) {
                    hoveredIndex_=i;
                }
                if (!expandAbove_)
                    y+=controls_[i]->getHeight();
                controls_[i]->update(userInputs, screenWidth, screenHeight, covered, newClip );
            }
        }

        if (hoveredIndex_<controls_.size() &&  ((userInputs.leftMouseDown && !userInputs.prevLeftMouseDown) || (userInputs.rightMouseDown && !userInputs.prevRightMouseDown))) {
            selectedIndex_=hoveredIndex_;
            changedSelection_=true;
            recalculateExpansion();
            setDepth(depth_);
        }
        if (hoverMenu) {
            hoverInherited_=true;
        }
        else {
            expanded_=false;
        }
    }

}

void DropdownMenu::recalculateExpansion() {
    expanseHeight_=0;

    controls_[selectedIndex_]->setY0(y0_);
    for (size_t i = 0; i < controls_.size(); ++i) {
        if (i!=selectedIndex_) {
            expanseHeight_+=controls_[i]->getHeight();
        }
    }

    width_=controls_[selectedIndex_]->getWidth()+expandButtonTexture_->getWidth();
    height_=std::max(controls_[selectedIndex_]->getHeight(),expandButtonTexture_->getHeight());
    expandButtonTextureX_ =x0_+(controls_[selectedIndex_]->getWidth());

    expanseWidth_=width_;
    for (const auto &control : controls_)
        expanseWidth_=std::max(control->getWidth(),expanseWidth_);
    width_=expanseWidth_;

}


void DropdownMenu::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect &clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (depth==depth_)
        expandButtonTexture_->render(expandButtonTextureX_,y0_,hoverButton_?RHover_:r,hoverButton_?GHover_:g,hoverButton_?BHover_:b,a,renderer,clip);
    if (expanded_) {
        if (depth==depth_+10) {

            SDL_Rect bgr{x0_,expandAbove_? y0_-expanseHeight_ : y0_+height_,width_,expanseHeight_};
            SDL_SetRenderDrawColor(renderer,RBgr_,GBgr_,GBgr_,255);
            SDL_RenderFillRect(renderer,&bgr);
        }
        for (size_t i = 0; i < controls_.size(); ++i) {
            SDL_Rect newClip {x0_,expandAbove_? y0_-expanseHeight_ : y0_+height_,width_,expanseHeight_};
            if (i!=selectedIndex_) {
                if (i==hoveredIndex_) {
                    if (depth==depth_+10) {
                        SDL_Rect bgr{x0_,controls_[i]->getY0(),width_,controls_[i]->getHeight()};
                        SDL_SetRenderDrawColor(renderer,RSelect_,GSelect_,BSelect_,255);
                        SDL_RenderFillRect(renderer,&bgr);
                    }
                }
                controls_[i]->render(renderer, screenWidth, screenHeight, depth, newClip, r, g, b, a);
            }
        }
    }
    controls_[selectedIndex_]->render(renderer, screenWidth, screenHeight, depth, clip, r, g, b, a);
}
