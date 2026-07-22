//
// Created by nikolaj on 6/17/26.
//

#ifndef DUNGEONSKETCHTWO_NUMBERINPUTCONTROL_H
#define DUNGEONSKETCHTWO_NUMBERINPUTCONTROL_H
#include <algorithm>

#include "numberControl.h"


class NumberInputControl : public control {
public:
    explicit NumberInputControl(const NumberRenderer& numberRenderer, int value, int min, int max, std::shared_ptr<const TexWrap> plusTex, std::shared_ptr<const TexWrap> minusTex, Uint8 R=150, Uint8 G=150, Uint8 B=150);
    ~NumberInputControl() override = default;

    void render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;

    [[nodiscard]] int getValue() const {return value_;}
    void setValue(int value);

    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) override;

    [[nodiscard]] bool isClicked() const {return clicked_;}

    void setAllowIncrement(bool val) {allowIncrement_ = val;}
protected:
    const NumberRenderer& numberRenderer_;
    int value_;
    int min_;
    int max_;
    int numWidth_;
    std::shared_ptr<const TexWrap> plusTex_;
    std::shared_ptr<const TexWrap> minusTex_;
    bool hoverPlus_ = false;
    bool hoverMinus_ = false;
    bool allowIncrement_ = true;
    Uint8 R_;
    Uint8 G_;
    Uint8 B_;

    bool clicked_ = false;
};


#endif //DUNGEONSKETCHTWO_NUMBERINPUTCONTROL_H