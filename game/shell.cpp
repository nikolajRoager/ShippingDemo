//
// Created by nikolaj on 8/1/26.
//

#include "shell.h"

#include "../glm/mat2x2.hpp"
#include "../glm/geometric.hpp"

void Shell::update(double dt) {
    //When we calculate impact we will back-track to when we actually hit the water
    if (timeToImpact_<0)
        return;
    if (timeToImpact_<dt) {
        position_ += velocity_*timeToImpact_;
    }
    else {
        position_ += velocity_ * dt;
    }
    timeToImpact_ -= dt;
}

Shell::Shell(glm::dvec2 position, double speed, glm::dvec2 target, std::mt19937& rng,bool surface,bool isPlayer) {
    isPlayer_ = isPlayer;
    targetSurface_ = surface;
    //.1% error, and 1 milli rad error is slightly larger than what is realistic, but otherwise you can't see it
    std::uniform_real_distribution<double> dist(-0.001, 0.001);
    glm::dvec2 D = target-position ;
    double distance = glm::length(D);
    double angle =  dist(rng);
    //Use expansion of sin and cos, that is ok for small deflections, and computationally faster
    glm::dmat2 rotationMatrix = glm::dmat2((1), angle,-angle, (1));
    D=rotationMatrix*D;
    timeToImpact_ = distance/speed;
    //Apply 1% error on all guns
    speed+=speed*dist(rng);
    velocity_ = D*speed/distance;
    position_ = position;
}

