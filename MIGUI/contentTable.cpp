//
// Created by nikolaj on 3/1/26.
//

#include "contentTable.h"

#include <algorithm>
#include <iostream>

contentTable::contentTable(const std::vector<std::shared_ptr<control>>& header, const std::vector<std::vector<std::shared_ptr<control>>>& rows, int spacing, Uint8 headerR, Uint8 headerG, Uint8 headerB, Uint8 scrollR, Uint8 scrollG, Uint8 scrollB, Uint8 selectedR, Uint8 selectedG, Uint8 selectedB, bool noSelecting) : control() {
    noSelecting_ = noSelecting;
    spacing_=spacing;
    header_=header;
    rows_=rows;
    selectedR_=selectedR;
    selectedG_=selectedG;
    selectedB_=selectedB;
    scrollR_=scrollR;
    scrollG_=scrollG;
    scrollB_=scrollB;
    headerR_=headerR;
    headerG_=headerG;
    headerB_=headerB;
    isRightClicked=false;
    availableWidth=0;
    availableHeight=0;

    for (const auto& row : rows_) {
        if (row.size()!=header.size()) {
            throw std::runtime_error("Content table row size mismatch");
        }
    }

    localColumnPosition_.resize(header_.size()+1);

    recalcCols();
    recalcRows();

    horizontalScrollFactor=0;
    verticalScrollFactor=0;
    scrollBarHandleSize=64;

    scrollingHorizontal=false;
    scrollingVertical=false;
}

contentTable::~contentTable() = default;

int contentTable::getMaxDepth() const {
    int maxDepth = depth_;
    for (const auto& row : rows_) {
        for (const auto& ctrl: row) {
            maxDepth = std::max(maxDepth, ctrl->getMaxDepth());
        }
    }
    for (const auto& head : header_) {
        maxDepth = std::max(maxDepth, head->getMaxDepth());
    }
    return maxDepth;
}

void contentTable::setDepth(int depth) {
    depth_ = depth;
    for (auto& row : rows_) {
        for (auto& ctrl: row) {
            ctrl->setDepth(depth+1);
        }
    }
    for (auto& head : header_) {
        head->setDepth(depth+1);
    }
}

void contentTable::setX0(int x0) {
    x0_ = x0;
    updateChildPosition();
}

void contentTable::setY0(int y0) {
    y0_ = y0;
    updateChildPosition();
}

void contentTable::update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) {
    bool sizeChanged=false;
    for (const auto& row : rows_) {
        for (const auto& ctrl : row) {
            if (ctrl->getHeightChanged() || ctrl->getWidthChanged()) {
                sizeChanged=true;
            }
        }
    }
    if (sizeChanged) {
        widthChanged_=true;
        heightChanged_=true;
        recalcCols();
        recalcRows();
        updateChildPosition();
    }


    hoverInherited_ =
        (userInputs.mouseXPx>x0_ && userInputs.mouseYPx>y0_ && userInputs.mouseXPx<width_+x0_ && userInputs.mouseYPx<height_+y0_);
    //Our children may go outside the table, we still want to count as hovering if we are on them
    if (!hoverInherited_)
        for (const auto& head : header_) {
            hoverInherited_ = hoverInherited_ || head->getHoverInherited();
        }
    if (!hoverInherited_)
        for (const auto& row : rows_) {
            for (const auto& ctrl : row) {
                hoverInherited_ = hoverInherited_ || ctrl->getHoverInherited();
            }
        }

    int scrollYOffset = localRowPosition_.back()>=availableHeight? static_cast<int>(verticalScrollFactor*(localRowPosition_.back()-availableHeight)):0;

    SDL_Rect myClipRect = {x0_, y0_, width_, height_};
    SDL_Rect contentClipRect = {x0_, y0_+localRowPosition_[1], width_, height_-localRowPosition_[1]};

    if (!covered && (userInputs.leftMouseDown && !userInputs.prevLeftMouseDown) && !noSelecting_) {
        if (userInputs.mouseXPx>x0_ && userInputs.mouseYPx>y0_ && userInputs.mouseXPx<x0_+width_ && userInputs.mouseYPx<y0_+height_) {
            if (!userInputs.ctrlPressed && !userInputs.shiftPressed) {
                mainSelectedRow_=-1;
                selectedRows.clear();
            }
            for (size_t i = 0; i < rows_.size(); ++i) {
                if (userInputs.mouseYPx>y0_+localRowPosition_[i+1]-scrollYOffset   && userInputs.mouseYPx<y0_+localRowPosition_[i+2]-scrollYOffset  ) {
                    if (userInputs.ctrlPressed) {
                        if (selectedRows.contains(i)) {
                            if (mainSelectedRow_==i)
                                mainSelectedRow_=-1;
                            selectedRows.erase(i);
                        }
                        else {
                            selectedRows.insert(i);
                            mainSelectedRow_=i;
                        }
                    }
                    else {
                        selectedRows={i};
                        mainSelectedRow_=i;
                    }
                }
            }
        }
    }

    for (const auto& row : rows_) {
        for (const auto& ctrl : row) {
            ctrl->update(userInputs, screenWidth, screenHeight,covered,contentClipRect);
        }
    }
    for (const auto& head : header_) {
        head->update(userInputs, screenWidth, screenHeight,covered,myClipRect);
    }


    if (!userInputs.leftMouseDown) {
        scrollingHorizontal=false;
        scrollingVertical=false;
    }

    if (availableHeight<localRowPosition_.back()) {
        if (userInputs.scroll>0 ) {
            verticalScrollFactor-=0.1;
            verticalScrollFactor=std::ranges::clamp(verticalScrollFactor,0.0,1.0);
            updateChildPosition();
        }
        else if (userInputs.scroll<0 ) {
            verticalScrollFactor+=0.1;
            verticalScrollFactor=std::ranges::clamp(verticalScrollFactor,0.0,1.0);
            updateChildPosition();
        }
    }


    if (!covered) {
        //Mouse in horizontal scroll bar
        if (availableWidth<localColumnPosition_.back() && ((userInputs.leftMouseDown && userInputs.mouseYPx>y0_+availableHeight-16 && userInputs.mouseYPx<y0_+availableHeight) || scrollingHorizontal)) {
            int availableWidthMinusHandleSize = availableWidth-scrollBarHandleSize;
            horizontalScrollFactor=std::ranges::clamp(static_cast<double>(userInputs.mouseXPx-x0_-scrollBarHandleSize*0.5)/availableWidthMinusHandleSize,0.0,1.0);
            updateChildPosition();
            scrollingHorizontal=true;
        }
        //Mouse in vertical scroll bar
        if (availableHeight<localRowPosition_.back() && ((userInputs.leftMouseDown && userInputs.mouseXPx>x0_+availableWidth-16 && userInputs.mouseXPx<x0_+availableWidth) || scrollingVertical)) {
            int availableHeightMinusHandleSize = availableHeight-scrollBarHandleSize;
            verticalScrollFactor=std::ranges::clamp(static_cast<double>(userInputs.mouseYPx-y0_-scrollBarHandleSize*0.5)/availableHeightMinusHandleSize,0.0,1.0);
            updateChildPosition();
            scrollingVertical=true;
        }
    }

}

void contentTable::addRow(const std::vector<std::shared_ptr<control> > &row) {
    selectedRows={rows_.size()};
    mainSelectedRow_=rows_.size();
    rows_.push_back(row);
    widthChanged_=true;
    heightChanged_=true;
    recalcCols();
    recalcRows();
    updateChildPosition();
}


void contentTable::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& /*clip*/, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {

    SDL_Rect myClipRect = {x0_, y0_, width_, height_};
    SDL_Rect contentClipRect = {x0_, y0_+localRowPosition_[1], width_, height_-localRowPosition_[1]};

    if (depth==depth_)
    {
        int scrollYOffset = localRowPosition_.back()>=availableHeight? static_cast<int>(verticalScrollFactor*(localRowPosition_.back()-availableHeight)):0;

        //Draw header and selection markers
        SDL_SetRenderDrawColor(renderer, headerR_, headerG_, headerB_, 255);
        SDL_Rect bgr {x0_, y0_, width_ ,localRowPosition_[1]};
        SDL_RenderFillRect(renderer, &bgr);

        if (!noSelecting_) {
            SDL_SetRenderDrawColor(renderer, selectedR_, selectedG_, selectedB_, 255);
            for (size_t s : selectedRows) {
                int y =y0_+localRowPosition_[s+1]-scrollYOffset;
                int h =localRowPosition_[1];
                if (y<contentClipRect .y) {
                    if (y+h<contentClipRect.y)
                        continue;
                    h-=contentClipRect.y-y;
                    y=contentClipRect.y;
                }
                if (y+h>contentClipRect .y+contentClipRect.h) {
                    if (y>contentClipRect.y+contentClipRect.h)
                        continue;
                    h-=-contentClipRect.h-contentClipRect.y+y+h;
                }
                bgr ={x0_, y , width_ , h};
                SDL_RenderFillRect(renderer, &bgr);

            }
        }
    }
    //Draw content
    for (const auto& h : header_) {
        h->render(renderer, screenWidth, screenHeight, depth,myClipRect,r,g,b,a );
    }
    for (const auto& row : rows_) {
        for (const auto& element: row) {
            element->render(renderer, screenWidth, screenHeight, depth,contentClipRect,r,g,b,a);
        }
    }

    //Draw scroll bars
    if (depth==depth_) {
        if (availableWidth<localColumnPosition_.back()) {
            int thisX0=x0_, thisY0=y0_+availableHeight-16;
            SDL_Rect scrollBarHorizontal {thisX0,thisY0,availableWidth,16};
            SDL_SetRenderDrawColor(renderer, headerR_, headerG_, headerB_, 255);
            SDL_RenderFillRect(renderer, &scrollBarHorizontal);

            int availableWidthMinusHandleSize = availableWidth-scrollBarHandleSize;
            int scrollBarHandleX0 =static_cast<int>(availableWidthMinusHandleSize*horizontalScrollFactor)+x0_;
            SDL_Rect scrollBarHandle {scrollBarHandleX0,thisY0,scrollBarHandleSize,16};
            SDL_SetRenderDrawColor(renderer, scrollR_, scrollB_, scrollG_, 255);
            SDL_RenderFillRect(renderer, &scrollBarHandle);
        }

        if (availableHeight<localRowPosition_.back()) {
            int thisX0=x0_+availableWidth-16, thisY0=y0_;
            SDL_Rect scrollBarVertical {thisX0,thisY0,16,availableHeight};
            SDL_SetRenderDrawColor(renderer, headerR_, headerG_, headerB_, 255);
            SDL_RenderFillRect(renderer, &scrollBarVertical );

            int availableHeightMinusHandleSize = availableHeight-scrollBarHandleSize;
            int scrollBarHandleY0 =static_cast<int>(availableHeightMinusHandleSize*verticalScrollFactor)+y0_;
            SDL_Rect scrollBarHandle {thisX0,scrollBarHandleY0,16,scrollBarHandleSize};
            SDL_SetRenderDrawColor(renderer, scrollR_, scrollB_, scrollG_, 255);
            SDL_RenderFillRect(renderer, &scrollBarHandle);
        }
    }
}

void contentTable::recalcCols() {
    localColumnPosition_[0]=0;
    //We shrink to fit all columns
    int step = 0;
    for (size_t i = 0; i < header_.size(); ++i) {
        int width = header_[i]->getWidth();
        for (auto & row : rows_) {
            width = std::max(row[i]->getWidth(), width);
        }
        step += width+spacing_;
        localColumnPosition_[i+1] = step;
    }
    width_=localColumnPosition_.back();
    widthChanged_ = true;
}

void contentTable::recalcRows() {
    localRowPosition_.resize(rows_.size()+2);
    //We shrink to fit all rows
    //First the header
    int height = 0;
    for (const auto& h : header_) {
        height = std::max(height, h->getHeight());
    }
    localRowPosition_[0] = 0;
    localRowPosition_[1] = height;

    int step = height;

    for (size_t i = 0 ; i < rows_.size(); i++) {
        height=0;
        for (const auto& element : rows_[i]) {
            height = std::max(height, element->getMaxDepth());
        }
        step += height+spacing_;
        localRowPosition_[i+2] = step;
    }
    updateChildPosition();
    height_=localRowPosition_.back();
    heightChanged_ = true;
}

void contentTable::updateChildPosition() {
    int scrollXOffset = localColumnPosition_.back()>=availableWidth? static_cast<int>(horizontalScrollFactor*(localColumnPosition_.back()-availableWidth)):0;
    int scrollYOffset = localRowPosition_.back()>=availableHeight? static_cast<int>(verticalScrollFactor*(localRowPosition_.back()-availableHeight)):0;
    for (size_t i = 0; i < header_.size(); i++) {
        header_[i]->setX0(x0_+localColumnPosition_[i]-scrollXOffset );
        header_[i]->setY0(y0_);
    }
    for (size_t j = 0; j < rows_.size(); j++) {
        for (size_t i = 0; i < rows_[j].size(); i++) {
            rows_[j][i]->setX0(x0_+localColumnPosition_[i]-scrollXOffset );
            rows_[j][i]->setY0(y0_+localRowPosition_[j+1]-scrollYOffset );
        }
    }
}


