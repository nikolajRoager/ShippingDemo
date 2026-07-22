//
// Created by nikolaj on 7/16/26.
//

#include "multilineStackControl.h"

MultilineStackControl::MultilineStackControl(const std::vector<std::shared_ptr<control> > &children, int spacing) {
    children_ = children;
    spacing_ = spacing;

    updateChildrenLocation();
}


int MultilineStackControl::getMaxDepth() const {
    int max = depth_;
    for (const auto& child : children_) {
        max = std::max(max, child->getMaxDepth());
    }
    return max;
}

void MultilineStackControl::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect &clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    for (const auto& child : children_) {
        child->render(renderer, screenWidth, screenHeight, depth, clip,r,g,b,a);
    }
}

void MultilineStackControl::setDepth(int depth) {
    depth_ = depth;
    for (const auto& child : children_) {
        child->setDepth(depth+1);
    }
}

void MultilineStackControl::setX0(int x0) {
    x0_ = x0;
    updateChildrenLocation();
}

void MultilineStackControl::setY0(int y0) {
    y0_ = y0;
    updateChildrenLocation();
}

void MultilineStackControl::update(const InputData &userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect &clip) {
    hoverInherited_ =
        (userInputs.mouseXPx>x0_ && userInputs.mouseYPx>y0_ && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<height_+y0_);
    //Update children
    bool sizeChanged = false;
    for (auto &child : children_) {
        hoverInherited_=hoverInherited_|| child->getHoverInherited();
        child->update(userInputs, screenWidth, screenHeight, covered, clip);
        sizeChanged = sizeChanged || child->getHeightChanged() || child->getWidthChanged();
    }
    if (sizeChanged)
        updateChildrenLocation();
}


void MultilineStackControl::updateChildrenLocation() {
    int y= 0;
    int x=0;
    int rowHeight=0;
    height_=0;
    //Loop through the children
    for (auto &child : children_) {
        //Check if we should move to the next row
        if (x+child->getWidth()>width_ && child->getWidth()<width_ /*Failsafe, if the element is too wide to fit on the line it just overflows, instead of entering into an infinite loop*/) {
            y+=rowHeight+spacing_;
            height_+=rowHeight+spacing_;
            x=0;
            rowHeight =0;
        }
        child->setY0(y+y0_);
        child->setX0(x+x0_);
        rowHeight=std::max(rowHeight, child->getHeight());
        x+=child->getWidth()+spacing_;
    }
    height_+=rowHeight;
    heightChanged_ = true;
}





void MultilineStackControl::setAvailableWidth(int newWidth) {
    width_ = newWidth;
    updateChildrenLocation();
}


