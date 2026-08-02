//
// Created by nikolaj on 7/29/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_NATOSYMBOLMANAGER_H
#define WHIRLWINDSOFDANGERSKETCH_NATOSYMBOLMANAGER_H
#include <map>
#include <memory>

#include "../textureManager.h"
#include "../TexWrap.h"

namespace fs = std::filesystem;

class NATOSymbolManager {
public:
    enum ShipType {UNKNOWN,CONTAINER_SHIP,OIL_TANKER,FISHER,PASSENGER_SHIP,CORVETTE,FRIGATE,DESTROYER,CRUISER,CARRIER};
    enum Side {FRIEND,FOE,NEUTRAL,UNKNOWN_SIDE};
    static void requestTextures(std::vector<fs::path>& textureRequests);
    explicit NATOSymbolManager(const TextureManager& manager);
    void renderShip(int x, int y, Side side, ShipType type, bool eliminated, SDL_Renderer* renderer);
    int getSymbolWidth() const {return symbolWidth;};
    int getSymbolHeight() const {return symbolHeight;}
private:
    int symbolWidth;
    int symbolHeight;
    std::map<Side,std::shared_ptr<const TexWrap>> shipTexture_;
    std::shared_ptr<const TexWrap> eliminatedTexture_;

    std::map<ShipType,std::shared_ptr<const TexWrap>> shipTypeTextures_;
};


#endif //WHIRLWINDSOFDANGERSKETCH_NATOSYMBOLMANAGER_H