//
// Created by nikolaj on 2/28/26.
//

#ifndef MIGUIDEMO_STACKCONTROL_H
#define MIGUIDEMO_STACKCONTROL_H
#include <vector>

#include "control.h"


///Stacked controls in a single line
class stackControl : public control {
public:
    enum orientation {
        HORIZONTAL,
        VERTICAL,
    };

    stackControl(orientation orientation, const std::vector<std::shared_ptr<control>> &children, int spacing=0);
    ~stackControl() override =default;
    ///Render the control
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;
    ///Update the control
    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) override;

    void setX0(int x0) override;
    void setY0(int y0) override;
    void setDepth(int depth) override;
    [[nodiscard]] int getMaxDepth() const override;
private:

    void updateChildrenLocation();
    int spacing_;
    orientation orientation_;
    std::vector<std::shared_ptr<control>> children_;
};


#endif //MIGUIDEMO_STACKCONTROL_H