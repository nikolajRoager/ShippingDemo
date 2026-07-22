//
// Created by nikolaj on 3/14/26.
//

#ifndef MIGUIDEMO_DIALOGUE_H
#define MIGUIDEMO_DIALOGUE_H
#include <memory>

#include "buttonControl.h"
#include "control.h"

///A dialogue is not a control per se, it is a container for controls which can be dragged around
class dialogue {
public:
    dialogue(int x, int y, const std::string& name, int dialogueMinWidth, int dialogueMaxHeight,std::shared_ptr<control> innerContent,SDL_Renderer* renderer, TTF_Font* font, bool closable=false);

    ///Render the dialogue
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight) const;
    ///Update the dialogue
    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered);

    [[nodiscard]] bool getHoverInherited() const {
        return content_->getHoverInherited() || isMovingDialogue_;
    }

    [[nodiscard]] bool getShouldClose() const {return shouldClose_;}

    ///Sets the flag that this should be closed, it is the responsibility of whoever manages the dialogues to then close it
    void close() {shouldClose_=true;}
private:
    std::shared_ptr<control> content_;
    std::shared_ptr<control> header_;
    std::shared_ptr<buttonControl> closeButton_;
    int x0_;
    int y0_;

    //For moving the dialogue with the mouse
    bool isMovingDialogue_ = false;
    int relativeMouseX_ = 0;
    int relativeMouseY_ = 0;

    bool shouldClose_ = false;
};


#endif //MIGUIDEMO_DIALOGUE_H