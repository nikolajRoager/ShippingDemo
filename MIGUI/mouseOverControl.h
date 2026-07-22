//
// Created by nikolaj on 3/7/26.
//

#ifndef MIGUIDEMO_MOUSEOVERCONTROL_H
#define MIGUIDEMO_MOUSEOVERCONTROL_H
#include "control.h"


class mouseOverControl : public control {
public:

    mouseOverControl(const std::shared_ptr<control> &content, const std::shared_ptr<control> &mouseOverContent, Uint8 R=0, Uint8 G=0, Uint8 B=0);
    ~mouseOverControl() override = default;

    ///Render the control
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;
    ///Update the control
    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) override;

    ///Update how much width is available, the control might or might not  care depending on the class
    void setAvailableWidth(int width) override {
        content_->setAvailableWidth(width);
    };
    ///Update how much width is available, the control might or might not care depending on the class
    void setAvailableHeight(int height) override {
        content_->setAvailableHeight(height);
    };

    void setX0(int x0) override {x0_=x0; content_->setX0(x0_);};
    void setY0(int y0) override {y0_=y0; content_->setY0(y0_);};

    void setDepth(int depth) override {depth_ = depth; content_->setDepth(depth+1); mouseOverContent_->setDepth(depth+10);};
    [[nodiscard]] int getMaxDepth() const override {
        if (hover_)
            return std::max(std::max(depth_,content_->getMaxDepth()),mouseOverContent_->getMaxDepth());
        return std::max(depth_,content_->getMaxDepth());
    }
private:

    std::shared_ptr<control> content_;
    std::shared_ptr<control> mouseOverContent_;
    bool hover_ = false;

    Uint8 R_, G_, B_;
};


#endif //MIGUIDEMO_MOUSEOVERCONTROL_H