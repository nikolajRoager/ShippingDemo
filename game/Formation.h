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
class Formation {
public:
    Formation(double desiredDistance, const std::vector<std::shared_ptr<Ship>>& ships, std::deque<glm::dvec2> waypoints, bool radarOn);

    void render(SDL_Renderer* renderer, double mapTopLeftX, double mapTopLeftY, double scale) const;

    void removeShip(int index) {
        ships_.erase(ships_.begin() + index);
    }
    void addShip(const std::shared_ptr<Ship>& ship) {
        ships_.push_back(ship);
    }

    [[nodiscard]] const std::deque<glm::dvec2>& getWaypoints() const {return waypoints_;}
    void overwriteWaypoint(glm::dvec2 waypoint) {
        waypoints_.clear();
        waypoints_.push_back(waypoint);
    }
    void addWaypoint(glm::dvec2 waypoint) {
        waypoints_.push_back(waypoint);
    }
    ///Set speed, speed command is an enum between STOP, SLOW, CRUISE, and FULL (formation uses the speed of the slowest ship)
    void setSpeed(Ship::Speed speed);


    [[nodiscard]] bool getFormationRadarOn() const {return formationRadarOn_;}
    void toggleRadar() {formationRadarOn_=!formationRadarOn_; updateRadar();}
    ///Gives ships new orders to stay in formation
    void update();

    [[nodiscard]] const std::vector<std::shared_ptr<Ship>>& getShips() const {return ships_;}

    [[nodiscard]] bool noWaypoints() const {return waypoints_.empty();}
    [[nodiscard]] Ship::Speed getSpeed() const {return formationSpeed_;}

private:
    ///Change radar policy, called whenever radar policy changes, or a ship leaves the formation
    void updateRadar();
    bool formationRadarOn_=false;
    double desiredDistance_;
    std::vector<std::shared_ptr<Ship>> ships_;
    double formationHeading_ = 0.0;
    Ship::Speed formationSpeed_ = Ship::Speed::CRUISE;
    std::deque<glm::dvec2> waypoints_;

    [[nodiscard]] double referenceSpeed() const;
    void removeSunkShips();
};


#endif //WHIRLWINDSOFDANGERSKETCH_LINEFORMATION_H