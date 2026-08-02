//
// Created by nikolaj on 7/22/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_INTELLIGENCEMANAGER_H
#define WHIRLWINDSOFDANGERSKETCH_INTELLIGENCEMANAGER_H
#include <memory>
#include <vector>

#include "ship.h"
#include "../TexWrap.h"

///Class which collects all intel reports and tries to figure out if ships are neutral or hostile
class IntelligenceManager {
public:
    IntelligenceManager(
        std::shared_ptr<const TexWrap> unknownShipNameTexture,
        std::shared_ptr<const TexWrap> sinkingShipNameTexture,
        std::shared_ptr<const TexWrap> transponderText,
        std::shared_ptr<const TexWrap> visionText,
        std::shared_ptr<const TexWrap> radarText,
        std::shared_ptr<const TexWrap> ESMText,
        const std::vector<std::shared_ptr<const TexWrap>>& sizeTexts,
        size_t nEnemies, size_t nNeutrals,
        double radarHorizonCoefficient,
        double clearDayVisionFraction,
        double clearNightVisionFraction,
        double rainyDayVisionFraction,
        double rainyNightVisionFraction,
        double referenceRadarHorizonHeight,
        double referenceSqrtSqrtRadarCrossSection,
        double ESMRangeFactor
        );
    void update(std::vector<std::shared_ptr<Ship>>& friendlyShips,std::vector<std::shared_ptr<Ship>>& enemyShips,std::vector<std::shared_ptr<Ship>>&  neutralShips, bool day=true, bool rain=false, bool writeEffectsToDisplay=false);
    void render(const std::vector<std::shared_ptr<Ship>>& friendlyShips,const std::vector<std::shared_ptr<Ship>>& enemyShips,const std::vector<std::shared_ptr<Ship>>&  neutralShips,const std::unique_ptr<NATOSymbolManager>& symbolManager,SDL_Renderer* renderer, double mapTopLeftX, double mapTopLeftY, double scale, int mouseX, int mouseY, bool day=true, bool rain=false) const;
private:
    ///How much further can ESM see than the radar can see
    double ESMRangeFactor_;
    ///What is the 4th root of the reference radar cross section we will use to display radar range
    double referenceSqrtSqrtRadarCrossSection_;
    ///What height target object is used as a reference for the displayed radar horizon range, already square-rooted
    double referenceRadarHorizonSqrtHeight_;
    ///Coefficient C in the equation radarHorizon = C*(sqrt(own height)+sqrt(target height)), which is a reasonable approximation for radar horizon distance
    double radarHorizonCoefficient_;
    ///What fraction of radar horizon range can we see in clear weather day
    double clearDayVisionFraction_;
    ///What fraction of radar horizon range can we see in clear weather at night
    double clearNightVisionFraction_;
    ///What fraction of radar horizon range can we see in rainy weather at day
    double rainyDayVisionFraction_;
    ///What fraction of radar horizon range can we see in rainy weather at night
    double rainyNightVisionFraction_;


    struct Contact {
        ///Radar cross-section^0.25 goes from 0.4 (drone) to 12.5 (USS Gerald R. Ford or supertankers), let us apply some words to describe them
        enum Size {
            TINY,//0.5 or less: drones
            VERY_SMALL,//0.5-1.5: Missiles
            SMALL,//1.5-3.5 Planes and helicopters
            MEDIUM_SMALL,//3.5-5.5 Fishing trawlers, patrol boats
            MEDIUM,//5.5-7.5 Corvettes and small frigates
            LARGE,//7.5-9.5 large frigates and destroyers
            VERY_LARGE,//9.5-11.5 Kirov, Oil tankers
            HUGE,//11.5 or above: Supercarriers, Supertankers
        } size=TINY;
        enum Status {HIDDEN,SPOTTED,IDENTIFIED} status_=HIDDEN;
        glm::dvec2 position_=glm::dvec2(0.0);
        NATOSymbolManager::ShipType type_=NATOSymbolManager::UNKNOWN;
        ///How has this thing been spotted this update cycle?
        bool transponder=false, radar=false, esm=false, vision=false;
    };

    ///Whether each enemy is known or not
    std::vector<Contact> enemyContacts_;
    ///Whether each neutral is known or not
    std::vector<Contact> neutralContacts_;

    std::shared_ptr<const TexWrap> unknownShipNameText_;
    std::shared_ptr<const TexWrap> sinkingShipNameText_;
    std::shared_ptr<const TexWrap> transponderText_;
    std::shared_ptr<const TexWrap> visionText_;
    std::shared_ptr<const TexWrap> radarText_;
    std::shared_ptr<const TexWrap> ESMText_;
    std::vector<std::shared_ptr<const TexWrap>> sizeTexts_;


    ///The update function for a single ship, which may be enemy of neutral
    void updateShip(std::vector<std::shared_ptr<Ship> > &friendlyShips, int i, std::vector<Contact>& contact, std::shared_ptr<Ship>& ship, bool writeEffectsToDisplay,bool day, bool rain) const;

    void printContactInfo(SDL_Renderer* renderer,std::shared_ptr<const TexWrap> nameTexture, const Contact& contact, int screenX, int screenY) const;
};


#endif //WHIRLWINDSOFDANGERSKETCH_INTELLIGENCEMANAGER_H