//
// Created by nikolaj on 8/1/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_SHELL_H
#define WHIRLWINDSOFDANGERSKETCH_SHELL_H
#include <random>

#include "../glm/vec2.hpp"


///Shells are NOT simulated as actual projectiles, that would be way to processor intensive, instead shells
class Shell {
public:
    Shell(glm::dvec2 position, double speed, glm::dvec2 target, std::mt19937& rng,bool surface,bool isPlayer);

    bool isPlayer() const { return isPlayer_; }
    //Shells are too small to see, so they don't get a display function
    bool targetSurface() const {return targetSurface_;}
    void update(double dt);
    double getTimeToImpact() const {return timeToImpact_;}
    const glm::dvec2& getPosition() const {return position_;};
private:
    bool isPlayer_;
    bool targetSurface_;
    double timeToImpact_;
    glm::dvec2 position_=glm::dvec2(0);
    glm::dvec2 velocity_=glm::dvec2(0);
};


#endif //WHIRLWINDSOFDANGERSKETCH_SHELL_H