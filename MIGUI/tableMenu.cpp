//
// Created by nikolaj on 3/14/26.
//

#include "tableMenu.h"

#include "tableControl.h"
#include "textureControl.h"

int tableMenu::getMaxDepth() const {
    return std::max(depth_,table_->getMaxDepth());
}

void tableMenu::setDepth(int depth) {
    depth_ = depth;
    //This updates our shared children too
    table_->setDepth(depth);
}

void tableMenu::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect &clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    //This displays our shared children too
    table_->render(renderer, screenWidth, screenHeight, depth, clip,r,g,b,a);
}

void tableMenu::update(const InputData &userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect &clip) {
    //This updates our shared children too
    table_->update(userInputs, screenWidth, screenHeight, covered, clip);
    //This gets hover inherited from all our children too
    hoverInherited_ = table_->getHoverInherited();
}

void tableMenu::setX0(int x0) {
    //This sets x of our shared children too
    table_->setX0(x0);
}

void tableMenu::setY0(int y0) {
    //This sets the y of our shared children too
    table_->setY0(y0);
}

tableMenu::tableMenu(int availableWidth, int availableHeight,const std::vector<std::string> &items,SDL_Renderer *renderer,TTF_Font *font) {
    std::vector<std::shared_ptr<control>> childrenAsControls;
    childrenAsControls.reserve(items.size());
    for (const auto& item : items) {
        auto child = std::make_shared<buttonControl>(std::make_shared<textureControl>(std::make_shared<TexWrap>(item,renderer,font)),50,50,50,true);
        children_.emplace(item,child);
        childrenAsControls.push_back(child);
    }
    table_ = std::make_shared<tableControl>(availableWidth,availableHeight,std::vector<tableControl::rowOrCol>(items.size(),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,100)),std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,100)},childrenAsControls,std::vector<tableControl::background>(items.size(),tableControl::background(50,50,50)));
}