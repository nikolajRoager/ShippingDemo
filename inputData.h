//
// Created by nikolaj on 6/1/26.
//

#ifndef DUNGEONSKETCH_INPUTDATA_H
#define DUNGEONSKETCH_INPUTDATA_H
#include <string>

struct InputData {
    int scroll=0;
    int mouseXPx=0, mouseYPx=0;

    bool leftPressed=false;
    bool rightPressed=false;
    bool upPressed=false;
    bool downPressed=false;
    bool zoomInPressed=false;
    bool zoomOutPressed=false;

    bool aPressed=false;
    bool rPressed=false;
    bool dPressed=false;
    bool wPressed=false;
    bool sPressed=false;
    bool qPressed=false;
    bool ePressed=false;

    bool onePressed=false;
    bool twoPressed=false;
    bool threePressed=false;
    bool fourPressed=false;
    bool fivePressed=false;
    bool sixPressed=false;

    bool commaPressed=false;
    bool periodPressed=false;

    bool prevCommaPressed=false;
    bool prevPeriodPressed=false;

    bool homePressed=false;
    bool prevHomePressed=false;

    bool nPressed=false;
    bool prevNPressed=false;

    bool mPressed=false;
    bool prevMPressed=false;

    bool vPressed=false;
    bool prevVPressed=false;

    bool rightMouseDown=false;
    bool prevRightMouseDown=false;
    bool leftMouseDown=false;
    bool prevLeftMouseDown=false;

    bool sizeChanged=false;

    bool prevAPressed=false;
    bool prevRPressed=false;
    bool prevDPressed=false;
    bool prevWPressed=false;
    bool prevSPressed=false;
    bool prevQPressed=false;
    bool prevEPressed=false;


    bool prevOnePressed=false;
    bool prevTwoPressed=false;
    bool prevThreePressed=false;
    bool prevFourPressed=false;
    bool prevFivePressed=false;
    bool prevSixPressed=false;

    bool enterPressed=false;
    bool prevEnterPressed=false;
    bool escapePressed=false;
    bool prevEscapePressed=false;
    bool spacePressed=false;
    bool prevSpacePressed=false;
    bool escPressed=false;

    bool shiftPressed=false;
    bool ctrlPressed=false;

    bool typingIsActive = false;
    bool typingTextUpdated =false;
    std::string typingText;
};

#endif //DUNGEONSKETCH_INPUTDATA_H