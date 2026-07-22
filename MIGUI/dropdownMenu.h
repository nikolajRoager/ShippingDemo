//
// Created by nikolaj on 6/18/26.
//

#ifndef DUNGEONSKETCHTWO_DROPDOWNMENU_H
#define DUNGEONSKETCHTWO_DROPDOWNMENU_H
#include <vector>

#include "control.h"


class DropdownMenu : public control{
public:
    explicit DropdownMenu(const std::vector <std::shared_ptr<control>>& controls, std::shared_ptr<const TexWrap> expandButtonTexture, Uint8 RHover=150, Uint8 GHover=150, Uint8 BHover=150, Uint8 RSelect=100, Uint8 GSelect=100, Uint8 BSelect=200, Uint8 RBgr=50, Uint8 GBgr=50, Uint8 BBgr=50);
    ~DropdownMenu() override = default;

    ///Render the control
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;

    ///Update the control
    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) override;

    void setX0(int x0) override;
    void setY0(int y0) override;

    void setDepth(int depth) override;
    [[nodiscard]] int getMaxDepth() const override;

    [[nodiscard]] bool getChangedSelection() const {return changedSelection_;}
    [[nodiscard]] size_t getSelection() const {return selectedIndex_;}

    void setSelection(size_t selection) {selectedIndex_ = selection; recalculateExpansion();}
private:
    bool expandAbove_=false;

    int expandButtonTextureX_ = 0;

    void recalculateExpansion();
    int expanseHeight_= 0;
    int expanseWidth_= 0;

    bool hoverButton_ = false;
    bool expanded_ = false;

    bool changedSelection_ = false;
    size_t hoveredIndex_ = -1;
    std::vector <std::shared_ptr<control>> controls_;
    std::shared_ptr<const TexWrap> expandButtonTexture_;
    size_t selectedIndex_;

    Uint8 RHover_= 150;
    Uint8 GHover_= 150;
    Uint8 BHover_= 150;
    Uint8 RSelect_ = 100;
    Uint8 GSelect_ = 100;
    Uint8 BSelect_ = 200;
    Uint8 RBgr_ = 0;
    Uint8 GBgr_ = 0;
    Uint8 BBgr_ = 0;
};


#endif //DUNGEONSKETCHTWO_DROPDOWNMENU_H