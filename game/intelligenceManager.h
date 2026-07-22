//
// Created by nikolaj on 7/22/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_INTELLIGENCEMANAGER_H
#define WHIRLWINDSOFDANGERSKETCH_INTELLIGENCEMANAGER_H
#include <memory>

#include "../TexWrap.h"

///Class which collects all intel reports and tries to figure out if ships are neutral or hostile
class IntelligenceManager {
public:
    void update();
    void Render(SDL_Renderer* renderer, double mapTopLeftX, double mapTopLeftY, double scale);
private:
    std::shared_ptr<const TexWrap> friendlyShipTexture_;
    std::shared_ptr<const TexWrap> enemyShipTexture_;
    std::shared_ptr<const TexWrap> neutralShipTexture_;
    std::shared_ptr<const TexWrap> unknownShipTexture_;

};


#endif //WHIRLWINDSOFDANGERSKETCH_INTELLIGENCEMANAGER_H