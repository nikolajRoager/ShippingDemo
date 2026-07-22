//
// Created by nikolaj on 2/27/26.
//

#ifndef MIGUIDEMO_TEXTURECONTROL_H
#define MIGUIDEMO_TEXTURECONTROL_H
#include <utility>

#include "control.h"
#include "../TexWrap.h"

class textureControl : public control {
public:
    explicit textureControl(std::shared_ptr<const TexWrap> tex, bool background=false);
    ~textureControl() override = default;

    void render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;
    void updateSize() {
        width_=texture_->getWidth();
        height_=texture_->getHeight();
        widthChanged_= true;
        heightChanged_ = true;
    }

    void setTexture(std::shared_ptr<const TexWrap> tex) {
        texture_ = std::move(tex);
        updateSize();
    }


    void setBackground(bool background) {
        background_ = background;
    }
private:
    bool background_;

    std::shared_ptr<const TexWrap> texture_;
};


#endif //MIGUIDEMO_TEXTURECONTROL_H