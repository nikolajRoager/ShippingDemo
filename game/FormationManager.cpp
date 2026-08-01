//
// Created by nikolaj on 7/31/26.
//

#include "FormationManager.h"


FormationManager::FormationManager(double desiredDistance,std::vector<std::shared_ptr<Formation> > &formations) {
    desiredDistance_ = desiredDistance;
    formations_=formations;
    selectedFormation_=0;
    nShips=0;
    cardHeight_=200;
    cardWidth_=130;
    bool notSetSize=true;
    for (const auto &formation : formations_) {
        nShips+=static_cast<int> (formation->getShips().size());
        if (notSetSize && !formation->getShips().empty()) {
            cardWidth_=formation->getShips()[0]->getCardTexture()->getWidth();
            cardHeight_=formation->getShips()[0]->getCardTexture()->getHeight();
            notSetSize=false;
        }

    }
}

void FormationManager::update() {
    //TODO, remove empty formations

    for (auto &formation : formations_)
        formation->update();


    size_t newNShips=0;
    removeEmptyFormations();
    for (const auto &formation : formations_)
        newNShips+=formation->getShips().size();
    if (newNShips!=nShips) {
        nShips=static_cast<int> (newNShips);
        //deselect selection, since something changed
        pickCard_=false;
    }

}

bool FormationManager::updateGraphical(int screenWidth, int screenHeight,const InputData &userInputs) {
    int y = screenHeight-cardHeight_;
    int x = screenWidth/2-(nShips*cardWidth_)/2-static_cast<int>(formations_.size())*4/*A little space betwixt formations*/;
    int width = (nShips*cardWidth_)+static_cast<int>(formations_.size())*8/*A little space betwixt formations*/;
    //Display a faux formation at the right end if we are picking a card
    if (pickCard_)
        width+=8+cardWidth_;

    bool hoverCards = (userInputs.mouseXPx>x && userInputs.mouseXPx<x+width && userInputs.mouseYPx>y);

    //Drop the card if the selection magically became invalid
    if (pickCard_ && (selectedCardFormation_<0 || selectedCardFormation_>=formations_.size() || selectedCardShip_<0 || formations_[selectedCardFormation_]->getShips().size()<=selectedCardShip_) ) {
        pickCard_=false;
    }

    //Pick up a card or select formation
    bool rightPressed = (userInputs.rightMouseDown && !userInputs.prevRightMouseDown);
    bool leftPressed = (userInputs.leftMouseDown && !userInputs.prevLeftMouseDown);
    if (!pickCard_ && hoverCards && (leftPressed || rightPressed)) {
        bool foundCard=false;
        for (int i = 0; i < formations_.size(); ++i) {
            const auto& ships = formations_[i]->getShips();
            x+=4;
            //Print the ship cards which are not picked up
            for (int j = 0; j < ships.size(); ++j) {
                if (userInputs.mouseXPx>x && userInputs.mouseXPx<x+cardWidth_ && userInputs.mouseYPx>y) {
                    //Either pick up the card, or select the formation
                    if (leftPressed) {
                        selectedCardFormation_=i;
                        selectedCardShip_=j;
                        pickCard_=true;
                    }
                    else if (rightPressed) {
                        selectedFormation_=i;
                    }
                    foundCard=true;
                    break;
                }
                x+=cardWidth_;
            }
            if (foundCard)
                break;
            x+=4;
        }
    }
    //Drop card
    else if (pickCard_ && !userInputs.leftMouseDown) {
        //Just return it to its old formation if we drop it outside
        if (!hoverCards) {
            //Nothing to do
        }
        else {
            bool foundDropOff = false;
            //Loop through the formations, and determine where it ends up
            //All the things which modify x are in another part of the if-else branch, so using x here is fine
            for (int i = 0; i < formations_.size(); ++i) {
                const auto& ships = formations_[i]->getShips();
                bool xGreater = userInputs.mouseXPx>x;
                x+=4;
                //Print the ship cards which are not picked up
                for (int j = 0; j < ships.size(); ++j) {
                    x+=cardWidth_;
                }
                x+=4;
                if (xGreater && userInputs.mouseXPx<x) {
                    moveToFormation(i);
                    foundDropOff = true;
                    break;
                }
            }
            //Then it MUST be in the new formation slot
            if (!foundDropOff) {
                createNewFormation();
            }
        }
        pickCard_=false;
    }

    return hoverCards || pickCard_;
}

void FormationManager::createNewFormation() {
    //Safety check, likely not needed
    if (!pickCard_  || (selectedCardFormation_<0 || selectedCardFormation_>=formations_.size() || selectedCardShip_<0 || formations_[selectedCardFormation_]->getShips().size()<=selectedCardShip_) ) {
        pickCard_=false;
        return;
    }
    auto& ships = formations_[selectedCardFormation_]->getShips();
    std::vector<std::shared_ptr<Ship>> newShips{
        ships[selectedCardShip_]
    };

    //Inherit all other information from the previous formation
    std::deque<glm::dvec2> waypoints = formations_[selectedCardFormation_]->getWaypoints();
    formations_.push_back(std::make_shared<Formation>(desiredDistance_,newShips,waypoints,formations_[selectedCardFormation_]->getFormationRadarOn()));

    formations_[selectedCardFormation_]->removeShip(selectedCardShip_);
    pickCard_=false;
    removeEmptyFormations();
}

void FormationManager::moveToFormation(int newFormation) {
    //Safety check, likely not needed
    if (newFormation==selectedCardFormation_ || !pickCard_  || newFormation<0 || newFormation>=formations_.size() || (selectedCardFormation_<0 || selectedCardFormation_>=formations_.size() || selectedCardShip_<0 || formations_[selectedCardFormation_]->getShips().size()<=selectedCardShip_) ) {
        pickCard_=false;
        return;
    }

    auto& ships = formations_[selectedCardFormation_]->getShips();
    formations_[newFormation]->addShip(ships[selectedCardShip_]);
    formations_[selectedCardFormation_]->removeShip(selectedCardShip_);
    pickCard_=false;
    removeEmptyFormations();

}

void FormationManager::removeEmptyFormations() {
    bool erased = false;
    for (int i = static_cast<int>(formations_.size())-1; i >= 0; i--) {
        if (formations_[i]->getShips().empty()) {
            erased = true;
            if (i==selectedFormation_)
                selectedFormation_=-1;
            else if (i<selectedCardFormation_)
                --selectedCardFormation_;
            formations_.erase(formations_.begin() + i);
        }
    }

    //Just drop the selection if something changed
    if (erased) {
        pickCard_=false;
    }
}




void FormationManager::renderGUI(SDL_Renderer *renderer, int screenWidth, int screenHeight,const InputData& userInputs) const {
    int y = screenHeight-cardHeight_;
    int x = screenWidth/2-(nShips*cardWidth_)/2-static_cast<int>(formations_.size())*4/*A little space betwixt formations*/;

    for (int i = 0; i < formations_.size(); ++i) {
        //Make a little background for each formation
        switch (i%6) {
            case 0:
            default:
                SDL_SetRenderDrawColor(renderer, 128, 128, 255, 255);
                break;
            case 1:
                SDL_SetRenderDrawColor(renderer, 128, 255, 128, 255);
                break;
            case 2:
                SDL_SetRenderDrawColor(renderer, 255, 128, 128, 255);
                break;
            case 3:
                SDL_SetRenderDrawColor(renderer, 128, 255, 255, 255);
                break;
            case 4:
                SDL_SetRenderDrawColor(renderer, 255, 128, 255, 255);
                break;
            case 5:
                SDL_SetRenderDrawColor(renderer, 255, 255, 128, 255);
                break;
        }
        const auto& ships = formations_[i]->getShips();
        SDL_Rect background {x,y-(selectedFormation_==i? 16:4),cardWidth_*static_cast<int>(ships.size())+8,cardHeight_+(selectedFormation_==i? 16:4)};
        SDL_RenderFillRect(renderer, &background);
        x+=4;
        //Print the ship cards which are not picked up
        for (int j = 0; j < ships.size(); ++j) {
            //Skip cards we have picked up
            if (pickCard_ && j==selectedCardShip_ && i==selectedCardFormation_) {
                x+=cardWidth_;
                continue;
            }
            ships[j]->getCardTexture()->render(x,y,renderer);
            if (userInputs.mouseXPx>x && userInputs.mouseXPx<x+cardWidth_ && userInputs.mouseYPx>y) {
                //Selected, print the name, with a slight black outline
                ships[j]->getNameTexture()->render(x+2,y-4-ships[j]->getNameTexture()->getHeight(),0,0,0,renderer);
                ships[j]->getNameTexture()->render(x,y-6-ships[j]->getNameTexture()->getHeight(),renderer);
            }
            x+=cardWidth_;
        }
        x+=4;
    }
    if (pickCard_) {
        //Draw a formation where we can drop of the card
        switch (formations_.size()%6) {
            case 0:
            default:
                SDL_SetRenderDrawColor(renderer, 128, 128, 255, 255);
                break;
            case 1:
                SDL_SetRenderDrawColor(renderer, 128, 255, 128, 255);
                break;
            case 2:
                SDL_SetRenderDrawColor(renderer, 255, 128, 128, 255);
                break;
            case 3:
                SDL_SetRenderDrawColor(renderer, 128, 255, 255, 255);
                break;
            case 4:
                SDL_SetRenderDrawColor(renderer, 255, 128, 255, 255);
                break;
            case 5:
                SDL_SetRenderDrawColor(renderer, 255, 255, 128, 255);
                break;
        }
        SDL_Rect background {x,y-4,cardWidth_+8,cardHeight_+4};
        SDL_RenderFillRect(renderer, &background);

        //Draw the card we have picked up
        if (selectedCardFormation_ < formations_.size()) {
            const auto& ships = formations_[selectedCardFormation_]->getShips();
            if (selectedCardShip_ < ships.size()) {
                ships[selectedCardShip_]->getCardTexture()->render(userInputs.mouseXPx,userInputs.mouseYPx,renderer,1.0,true,true);
                ships[selectedCardShip_]->getNameTexture()->render(userInputs.mouseXPx-cardWidth_/2+2,userInputs.mouseYPx-cardHeight_/2-4-ships[selectedCardShip_]->getNameTexture()->getHeight(),0,0,0,renderer);
                ships[selectedCardShip_]->getNameTexture()->render(userInputs.mouseXPx-cardWidth_/2,userInputs.mouseYPx-cardHeight_/2-6-ships[selectedCardShip_]->getNameTexture()->getHeight(),renderer);
            }
        }
    }
}

void FormationManager::render(SDL_Renderer *renderer, double mapTopLeftX, double mapTopLeftY, double scale) const {
    for (const auto &formation : formations_)
        formation->render(renderer, mapTopLeftX, mapTopLeftY, scale);
}



