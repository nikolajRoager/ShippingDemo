//
// Created by nikolaj on 6/14/26.
//

#include "slideControl.h"

SlideControl::SlideControl(const std::vector<std::shared_ptr<control> > &children) {
    children_ = children;
    if (children_.empty())
        throw std::invalid_argument("There is no children defined for slide control");
    activeChild_=0;
}
void SlideControl::setX0(int x0) {
    x0_ = x0;

    for (auto &child : children_) {
        child->setX0(x0);
    }
}

void SlideControl::setY0(int y0) {
    y0_ = y0;

    for (auto &child : children_) {
        child->setY0(y0);
    }
}
int SlideControl::getMaxDepth() const {
    int max = depth_;
    for (const auto& child : children_) {
        max = std::max(max, child->getMaxDepth());
    }
    return max;
}


void SlideControl::setDepth(int depth) {
    depth_ = depth;
    for (const auto& child : children_) {
        child->setDepth(depth+1);
    }
}

void SlideControl::update(const InputData &userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect &clip) {

    hoverInherited_ =
        (userInputs.mouseXPx>x0_ && userInputs.mouseYPx>y0_ && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<height_+y0_);

    hoverInherited_=hoverInherited_|| children_[activeChild_]->getHoverInherited();

    widthChanged_=children_[activeChild_]->getWidthChanged();
    if (widthChanged_)
        width_=children_[activeChild_]->getWidth();
    heightChanged_=children_[activeChild_]->getHeightChanged();
    if (heightChanged_)
        height_=children_[activeChild_]->getHeight();

    children_[activeChild_]->update(userInputs, screenWidth, screenHeight, covered, clip);

}

void SlideControl::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect &clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    children_[activeChild_]->render(renderer, screenWidth, screenHeight, depth, clip, r, g, b , a);
}

