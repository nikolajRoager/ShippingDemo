//
// Created by nikolaj on 7/27/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_DRAWCIRCLE_H
#define WHIRLWINDSOFDANGERSKETCH_DRAWCIRCLE_H
#include <vector>
#include <SDL2/SDL_render.h>

struct CircleRecord {
    int x_, y_;
    double radius_;
    double radius2_;

    CircleRecord(int x, int y, double radius) {
        this->x_ = x;
        this->y_ = y;
        this->radius_ = radius;
        radius2_ = radius*radius;
    }
    CircleRecord() {
        x_ = y_ = 0;
        radius_ = 0;
        radius2_ = 0;
    }
};

void drawCircle(SDL_Renderer* renderer,int i, const std::vector<CircleRecord>& circles, Uint8 R, Uint8 G, Uint8 B, double dashLength=10, double spaceLength=5);

#endif //WHIRLWINDSOFDANGERSKETCH_DRAWCIRCLE_H