//
// Created by nikolaj on 6/14/26.
//

#ifndef DUNGEONSKETCHTWO_SLIDECONTROL_H
#define DUNGEONSKETCHTWO_SLIDECONTROL_H
#include <vector>

#include "control.h"


class SlideControl : public control {
public:


    explicit SlideControl(const std::vector<std::shared_ptr<control>> &children);
    ~SlideControl() override =default;
    ///Render the control
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;
    ///Update the control
    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) override;

    void setX0(int x0) override;
    void setY0(int y0) override;
    void setDepth(int depth) override;
    [[nodiscard]] int getMaxDepth() const override;
    void setActiveSlide(size_t i) {
        activeChild_=i;
        widthChanged_=true;
        heightChanged_=true;
        width_=children_[activeChild_]->getWidth();
        height_=children_[activeChild_]->getHeight();
    }

    ///Update how much width is available, the control might or might not  care depending on the class
    void setAvailableWidth(int width) override {
        for (const auto& control : children_) {
            control->setAvailableWidth(width);
        }
    };
    ///Update how much width is available, the control might or might not care depending on the class
    void setAvailableHeight(int height) override {
        for (const auto& control : children_) {
            control->setAvailableHeight(height);
        }
    };

    [[nodiscard]] size_t getActiveSlide() const {return activeChild_;}

private:
    size_t activeChild_;
    std::vector<std::shared_ptr<control>> children_;

};


#endif //DUNGEONSKETCHTWO_SLIDECONTROL_H