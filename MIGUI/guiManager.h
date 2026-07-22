//
// Created by nikolaj on 2/28/26.
//

#ifndef MIGUIDEMO_GUIMANAGER_H
#define MIGUIDEMO_GUIMANAGER_H
#include <list>
#include <memory>

#include "control.h"
#include "dialogue.h"


///Manages the display of a single base control
class GUIManager {
public:
    explicit GUIManager(const std::shared_ptr<control> &baseControl) {
        baseControl_ = baseControl;
        rightClickControl_=nullptr;
    }

    void addDialogue(std::shared_ptr<dialogue>& D) {
        dialogues_.push_back(D);
    }

    void addDialogue(const std::string& title, const std::shared_ptr<control>& content,int screenHeight,SDL_Renderer* renderer, TTF_Font* smallFont) {
        dialogues_.emplace_back(std::make_shared<dialogue>(200,64,title,100,screenHeight,content,renderer,smallFont,true));
    }

    ///Render the controls and dialogues
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, Uint8 R=255, Uint8 G = 255, Uint8 B = 255, Uint8 A = 255) const;
    ///Update the controls and dialogues
    void update(const InputData& userInputs, int screenWidth, int screenHeight);

    void setRightClickControl(std::shared_ptr<control> control, int x, int y);

    [[nodiscard]] bool hasRightClickControl() const {return rightClickControl_!=nullptr;}

    [[nodiscard]] bool hoverSomething() const {return hoverRightClickControlOrDialogue_;}

    void closeAllDialogues() {
        for (auto &dialogue : dialogues_) {
            dialogue->close();
        }
    }
private:
    std::shared_ptr<control> baseControl_;

    ///A single control hovering over all the others, typically brought up by right-clicking, and closed by clicking elsewhere
    std::shared_ptr<control> rightClickControl_;

    ///Hovering dialogues containing controls
    std::list<std::shared_ptr<dialogue>> dialogues_;

    int maxDepth_ = 0;
    int rightClickDepth_ = 0;
    bool hoverRightClickControl_ = false;
    bool hoverRightClickControlOrDialogue_ = false;
};


#endif //MIGUIDEMO_GUIMANAGER_H