//
// Created by nikolaj on 7/29/26.
//

#include "natoSymbolManager.h"

void NATOSymbolManager::requestTextures(std::vector<fs::path>& textureRequests) {
    textureRequests.push_back(fs::path("NATOSymbols")/"friendlyShip.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"enemyShip.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"unknownShip.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"neutralShip.png");

    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"unknown.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"containerShip.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"oilTanker.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"fisher.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"passengerShip.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"corvette.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"frigate.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"destroyer.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"cruiser.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"carrier.png");
    textureRequests.push_back(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"eliminated.png");
}

NATOSymbolManager::NATOSymbolManager(const TextureManager &manager) {
    eliminatedTexture_=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"eliminated.png");

    shipTexture_[FRIEND]=manager.getTexWrap(fs::path("NATOSymbols")/"friendlyShip.png");
    symbolWidth=shipTexture_[FRIEND]->getWidth();
    symbolHeight=shipTexture_[FRIEND]->getHeight();
    shipTexture_[FOE]=manager.getTexWrap(fs::path("NATOSymbols")/"enemyShip.png");
    shipTexture_[UNKNOWN_SIDE]=manager.getTexWrap(fs::path("NATOSymbols")/"unknownShip.png");
    shipTexture_[NEUTRAL]=manager.getTexWrap(fs::path("NATOSymbols")/"neutralShip.png");

    shipTypeTextures_[UNKNOWN]=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"unknown.png");
    shipTypeTextures_[CONTAINER_SHIP]=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"containerShip.png");
    shipTypeTextures_[OIL_TANKER]=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"oilTanker.png");
    shipTypeTextures_[FISHER]=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"fisher.png");
    shipTypeTextures_[PASSENGER_SHIP]=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"passengerShip.png");
    shipTypeTextures_[CORVETTE]=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"corvette.png");
    shipTypeTextures_[FRIGATE]=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"frigate.png");
    shipTypeTextures_[DESTROYER]=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"destroyer.png");
    shipTypeTextures_[CRUISER]=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"cruiser.png");
    shipTypeTextures_[CARRIER]=manager.getTexWrap(fs::path("NATOSymbols")/"militarySeaSurfaceIcons"/"carrier.png");
}

void NATOSymbolManager::renderShip(int x, int y, Side side, ShipType type, bool eliminated, SDL_Renderer *renderer) {
    shipTexture_[side]->render(x,y,renderer,1.0,true,true);
    shipTypeTextures_[type]->render(x,y,renderer,1.0,true,true);
    if (eliminated && side != UNKNOWN_SIDE)
        eliminatedTexture_->render(x,y,renderer,1.0,true,true);
}
