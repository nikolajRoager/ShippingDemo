//
// Created by nikolaj on 2/28/26.
//

#include "guiManager.h"

#include <ranges>

void GUIManager::update(const InputData& userInputs, int screenWidth, int screenHeight) {
    SDL_Rect clipRect {0,0, screenWidth, screenHeight};

    //First update the right-click control, it is on top of everything else
    if (rightClickControl_!=nullptr) {
        rightClickControl_->update(userInputs, screenWidth, screenHeight, false, clipRect);
        hoverRightClickControl_ = rightClickControl_->getHoverInherited();
        if ((userInputs.leftMouseDown) && !hoverRightClickControl_) {
            rightClickControl_=nullptr;
            hoverRightClickControl_ = false;
        }
    }
    else
        hoverRightClickControl_ = false;

    //Loop through dialogues from front so front obscure the rest
    bool hoverDialogue=false;
    if (!hoverRightClickControl_)
        for (auto itr = dialogues_.begin(); itr != dialogues_.end(); ++itr) {
            (*itr)->update(userInputs,screenWidth,screenHeight,hoverDialogue || hoverRightClickControl_ );
            //If we hover over a dialogue, and presses a button, we bring it to the front
            if (!hoverDialogue && (*itr)->getHoverInherited()) {
                hoverDialogue = true;
                if ((userInputs.leftMouseDown && !userInputs.prevLeftMouseDown) || (userInputs.rightMouseDown && !userInputs.prevRightMouseDown))
                    dialogues_.splice(dialogues_.begin(),dialogues_,itr);
            }
        }

    dialogues_.remove_if([](const auto& dialogue) {return dialogue->getShouldClose();});


    baseControl_->update(userInputs,screenWidth,screenHeight, hoverRightClickControl_ || hoverDialogue,clipRect);
    baseControl_->setAvailableWidth(screenWidth);
    baseControl_->setAvailableHeight(screenHeight);
    baseControl_->setDepth(0);
    maxDepth_ = baseControl_->getMaxDepth();

    hoverRightClickControlOrDialogue_ = hoverDialogue || hoverRightClickControl_;
}

void GUIManager::setRightClickControl(std::shared_ptr<control> control, int x, int y) {
    rightClickControl_ = std::move(control);
    rightClickDepth_ = rightClickControl_->getDepth();
    rightClickControl_->setX0(x);
    rightClickControl_->setY0(y);
}


void GUIManager::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, Uint8 R, Uint8 G, Uint8 B, Uint8 A) const {
    SDL_Rect clipRect {0,0, screenWidth, screenHeight};
    //Render essentially breadth first, rather than depth first, this means further down the tree are on top
    for (int i = 0; i <= maxDepth_ ; ++i)
        baseControl_->render(renderer, screenWidth, screenHeight,i,clipRect,R,G,B,A);

    //Display dialogues in reverse order, so the front dialogue obscures the rest.
    for (const auto & dialogue : std::ranges::reverse_view(dialogues_)) {
        dialogue->render(renderer,screenWidth,screenHeight);
    }

    if (rightClickControl_!=nullptr)
        for (int i = 0; i <= rightClickDepth_; ++i)
            rightClickControl_->render(renderer, screenWidth, screenHeight,i,clipRect,255,255,255,255);
}

