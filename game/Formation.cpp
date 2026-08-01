//
// Created by nikolaj on 7/19/26.
//

#include "Formation.h"

#include <algorithm>
#include <iostream>
#include <utility>

Formation::Formation(double desiredDistance, const std::vector<std::shared_ptr<Ship> > &ships, std::deque<glm::dvec2> waypoints, bool radarOn) {
    formationRadarOn_ = radarOn;
    desiredDistance_ = desiredDistance;
    ships_ = ships;
    waypoints_=std::move(waypoints);
    if (waypoints_.empty()) {
        formationSpeed_=Ship::STOP;
    }
    else {
        formationSpeed_=Ship::CRUISE;
    }
    updateRadar();
}

void Formation::render(SDL_Renderer *renderer, double mapTopLeftX, double mapTopLeftY, double scale) const {
    if (ships_.empty())
        return;
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

    glm::dvec2 pos = ships_[0]->getPosition();
    int prevTexX=static_cast<int>(mapTopLeftX + pos.x * scale);
    int prevTexY=static_cast<int>(mapTopLeftY + pos.y * scale);
    for (auto waypoint : waypoints_) {
        int texX = static_cast<int>(mapTopLeftX + waypoint.x * scale);
        int texY = static_cast<int>(mapTopLeftY + waypoint.y * scale);
        SDL_RenderDrawLine(renderer, texX-20, texY, texX+20, texY);
        SDL_RenderDrawLine(renderer, texX, texY-20, texX, texY+20);
        SDL_RenderDrawLine(renderer, prevTexX,prevTexY,texX,texY);

        prevTexX=texX;
        prevTexY=texY;
    }
}




double Formation::referenceSpeed() const {
    double minSpeed = std::numeric_limits<double>::max();
    for (auto& s : ships_) minSpeed = std::min(minSpeed, s->speedForEnum(formationSpeed_));
    return minSpeed;
}

void Formation::update() {
    removeSunkShips();
    if (ships_.empty()) return;

    auto& lead = ships_[0];



    while (!waypoints_.empty()) {
        //Set heading to waypoint
        glm::dvec2 direction = waypoints_.front()-lead->getPosition();
        double distance2 = glm::dot(direction, direction);
        if (distance2<desiredDistance_*desiredDistance_) {
            //We have arrived at a waypoint
            waypoints_.pop_front();
        }
        else {
            formationHeading_=std::atan2(direction.y, direction.x);
            //continue right along
            break;
        }
    }
    if (waypoints_.empty()) {
        formationSpeed_=Ship::STOP;
    }

    double baseSpeed = referenceSpeed();
    glm::dvec2 leadPosition = lead->getPosition();

    lead->setHeading(formationHeading_);
    lead->setSpeedExact(baseSpeed);

    const double Kp_speed = 0.05; // tune: how aggressively stragglers catch up

    for (size_t i = 1; i < ships_.size(); ++i) {
        auto& ship = ships_[i];
        glm::dvec2 toTarget = leadPosition - ship->getPosition();
        double dist = glm::length(toTarget);

        if (dist > desiredDistance_) {
            double speed = std::clamp(baseSpeed + Kp_speed * dist, 0.0, ship->getMaxSpeed());
            ship->setSpeedExact(speed);
            ship->setHeading(atan2(toTarget.y, toTarget.x));
        }
        else {
            ship->setHeading(formationHeading_);

            ship->setSpeedExact(baseSpeed);
        }

    }
}

void Formation::updateRadar() {
    if (ships_.empty()) return;
    if (formationRadarOn_) {
        //Only one ship should be emitting, and that should be the ship with the best radar
        double highestCoefficient=0;
        int tallestId=0;
        for (int i = 1; i < ships_.size(); ++i) {
            ships_[i]->setRadar(false);
            if (ships_[i]->getRadarCoefficient()>highestCoefficient) {
                highestCoefficient=ships_[i]->getRadarCoefficient();
                tallestId=i;
            }
        }
        ships_[tallestId]->setRadar(true);
    }
    else {
        //Tell everyone to go to EMCON
        for (auto& ship : ships_) {
            ship->setRadar(false);
        }
    }
}


void Formation::removeSunkShips() {
    size_t erased = std::erase_if(ships_,
                  [](const std::shared_ptr<Ship>& s){ return s->getHealth() <= 0; });
    //Check if the radar picket has been sunk
    if (erased!=0)
        updateRadar();
}

void Formation::setSpeed(Ship::Speed speed) {
    if (!waypoints_.empty())
        formationSpeed_ = speed;
    else
        formationSpeed_ = Ship::STOP;
}