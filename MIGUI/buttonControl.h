//
// Created by nikolaj on 2/28/26.
//

#ifndef MIGUIDEMO_BUTTON_H
#define MIGUIDEMO_BUTTON_H
#include <iostream>

#include "control.h"


///A clickable button, with a single control inside, which we shrink to fit
class buttonControl : public control{
public:
    explicit buttonControl(const std::shared_ptr<control> &content, Uint8 R=150, Uint8 G=150, Uint8 B=150, bool expandWidth=false);
    ~buttonControl() override = default;

    ///Render the control
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;
    ///Update the control
    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) override;

    ///Update how much width is available, the control might or might not  care depending on the class
    void setAvailableWidth(int width) override {
        content_->setAvailableWidth(width);
        if (expandWidth_) {width_ = std::max(width,width_); widthChanged_ = true;}
    };
    ///Update how much width is available, the control might or might not care depending on the class
    void setAvailableHeight(int height) override {
        content_->setAvailableHeight(height);
    };

    void setX0(int x0) override {x0_=x0; content_->setX0(x0_);};
    void setY0(int y0) override {
        y0_=y0; content_->setY0(y0_);
    };

    void setDepth(int depth) override {depth_ = depth; content_->setDepth(depth+1);};
    [[nodiscard]] int getMaxDepth() const override {
        return std::max(depth_,content_->getMaxDepth());
    }

    [[nodiscard]] bool isClicked() const {return clicked_;}
private:
    std::shared_ptr<control> content_;
    bool clicked_ = false;
    bool hover_ = false;
    bool expandWidth_ = false;

    Uint8 R_, G_, B_;
};


#endif //MIGUIDEMO_BUTTON_H