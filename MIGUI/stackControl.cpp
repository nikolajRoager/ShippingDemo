//
// Created by nikolaj on 2/28/26.
//

#include "stackControl.h"

#include <iostream>

int stackControl::getMaxDepth() const {
    int max = depth_;
    for (const auto& child : children_) {
        max = std::max(max, child->getMaxDepth());
    }
    return max;
}


void stackControl::setDepth(int depth) {
    depth_ = depth;
    for (const auto& child : children_) {
        child->setDepth(depth+1);
    }
}


stackControl::stackControl(orientation orientation, const std::vector<std::shared_ptr<control> > &children, int spacing) : control() {
    orientation_ = orientation;
    children_ = children;
    spacing_ = spacing;

    updateChildrenLocation();
}

void stackControl::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    for (const auto& child : children_) {
        child->render(renderer, screenWidth, screenHeight, depth, clip,r,g,b,a);
    }
}

void stackControl::update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) {
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

void stackControl::updateChildrenLocation() {
    switch (orientation_) {
        case HORIZONTAL:
            width_ =0;
            height_= 0;
            for (auto &child : children_) {
                child->setY0(y0_);
                child->setX0(width_+x0_);
                width_+= child->getWidth()+spacing_;
                height_= std::max(child->getHeight(), height_);
            }
        break;
        case VERTICAL:
            width_ =0;
            height_= 0;
            for (auto &child : children_) {
                child->setY0(height_+y0_);
                child->setX0(x0_);
                height_ += child->getHeight()+spacing_;
                width_= std::max(child->getWidth(), width_);
            }
            break;
    }
    widthChanged_=true;
    heightChanged_=true;
}


void stackControl::setX0(int x0) {
    x0_ = x0;

    updateChildrenLocation();
}

void stackControl::setY0(int y0) {
    y0_ = y0;

    updateChildrenLocation();
}

