//
// Created by nikolaj on 3/1/26.
//

#ifndef MIGUIDEMO_CONTENTTABLE_H
#define MIGUIDEMO_CONTENTTABLE_H
#include <iostream>
#include <set>
#include <vector>

#include "control.h"


class contentTable : public control {
public:
    explicit contentTable(const std::vector<std::shared_ptr<control>>& header, const std::vector<std::vector<std::shared_ptr<control>>>& rows, int spacing, Uint8 headerR=100, Uint8 headerG=100, Uint8 headerB=100, Uint8 scrollR=200, Uint8 scrollG=200, Uint8 scrollB=200, Uint8 selectedR=100, Uint8 selectedG=100, Uint8 selectedB=200, bool noSelecting=false);
    ~contentTable() override;

    void addRow(const std::vector<std::shared_ptr<control>>& row);

    ///Render the control
    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight, int depth, SDL_Rect& clip, Uint8 r, Uint8 g, Uint8 b, Uint8 a) override;

    ///Update the control
    void update(const InputData& userInputs, int screenWidth, int screenHeight, bool covered, SDL_Rect& clip) override;

    void setX0(int x0) override;
    void setY0(int y0) override;

    void setDepth(int depth) override;
    [[nodiscard]] int getMaxDepth() const override;

    [[nodiscard]] const std::set<size_t>& getSelectedRows() const {return selectedRows;}
    [[nodiscard]] size_t getMainSelectedRow() const {return mainSelectedRow_;}

    [[nodiscard]] bool getIsRightClicked() const {return isRightClicked;}

    void setAvailableWidth(int width) override {
        availableWidth = width;
        width_ = std::min(localColumnPosition_.back(),availableWidth);
        updateChildPosition();

    }
    void setAvailableHeight(int height) override {
        availableHeight = height;
        height_=std::min(localRowPosition_.back(),availableHeight);
        updateChildPosition();
    }

    void setSelection(size_t s) {
        selectedRows.clear();
        selectedRows.insert(s);
        mainSelectedRow_=s;
    }
    void dropSelection() {
        selectedRows={};
        mainSelectedRow_=-1;
    }
    void dropSelection(const std::set<size_t> &toDrop) {
        for (size_t i : toDrop) {
            if (selectedRows.contains(i)) selectedRows.erase(i);
        }
        if (toDrop.contains(mainSelectedRow_)) {
            mainSelectedRow_ = -1;
        }
    }

    void deleteRows() {
        selectedRows.clear();
        mainSelectedRow_ = -1;
        rows_.clear();
    }
private:
    bool noSelecting_ = false;

    void recalcCols();
    void recalcRows();

    void updateChildPosition();


    Uint8 headerR_, headerG_, headerB_;
    Uint8 selectedR_, selectedG_, selectedB_;
    Uint8 scrollR_, scrollG_, scrollB_;

    int spacing_;
    std::vector<std::shared_ptr<control>> header_;
    std::vector<std::vector<std::shared_ptr<control>>> rows_;

    ///Used for aligning the header and the rows
    std::vector <int> localRowPosition_, localColumnPosition_;

    std::set<size_t> selectedRows;
    size_t mainSelectedRow_ = -1;
    //Did the user right click inside the table
    bool isRightClicked;

    int availableWidth, availableHeight;

    double horizontalScrollFactor;
    double verticalScrollFactor;

    int scrollBarHandleSize;

    bool scrollingHorizontal;
    bool scrollingVertical;
};


#endif //MIGUIDEMO_CONTENTTABLE_H