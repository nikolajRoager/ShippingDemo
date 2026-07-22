//
// Created by nikolaj on 7/19/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_LINEFORMATION_H
#define WHIRLWINDSOFDANGERSKETCH_LINEFORMATION_H
#include <deque>
#include <memory>
#include <vector>

#include "ship.h"

///A class which manages the formation and station keeping of ship
///A line is not a particularly realistic formation ... but it is easy to program
class LooseFormation {
public:
    LooseFormation(double desiredDistance, const std::vector<std::shared_ptr<Ship>>& ships, std::deque<glm::dvec2> waypoints);

    void render(SDL_Renderer* renderer, double mapTopLeftX, double mapTopLeftY, double scale) const;

    void overwriteWaypoint(glm::dvec2 waypoint) {
        waypoints_.clear();
        waypoints_.push_back(waypoint);
    }
    void addWaypoint(glm::dvec2 waypoint) {
        waypoints_.push_back(waypoint);
    }
    ///Set speed, speed command is an enum between STOP, SLOW, CRUISE, and FULL (formation uses the speed of the slowest ship)
    void setSpeed(Ship::Speed speed);
    ///Gives ships new orders to stay in formation
    void update();

    bool noWaypoints() const {return waypoints_.empty();}

private:

    double desiredDistance_;
    std::vector<std::shared_ptr<Ship>> ships_;
    double formationHeading_ = 0.0;
    Ship::Speed formationSpeed_ = Ship::Speed::CRUISE;
    std::deque<glm::dvec2> waypoints_;

    [[nodiscard]] double referenceSpeed() const;
    void removeSunkShips();
};


#endif //WHIRLWINDSOFDANGERSKETCH_LINEFORMATION_H