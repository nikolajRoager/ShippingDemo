//
// Created by nikolaj on 3/14/26.
//

#include "dialogue.h"

#include <iostream>
#include <utility>

#include "tableControl.h"
#include "textureControl.h"

dialogue::dialogue(int x, int y, const std::string& name, int dialogueMinWidth, int dialogueMaxHeight, std::shared_ptr<control> innerContent,SDL_Renderer* renderer, TTF_Font* font, bool closable) {
    x0_ = x;
    y0_ = y;

    auto headerName = std::make_shared<textureControl>(std::make_shared<TexWrap>(name,renderer,font));
    if (closable) {
        closeButton_ = std::make_shared<buttonControl>(std::make_shared<textureControl>(std::make_shared<TexWrap>("X",renderer,font)));
        header_ = std::make_shared<tableControl>(dialogueMinWidth,headerName->getHeight(),std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,dialogueMinWidth)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,dialogueMinWidth),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,dialogueMinWidth)},std::vector<std::shared_ptr<control> >{headerName ,closeButton_});
    }
    else {
        closeButton_ = nullptr;
        header_ = headerName;
    }
    content_ = std::make_shared<tableControl>(dialogueMinWidth,dialogueMaxHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,100),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,dialogueMaxHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,dialogueMinWidth)},std::vector<std::shared_ptr<control>>{header_,std::move(innerContent)},std::vector<tableControl::background>{tableControl::background(150,150,150),tableControl::background(100,100,100)});//, ;
    content_->setX0(x0_);
    content_->setY0(y0_);
    content_->setDepth(0);
}

void dialogue::update(const InputData &userInputs, int screenWidth, int screenHeight, bool covered) {
    SDL_Rect clip {0,0,screenWidth,screenHeight};
    content_->update(userInputs, screenWidth, screenHeight, covered, clip);

    //If we are hovering over the header, we can start moving the dialogue
    int hoverHeader = userInputs.mouseXPx>x0_ && userInputs.mouseXPx<x0_+content_->getWidth() && userInputs.mouseYPx>y0_ && userInputs.mouseYPx<y0_+header_->getHeight();
    if (!covered && hoverHeader && userInputs.leftMouseDown && !isMovingDialogue_) {
        isMovingDialogue_ = true;
        //We want to keep the relative mouse position fixed when the mouse moves
        relativeMouseX_ = userInputs.mouseXPx-x0_;
        relativeMouseY_ = userInputs.mouseYPx-y0_;
    }


    if (isMovingDialogue_) {
        //If something else come up on top of this, or we stopped holding down left, stop moving this thing
        if (!userInputs.leftMouseDown || covered) {
            isMovingDialogue_ = false;
            relativeMouseX_ = 0;
            relativeMouseY_ = 0;
        }
        else {
            //We want to keep the relative mouse position fixed when the mouse moves
            x0_= userInputs.mouseXPx-relativeMouseX_;
            y0_= userInputs.mouseYPx-relativeMouseY_;
            content_->setX0(x0_);
            content_->setY0(y0_);
        }
    }

    if (closeButton_!=nullptr) {
        if (closeButton_->isClicked())
            shouldClose_ = true;
    }

}

void dialogue::render(SDL_Renderer *renderer, int screenWidth, int screenHeight) const {
    SDL_Rect clipRect {0,0,screenWidth,screenHeight};

    for (int i = 0; i <= content_->getMaxDepth(); ++i)
        content_->render(renderer, screenWidth, screenHeight,i,clipRect,255,255,255,255);
}