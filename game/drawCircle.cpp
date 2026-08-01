//
// Created by nikolaj on 7/27/26.
//

#include "drawCircle.h"

void drawCircle(SDL_Renderer* renderer,int i, const std::vector<CircleRecord>& circles, Uint8 R, Uint8 G, Uint8 B, double dashLength, double spaceLength){
    const CircleRecord& record = circles[i];
    //Radius will be set to exactly 0 if radar is off, there is no opportunity for floating point errors to accumulate between then and now, so this is safe
    if (record.radius_<=0) return;
    SDL_SetRenderDrawColor(renderer,R,G,B,255);
    double dThetaDash = dashLength/(record.radius_);
    double dThetaSpace = spaceLength/(record.radius_);
    double theta=0;
    while (theta<2*M_PI) {
        int x0 =static_cast<int>(record.x_+std::cos(theta)*record.radius_);
        int y0 =static_cast<int>(record.y_+std::sin(theta)*record.radius_);
        //Reject circles which overlap other circles
        bool skip=false;
        for (int  j = 0;  j < circles.size(); ++j) {
            //OTHER circles
            if (j!=i) {
                double dx = circles[j].x_-x0;
                double dy = circles[j].y_-y0;
                if (dx*dx+dy*dy<circles[j].radius2_) {
                    skip=true;
                    break;
                }
            }
        }
        if (!skip) {
            theta+=dThetaDash;
            int x1 =static_cast<int>(record.x_+std::cos(theta)*record.radius_);
            int y1 =static_cast<int>(record.y_+std::sin(theta)*record.radius_);
            SDL_RenderDrawLine(renderer,x0,y0,x1,y1);
            theta+=dThetaSpace;
        }
        else
        {
            theta+=dThetaDash+dThetaSpace;
        }
    }
}