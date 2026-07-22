//
// Created by nikolaj on 2/25/26.
//

#ifndef MIGUIDEMO_CONTROL_H
#define MIGUIDEMO_CONTROL_H
#include <SDL2/SDL_render.h>
#include "../NumberRenderer.h"
#include "../inputData.h"
#include<memory>

class control {
public:
    control();
    virtual ~control() = default;

    ///Render the control
    virtual void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a)=0;
    ///Update the control
    virtual void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip);

    ///Update how much width is available, the control might or might not  care depending on the class
    virtual void setAvailableWidth(int /*width*/) {};
    ///Update how much width is available, the control might or might not care depending on the class
    virtual void setAvailableHeight(int /*height*/) {};

    ///Has the size of this element changed since last time we called this function
    bool getWidthChanged() {
        if (widthChanged_) {
            widthChanged_ = false;
            return true;
        }
        return false;
    };
    ///Has the size of this element changed since last time we called this function
    bool getHeightChanged() {
        if (heightChanged_) {
            heightChanged_ = false;
            return true;
        }
        return false;
    };

    virtual void setX0(int x0) {x0_=x0;}
    virtual void setY0(int y0) {y0_=y0;}
    [[nodiscard]] int getX0() const {return x0_;}
    [[nodiscard]] int getY0() const {return y0_;}
    [[nodiscard]] int virtual getWidth() const {return width_;}
    [[nodiscard]] int virtual getHeight() const {return height_;}
    [[nodiscard]] int getDepth() const {return depth_;}

    ///Are we hovering over the control or any of its children
    [[nodiscard]] bool getHoverInherited() const {return hoverInherited_;}

    virtual void setDepth(int depth) {depth_ = depth;}
    [[nodiscard]] virtual int getMaxDepth() const {return depth_;}
protected:

    bool hoverInherited_ = false;

    int depth_;

    ///Top left corner position of control
    int x0_,y0_;
    ///Width/height of control
    int width_,height_;
    ///Flag that width changed
    bool widthChanged_=false;
    ///Flag that height changed
    bool heightChanged_=false;
};


#endif //MIGUIDEMO_CONTROL_H