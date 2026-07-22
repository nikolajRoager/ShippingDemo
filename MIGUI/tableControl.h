//
// Created by nikolaj on 2/27/26.
//

#ifndef MIGUIDEMO_TABLE_H
#define MIGUIDEMO_TABLE_H
#include <vector>

#include "control.h"


///A table
class tableControl : public control {
public:
    ///Helper class which stores the layout of a row or column
    struct rowOrCol {
        ///Determines what size the row or column has
        enum SIZE_TYPE {
            ///Stays a fixed size
            FIXED = 0,
            ///Shrinks to fit content
            SHRINK= 1,
            ///Expand to fit available width/height
            EXPAND = 2,
        } sizeType_;

        ///If we have fixed size, what is said size; if we use expand or shrink size this will be auto-calculated in the tableControl constructor
        int size_;

        rowOrCol(const SIZE_TYPE sizeType, int size) : sizeType_(sizeType), size_(size) {}
    };

    struct background {
        bool transparent_;
        Uint8 R_,G_,B_;

        background(){transparent_=true;R_=G_=B_=0;}
        background(Uint8 R,Uint8 G,Uint8 B) {
            transparent_=false;
            R_ = R;
            G_ = G;
            B_ = B;
        }
    };

    ///Constructor
    ///@param availableWidth if you know the width, we can set up the table in the constructor (if not it will be updated later)
    ///@param availableHeight if you know the height, we can set up the table in the constructor (if not it will be updated later)
    ///@param rowLayout Layout of all rows
    ///@param columnLayout Layout of all columns
    ///@param children child elements , the index of a particular control is (column+row*columnLayout_.size())
    tableControl(int availableWidth, int availableHeight,const std::vector<rowOrCol>& rowLayout, const std::vector<rowOrCol>& columnLayout, const std::vector<std::shared_ptr<control>> &children, const std::vector<background> &backgrounds={});

    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) override;
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;

    void setAvailableHeight(int availableHeight) override;
    void setAvailableWidth(int availableWidth) override;

    void setX0(int x0) override;
    void setY0(int y0) override;

    void setDepth(int depth) override;
    [[nodiscard]] int getMaxDepth() const override;
private:

    void recalcCols(int availableWidth);
    void recalcRows(int availableHeight);

    void updateChildPosition();

    ///Layout of the table
    std::vector <rowOrCol> rowLayout_, columnLayout_;
    std::vector <int> localRowPosition_, localColumnPosition_;

    ///Child controls, the index of a particular control is (column+row*columnLayout_.size())
    std::vector <std::shared_ptr<control>> children_;

    std::vector<background> backgrounds_;

    ///Will this table auto expand to fill available width/height
    bool willExpandWidth_ =false ,willExpandHeight_ = false;
};


#endif //MIGUIDEMO_TABLE_H