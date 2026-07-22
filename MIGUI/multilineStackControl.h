//
// Created by nikolaj on 7/16/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_MULTILINESTACKCONTROL_H
#define WHIRLWINDSOFDANGERSKETCH_MULTILINESTACKCONTROL_H
#include <vector>

#include "control.h"


class MultilineStackControl : public control {
public:

    MultilineStackControl(const std::vector<std::shared_ptr<control>> &children, int spacing=0);
    ~MultilineStackControl() override =default;
    ///Render the control
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;
    ///Update the control
    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) override;

    void setAvailableWidth(int) override;

    void setX0(int x0) override;
    void setY0(int y0) override;
    void setDepth(int depth) override;
    [[nodiscard]] int getMaxDepth() const override;
private:

    void updateChildrenLocation();
    int spacing_;
    std::vector<std::shared_ptr<control>> children_;

};


#endif //WHIRLWINDSOFDANGERSKETCH_MULTILINESTACKCONTROL_H