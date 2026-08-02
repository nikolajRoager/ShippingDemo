//
// Created by nikolaj on 7/22/26.
//

#include "intelligenceManager.h"

#include <iostream>
#include <utility>

#include "drawCircle.h"

IntelligenceManager::IntelligenceManager(
    std::shared_ptr<const TexWrap> unknownShipNameTexture,
    std::shared_ptr<const TexWrap> sinkingShipNameTexture,
std::shared_ptr<const TexWrap> transponderText,
std::shared_ptr<const TexWrap> visionText,
std::shared_ptr<const TexWrap> radarText,
std::shared_ptr<const TexWrap> ESMText,
const std::vector<std::shared_ptr<const TexWrap>>& sizeTexts,
    size_t nEnemies, size_t nNeutrals,double radarHorizonCoefficient,
    double clearDayVisionFraction,
    double clearNightVisionFraction,
    double rainyDayVisionFraction,
    double rainyNightVisionFraction,
    double referenceRadarHorizonHeight,
    double referenceSqrtSqrtRadarCrossSection,
    double ESMRangeFactor
) {
    ESMRangeFactor_ = ESMRangeFactor;
    referenceSqrtSqrtRadarCrossSection_=referenceSqrtSqrtRadarCrossSection;
    referenceRadarHorizonSqrtHeight_= std::sqrt(referenceRadarHorizonHeight);
    radarHorizonCoefficient_ = radarHorizonCoefficient;
    clearDayVisionFraction_ = clearDayVisionFraction;
    clearNightVisionFraction_ = clearNightVisionFraction;
    rainyDayVisionFraction_ = rainyDayVisionFraction;
    rainyNightVisionFraction_ = rainyNightVisionFraction;

    sinkingShipNameText_ = std::move(sinkingShipNameTexture);
    unknownShipNameText_ = std::move(unknownShipNameTexture);
    transponderText_ = std::move(transponderText);
    ESMText_ = std::move(ESMText);
    visionText_ = std::move(visionText);
    radarText_ = std::move(radarText);
    sizeTexts_=sizeTexts;
    //Ships don't get erased when they sink so we can initialize the vectors at startup

    enemyContacts_=std::vector<Contact>(nEnemies);
    neutralContacts_=std::vector<Contact>(nNeutrals);
}

void IntelligenceManager::update(std::vector<std::shared_ptr<Ship> > &friendlyShips, std::vector<std::shared_ptr<Ship> > &enemyShips, std::vector<std::shared_ptr<Ship> > &neutralShips,  bool day, bool rain, bool writeEffectsToDisplay) {
    for (int i = 0; i < friendlyShips.size(); i++) {
        auto &ship = friendlyShips[i];
        //Friendly ships are always spotted
        if (writeEffectsToDisplay) {
            ship->setIdentified(true);
        }
    }

    for (int i = 0; i < neutralShips.size(); ++i) {
        auto &ship = neutralShips[i];
        updateShip(friendlyShips,i,neutralContacts_,ship,writeEffectsToDisplay,day,rain);
    }
    for (int i = 0; i < enemyShips.size(); ++i) {
        auto &ship = enemyShips[i];
        updateShip(friendlyShips,i,enemyContacts_,ship,writeEffectsToDisplay,day,rain);
    }
}

void IntelligenceManager::updateShip(std::vector<std::shared_ptr<Ship> > &friendlyShips, int i, std::vector<Contact> &contacts, std::shared_ptr<Ship> &ship, bool writeEffectsToDisplay,bool day, bool rain) const {
    //Reset our reason for detecting this
    contacts[i].esm=false;
    contacts[i].vision=false;
    contacts[i].transponder=false;
    contacts[i].radar=false;

    bool refreshedId = false;
    bool refreshSpotted = false;
    //If anybody has their transponder on, they are automatically detected
    //TODO, only if we are on the surface
    if (ship->transponderOn()) {
        contacts[i].status_=Contact::IDENTIFIED;
        contacts[i].position_=ship->getPosition();
        contacts[i].type_=ship->getType();
        contacts[i].transponder=true;
        refreshedId = true;
        refreshSpotted = true;
    }

    for (const auto & spotter : friendlyShips) {
        auto D = spotter->getPosition()-ship->getPosition();
        double dist2 = (D.x*D.x)+(D.y*D.y);

        double horizonRange = radarHorizonCoefficient_*(spotter->getSqrtHeight()+ship->getSqrtHeight());
        double visualRange = horizonRange;
        if (day && rain) {
            visualRange*=rainyDayVisionFraction_;
        }
        else if (!day && rain) {
            visualRange*=rainyNightVisionFraction_;
        }
        else if (day && !rain) {
            visualRange*=clearDayVisionFraction_;
        }
        else /*(!day && !rain)*/{
            visualRange*=clearNightVisionFraction_;
        }

        //Visual spotting is always on
        if (dist2<visualRange*visualRange) {
            //Visually spotted
            contacts[i].status_=Contact::IDENTIFIED;
            contacts[i].position_=ship->getPosition();
            contacts[i].type_=ship->getType();
            contacts[i].vision=true;
            refreshedId = true;
            refreshSpotted = true;
        }

        if (spotter->radarOn() && dist2<horizonRange*horizonRange) {
            double sigmaFourthRoot =ship->getRadarCrossSectionFourthRoot();
            double powerRange = spotter->getRadarCoefficient()*sigmaFourthRoot;
            if (dist2 < powerRange*powerRange) {
                //Promote hidden to spotted
                if (contacts[i].status_==Contact::HIDDEN)
                    contacts[i].status_=Contact::SPOTTED;
                refreshSpotted = true;
                contacts[i].position_=ship->getPosition();
                contacts[i].radar=true;
                //Get radar cross-section size
                if (sigmaFourthRoot<0.5)
                    contacts[i].size=Contact::TINY;
                else if (sigmaFourthRoot<1.5)
                    contacts[i].size=Contact::VERY_SMALL;
                else if (sigmaFourthRoot<3.5)
                    contacts[i].size=Contact::SMALL;
                else if (sigmaFourthRoot<5.5)
                    contacts[i].size=Contact::MEDIUM_SMALL;
                else if (sigmaFourthRoot<7.5)
                    contacts[i].size=Contact::MEDIUM;
                else if (sigmaFourthRoot<9.5)
                    contacts[i].size=Contact::LARGE;
                else if (sigmaFourthRoot<1.5)
                    contacts[i].size=Contact::VERY_LARGE;
                else
                    contacts[i].size=Contact::HUGE;

            }
        }
        //spotter ESM is always on
        if (ship->radarOn() && dist2< horizonRange*horizonRange) {
            double powerRange = ESMRangeFactor_*ship->getRadarCoefficient()*spotter->getRadarCrossSectionFourthRoot();
            if (dist2 < powerRange*powerRange) {
                //Promote hidden to spotted
                contacts[i].status_=Contact::IDENTIFIED;
                contacts[i].type_=ship->getType();
                refreshSpotted = true;
                refreshedId = true;
                contacts[i].position_=ship->getPosition();
                contacts[i].esm=true;
            }
        }
    }


    //If we lost track of something we previously identified, loose track
    if (!refreshSpotted) {
        contacts[i].status_=Contact::HIDDEN;
        contacts[i].type_=NATOSymbolManager::UNKNOWN;
        if (writeEffectsToDisplay) {
            ship->setIdentified(false);
        }
    }
    //Visually display identified targets
    if (contacts[i].status_==Contact::SPOTTED || contacts[i].status_==Contact::IDENTIFIED) {
        if (writeEffectsToDisplay) {
            ship->setIdentified(true);
        }
    }
}

void IntelligenceManager::printContactInfo(SDL_Renderer* renderer,std::shared_ptr<const TexWrap> nameTexture, const Contact &contact, int screenX, int screenY) const {
    nameTexture->render(screenX+2,screenY+2,0,0,0,renderer);
    nameTexture->render(screenX,screenY,renderer);
    screenY+=nameTexture->getHeight();
    if (contact.transponder) {
        transponderText_->render(screenX+2,screenY+2,0,0,0,renderer);
        transponderText_->render(screenX,screenY,renderer);
        screenY+=transponderText_->getHeight();
    }
    if (contact.vision) {
        visionText_->render(screenX+2,screenY+2,0,0,0,renderer);
        visionText_->render(screenX,screenY,renderer);
        screenY+=visionText_->getHeight();
    }
    if (contact.radar) {
        radarText_->render(screenX+2,screenY+2,0,0,0,renderer);
        radarText_->render(screenX,screenY,renderer);
        sizeTexts_[contact.size]->render(screenX+radarText_->getWidth(),screenY,renderer);
        screenY+=radarText_->getHeight();
    }
    if (contact.esm) {
        ESMText_->render(screenX+2,screenY+2,0,0,0,renderer);
        ESMText_->render(screenX,screenY,renderer);
    }
}


void IntelligenceManager::render(const std::vector<std::shared_ptr<Ship>>& friendlyShips,const std::vector<std::shared_ptr<Ship>>& enemyShips,const std::vector<std::shared_ptr<Ship>>&  neutralShips,const std::unique_ptr<NATOSymbolManager>& symbolManager,SDL_Renderer *renderer, double mapTopLeftX, double mapTopLeftY, double scale, int mouseX, int mouseY, bool day, bool rain) const {
    int offsetX=symbolManager->getSymbolWidth()/2;
    int offsetY=-symbolManager->getSymbolHeight()/2;
    //We will collect all range circles, and displaying them once we have all so that we can exclude overlap
    std::vector<CircleRecord> friendlyVisionCircles(friendlyShips.size());
    std::vector<CircleRecord> friendlyRadarHorizonCircles(friendlyShips.size());
    std::vector<CircleRecord> friendlyRadarPowerCircles(friendlyShips.size());
    for (int i = 0; i < friendlyShips.size(); ++i) {
        const auto& ship = friendlyShips[i];
        auto pos = ship->getPosition();
        int screenX =static_cast<int>(pos.x*scale+mapTopLeftX);
        int screenY =static_cast<int>(pos.y*scale+mapTopLeftY);

        symbolManager->renderShip(screenX,screenY,NATOSymbolManager::FRIEND,ship->getType(),ship->getHealth()<=0,renderer);

        double radarHorizonRange = scale*radarHorizonCoefficient_*(referenceRadarHorizonSqrtHeight_+ship->getSqrtHeight());


        //Always display gun ranges
        CircleRecord seaGunCircle;
        seaGunCircle.x_=screenX;
        seaGunCircle.y_=screenY;
        seaGunCircle.radius_=ship->getGunSeaRange()*scale;
        seaGunCircle.radius2_=seaGunCircle.radius_*seaGunCircle.radius_;

        drawCircle(renderer,seaGunCircle,0,0,0);

        CircleRecord airGunCircle;
        airGunCircle.x_=screenX;
        airGunCircle.y_=screenY;
        airGunCircle.radius_=ship->getGunAirRange()*scale;
        airGunCircle.radius2_=airGunCircle.radius_*airGunCircle.radius_;

        drawCircle(renderer,airGunCircle,128,128,128);


        if (ship->radarOn()) {
            friendlyRadarHorizonCircles[i].x_=screenX;
            friendlyRadarHorizonCircles[i].y_=screenY;
            friendlyRadarHorizonCircles[i].radius_=radarHorizonRange;
            friendlyRadarHorizonCircles[i].radius2_=radarHorizonRange*radarHorizonRange;

            double radarPowerRange = scale*ship->getRadarCoefficient()*referenceSqrtSqrtRadarCrossSection_;

            friendlyRadarPowerCircles[i].x_=screenX;
            friendlyRadarPowerCircles[i].y_=screenY;
            friendlyRadarPowerCircles[i].radius_=radarPowerRange ;
            friendlyRadarPowerCircles[i].radius2_=radarPowerRange*radarPowerRange;
        }

        if (day && rain) {
            radarHorizonRange*=rainyDayVisionFraction_;
        }
        else if (!day && rain) {
            radarHorizonRange*=rainyNightVisionFraction_;
        }
        else if (day && !rain) {
            radarHorizonRange*=clearDayVisionFraction_;
        }
        else /*(!day && !rain)*/{
            radarHorizonRange*=clearNightVisionFraction_;
        }
        friendlyVisionCircles[i].x_=screenX;
        friendlyVisionCircles[i].y_=screenY;
        friendlyVisionCircles[i].radius_=radarHorizonRange;
        friendlyVisionCircles[i].radius2_=radarHorizonRange*radarHorizonRange;
    }

    //draw all circles
    for (int i= 0; i < friendlyShips.size(); ++i) {
        drawCircle(renderer,i,friendlyVisionCircles,255,255,0);
        drawCircle(renderer,i,friendlyRadarHorizonCircles,0,255,0);
        drawCircle(renderer,i,friendlyRadarPowerCircles,0,255, 0 ,100,50);
    }

    for (int i = 0 ; i < neutralContacts_.size(); i++) {
        const auto& contact = neutralContacts_[i];
        if (contact.status_==Contact::HIDDEN)
            continue;
        int screenX =static_cast<int>(contact.position_.x*scale+mapTopLeftX);
        int screenY =static_cast<int>(contact.position_.y*scale+mapTopLeftY);
        if (contact.status_==Contact::SPOTTED) {
            symbolManager->renderShip(screenX,screenY,NATOSymbolManager::UNKNOWN_SIDE,contact.type_,neutralShips[i]->getHealth()<=0,renderer);
        }
        else if (contact.status_==Contact::IDENTIFIED) {
            symbolManager->renderShip(screenX,screenY,NATOSymbolManager::NEUTRAL,contact.type_,neutralShips[i]->getHealth()<=0,renderer);
        }
    }
    for (int i = 0 ; i < enemyContacts_.size(); i++) {
        const auto& contact = enemyContacts_[i];
        if (contact.status_==Contact::HIDDEN)
            continue;
        int screenX =static_cast<int>(contact.position_.x*scale+mapTopLeftX);
        int screenY =static_cast<int>(contact.position_.y*scale+mapTopLeftY);
        if (contact.status_==Contact::SPOTTED) {
            symbolManager->renderShip(screenX,screenY,NATOSymbolManager::UNKNOWN_SIDE,contact.type_,enemyShips[i]->getHealth()<=0,renderer);
        }
        else if (contact.status_==Contact::IDENTIFIED) {
            symbolManager->renderShip(screenX,screenY,NATOSymbolManager::FOE,contact.type_,enemyShips[i]->getHealth()<=0,renderer);
        }
    }
    //Make a second pass to print the names of the selected ship on top
    //We will only print the name of one ship
    bool firstPrinted = false;
    for (const auto& ship : friendlyShips) {
        auto pos = ship->getPosition();
        int screenX =static_cast<int>(pos.x*scale+mapTopLeftX);
        int screenY =static_cast<int>(pos.y*scale+mapTopLeftY);
        if (!firstPrinted) {
            int mouseXDist = (screenX-mouseX);
            int mouseYDist = (screenY-mouseY);

            if (std::abs(mouseXDist)<25 && std::abs(mouseYDist)<25) {
                ship->getNameTexture()->render(screenX+offsetX+2,screenY+offsetY+2,0,0,0,renderer);
                ship->getNameTexture()->render(screenX+offsetX,screenY+offsetY,renderer);
                firstPrinted = true;
            }
        }

    }
    for (int i = 0 ; i < neutralContacts_.size(); i++) {
        const auto& contact = neutralContacts_[i];
        if (contact.status_==Contact::HIDDEN)
            continue;
        int screenX =static_cast<int>(contact.position_.x*scale+mapTopLeftX);
        int screenY =static_cast<int>(contact.position_.y*scale+mapTopLeftY);
        if (contact.status_==Contact::SPOTTED) {
            if (!firstPrinted) {
                int mouseXDist = (screenX-mouseX);
                int mouseYDist = (screenY-mouseY);

                if (std::abs(mouseXDist)<25 && std::abs(mouseYDist)<25) {
                    printContactInfo(renderer,unknownShipNameText_,contact,screenX+offsetX,screenY+offsetY);
                    firstPrinted = true;
                }
            }
        }
        else if (contact.status_==Contact::IDENTIFIED) {
            if (!firstPrinted) {
                int mouseXDist = (screenX-mouseX);
                int mouseYDist = (screenY-mouseY);

                if (std::abs(mouseXDist)<25 && std::abs(mouseYDist)<25) {
                    printContactInfo(renderer,neutralShips[i]->getNameTexture(),contact,screenX+offsetX,screenY+offsetY);
                    firstPrinted = true;
                }
            }
        }
    }
    for (int i = 0 ; i < enemyContacts_.size(); i++) {
        const auto& contact = enemyContacts_[i];
        if (contact.status_==Contact::HIDDEN)
            continue;
        int screenX =static_cast<int>(contact.position_.x*scale+mapTopLeftX);
        int screenY =static_cast<int>(contact.position_.y*scale+mapTopLeftY);
        if (contact.status_==Contact::SPOTTED) {
            if (!firstPrinted) {
                int mouseXDist = (screenX-mouseX);
                int mouseYDist = (screenY-mouseY);

                if (std::abs(mouseXDist)<25 && std::abs(mouseYDist)<25) {
                    printContactInfo(renderer,unknownShipNameText_,contact,screenX+offsetX,screenY+offsetY);
                    firstPrinted = true;
                }
            }
        }
        else if (contact.status_==Contact::IDENTIFIED) {
            if (!firstPrinted) {
                int mouseXDist = (screenX-mouseX);
                int mouseYDist = (screenY-mouseY);

                if (std::abs(mouseXDist)<25 && std::abs(mouseYDist)<25) {
                    printContactInfo(renderer,enemyShips[i]->getNameTexture(),contact,screenX+offsetX,screenY+offsetY);
                    firstPrinted = true;
                }
            }
        }
    }
}