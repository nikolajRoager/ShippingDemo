//
// Created by nikolaj on 3/14/26.
//

#ifndef MIGUIDEMO_TABLEMENU_H
#define MIGUIDEMO_TABLEMENU_H
#include <map>
#include <vector>

#include "buttonControl.h"
#include "control.h"

class tableMenu : public control {
public:
    tableMenu(int availableWidth, int availableHeight,const std::vector<std::string>& items,SDL_Renderer *renderer,TTF_Font *font);
    ~tableMenu() override = default;

    ///Render the control
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;
    ///Update the control
    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) override;

    void setX0(int x0) override;
    void setY0(int y0) override;

    void setDepth(int depth) override;
    [[nodiscard]] int getMaxDepth() const override;

    [[nodiscard]] bool clickedMenu(const std::string& key) const {
        if (children_.contains(key)) {
            return children_.at(key)->isClicked();
        }
        return false;
    }
private:
    std::shared_ptr<control> table_;
    std::map<std::string,std::shared_ptr<buttonControl>> children_;
};


#endif //MIGUIDEMO_TABLEMENU_H