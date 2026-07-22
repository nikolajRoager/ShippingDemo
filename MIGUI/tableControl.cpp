//
// Created by nikolaj on 2/27/26.
//

#include "tableControl.h"
#include "tableControl.h"

#include <iostream>

int tableControl::getMaxDepth() const {
    int max = depth_;
    for (const auto& child : children_) {
        max = std::max(max, child->getMaxDepth());
    }
    return max;
}


void tableControl::setDepth(int depth) {
    depth_ = depth;
    for (const auto& child : children_) {
        child->setDepth(depth+1);
    }
}

tableControl::tableControl(int availableWidth, int availableHeight, const std::vector<rowOrCol> &rowLayout, const std::vector<rowOrCol> &columnLayout, const std::vector<std::shared_ptr<control>>& children,const std::vector<background> &backgrounds) : control() {
    columnLayout_=columnLayout;
    rowLayout_=rowLayout;
    children_ = children;

    if (columnLayout_.empty())
        throw std::invalid_argument("Invalid number of columns");
    if (rowLayout_.empty())
        throw std::invalid_argument("Invalid number of rows");
    if (children_.size() != rowLayout.size()*columnLayout.size())
        throw std::invalid_argument("Number of child elements did not match table size "+std::to_string(children_.size())+" child elements != "+std::to_string(rowLayout.size()*columnLayout.size())+" table size");

    //If no background is provided, everything is transparent
    if (backgrounds.empty()) {
        backgrounds_.resize(rowLayout.size()*columnLayout.size(),background());
    }
    else {
        backgrounds_=backgrounds;
        if (backgrounds_.size() != rowLayout.size()*columnLayout.size()) {
            throw std::invalid_argument("Number of backgrounds did not match table size "+std::to_string(rowLayout.size()*columnLayout.size())+" !="+std::to_string(backgrounds.size()));
        }
    }

    localColumnPosition_.resize(columnLayout.size()+1);
    localRowPosition_.resize(rowLayout.size()+1);

    willExpandHeight_ = willExpandWidth_ = false;

    recalcRows(availableHeight);
    recalcCols(availableWidth);

}

void tableControl::update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) {
    hoverInherited_ =
        (userInputs.mouseXPx>x0_ && userInputs.mouseYPx>y0_ && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<height_+y0_);
    //Update children
    for (auto &child : children_) {
        hoverInherited_ = hoverInherited_ || child->getHoverInherited();
        child->update(userInputs,screenWidth,screenHeight, covered, clip);
    }

    //Update whether we need to resize something
    bool updateWidth= false;
    bool updateHeight= false;
    for (const auto& child : children_) {
        if (child->getHeightChanged())
            updateHeight = true;
        if (child->getWidthChanged())
            updateWidth = true;
    }
    if (updateWidth) {
        recalcCols(width_);
    }
    if (updateHeight) {
        recalcRows(height_);
    }
}


void tableControl::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {

    if (depth==depth_) {
        for (size_t col = 0; col < columnLayout_.size(); col++) {
            for (size_t row = 0; row < rowLayout_.size(); row++) {
                size_t i = col+row*columnLayout_.size();
                if (!backgrounds_[i].transparent_) {
                    SDL_SetRenderDrawColor(renderer, backgrounds_[i].R_, backgrounds_[i].G_, backgrounds_[i].B_, 255);
                    SDL_Rect bgr {x0_+localColumnPosition_[col], y0_+localRowPosition_[row], columnLayout_[col].size_,rowLayout_[row].size_};
                    SDL_RenderFillRect(renderer, &bgr);
                }
            }
        }
    }


    //Draw children
    for (const auto& child : children_) {
        child->render(renderer, screenWidth, screenHeight, depth, clip,r,g,b,a);
    }

    if (depth==depth_) {
        /*
        //Draw table lines
        for (int x : localColumnPosition_) {
            x+=x0_;
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

            SDL_RenderDrawLine(renderer, x, y0_, x, y0_+height_);
        }
        for (int y : localRowPosition_) {
            y+=y0_;
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

            SDL_RenderDrawLine(renderer, x0_, y, x0_+width_, y);
        }
        */
    }
}

void tableControl::setAvailableHeight(int availableHeight) {
    if (willExpandHeight_ && availableHeight!=height_) {
        recalcRows(availableHeight);
    }
}

void tableControl::recalcCols(int availableWidth) {
    int usedWidth = 0;
    int numberOfExpandWidth = 0;

    //We will loop through rows and columns twice, once to get the height and width we are definitely going to use, and once more to  actually place down the rows and columns
    for (size_t i = 0; i < columnLayout_.size(); i++) {
        auto& col = columnLayout_[i];
        if (col.sizeType_==rowOrCol::EXPAND) {
            numberOfExpandWidth++;
            willExpandWidth_ = true;
        }
        else if (col.sizeType_==rowOrCol::FIXED) {
            usedWidth+= col.size_;
        }
        else if (col.sizeType_==rowOrCol::SHRINK) {
            //Loop through all elements in this column and find the widest
            col.size_=0;
            for (size_t j = 0; j < rowLayout_.size(); j++) {
                size_t idx = i+j*columnLayout_.size();
                col.size_ = std::max(col.size_, children_[idx]->getWidth());
            }
            usedWidth+= col.size_;
        }
    }

    //width of the expandable elements
    int expandableWidth = 0;
    if (willExpandWidth_) {
        expandableWidth =  (availableWidth - usedWidth)/numberOfExpandWidth;
    }

    localColumnPosition_[0]=0;
    int currentWidth = 0;
    for (size_t i = 0; i < columnLayout_.size(); i++) {
        auto& col = columnLayout_[i];
        if (col.sizeType_==rowOrCol::EXPAND) {
            col.size_ = expandableWidth;
        }
        currentWidth += col.size_;
        localColumnPosition_[i+1] = currentWidth;
    }

    if (width_!=currentWidth)
        widthChanged_ = true;
    width_=currentWidth;
    updateChildPosition();
}

void tableControl::recalcRows(int availableHeight) {


    int usedHeight = 0;
    int numberOfExpandHeight = 0;

    //We will loop through rows and columns twice, once to get the height and width we are definitely going to use, and once more to  actually place down the rows and columns
    for (size_t j = 0; j < rowLayout_.size(); j++) {
        auto& row = rowLayout_[j];
        if (row.sizeType_==rowOrCol::EXPAND) {
            numberOfExpandHeight++;
            willExpandHeight_ = true;
        }
        else if (row.sizeType_==rowOrCol::FIXED) {
            usedHeight += row.size_;
        }
        else if (row.sizeType_==rowOrCol::SHRINK) {
            //Loop through all elements in this row and find the tallest
            row.size_=0;
            for (size_t i = 0; i < columnLayout_.size(); i++) {
                size_t idx = i+j*columnLayout_.size();
                row.size_ = std::max(row.size_, children_[idx]->getHeight());
            }
            usedHeight += row.size_;
        }
    }

    //height of the expandable elements
    int expandableHeight = 0;
    if (willExpandHeight_) {
        expandableHeight =  (availableHeight - usedHeight)/numberOfExpandHeight;
    }

    localRowPosition_[0]=0;
    int currentHeight = 0;
    for (size_t i = 0; i < rowLayout_.size(); i++) {
        auto& row = rowLayout_[i];
        if (row.sizeType_==rowOrCol::EXPAND) {
            row.size_ = expandableHeight;
        }
        currentHeight += row.size_;
        localRowPosition_[i+1] = currentHeight;
    }

    if (height_!=currentHeight)
        heightChanged_ = true;
    height_ = currentHeight;
    updateChildPosition();
}

void tableControl::setAvailableWidth(int availableWidth) {
    if (willExpandWidth_ && availableWidth!=width_) {
        recalcCols(availableWidth);
    }
}

void tableControl::setX0(int x0) {
    x0_ = x0;
    updateChildPosition();
}

void tableControl::setY0(int y0) {
    y0_ = y0;
    updateChildPosition();
}

void tableControl::updateChildPosition() {
    for (size_t col = 0; col < columnLayout_.size(); col++) {
        for (size_t row = 0; row < rowLayout_.size(); row++) {
            size_t i = col+row*columnLayout_.size();
            children_[i]->setX0(x0_+localColumnPosition_[col]);
            children_[i]->setY0(y0_+localRowPosition_[row]);
            children_[i]->setAvailableWidth(columnLayout_[col].size_);
            children_[i]->setAvailableHeight(rowLayout_[row].size_);
        }
    }
}