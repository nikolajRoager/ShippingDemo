//
// Created by nikolaj on 7/18/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_SHIP_H
#define WHIRLWINDSOFDANGERSKETCH_SHIP_H

#include <memory>

#include "natoSymbolManager.h"
#include "../TexWrap.h"
#include "../glm/glm.hpp"

class Ship {
public:

    struct Gun {
        double location_;
        double omega_;
        double angle_;
        double restAngle_;
        double cooldown_;
        double reloadTime_;
        double seaRange_;
        double airRange_;
        double muzzleVelocity_;

        Gun(double location, double omega, double restAngle, double reloadTime, double seaRange, double airRange, double muzzleVelocity) {
            location_ = location;
            omega_ = omega;
            angle_ = restAngle;
            restAngle_ = restAngle;
            reloadTime_ = reloadTime;
            seaRange_ = seaRange;
            airRange_ = airRange;
            muzzleVelocity_ = muzzleVelocity;
            cooldown_ = 0;
        }
    };

    enum Side {FRIEND=0,FOE=1,NEUTRAL=2};
    //Note, FLANK speed is purely an american term, not used in most European navies
    enum Speed {STOP=0,SLOW=1,CRUISE=2,FULL=3};

    void setSpeed(Speed newSpeed);
    void setSpeedExact(double newSpeed) {
        currentThrust_=std::min(maxThrust_,frontalFrictionC_*newSpeed*newSpeed);
        if (newSpeed<0) currentThrust_=-currentThrust_;
    }

    [[nodiscard]] double speedForEnum(Ship::Speed s) const {
        switch (s) {
            case Ship::Speed::STOP:   return 0.0;
            case Ship::Speed::SLOW:   return cruiseSpeed_ * 0.5;
            case Ship::Speed::CRUISE: return cruiseSpeed_;
            case Ship::Speed::FULL:   return maxSpeed_;
        }
        return 0.0;
    }

    void destroy() {health_=0; currentThrust_=0.0;}
    //Instantly stops the ship, should only be used when a ship runs aground
    void instantStop () {currentThrust_=0.0; currentState_.velocity_=glm::vec2(0.0);}

    Ship(const std::string &className,Side side,NATOSymbolManager::ShipType type, const std::vector<Gun>& guns, int health,double maxSpeed,double cruiseSpeed,double mass, double length, double height,glm::dvec2 position, double heading,bool transponder,double radarCoefficient,std::shared_ptr<const TexWrap> texture,std::shared_ptr<const TexWrap> cardTexture,std::shared_ptr<const TexWrap> targetTexture,std::shared_ptr<const TexWrap> velocityTexture,SDL_Renderer *renderer,TTF_Font* smallFont);

    [[nodiscard]] NATOSymbolManager::ShipType getType() const {return type_;}

    [[nodiscard]] double getHeight() const {return height_;}
    [[nodiscard]] double getSqrtHeight() const {return sqrtHeight_;}

    [[nodiscard]] double getRadarCrossSectionFourthRoot() const {return radarCrossSectionFourthRoot_;}

    [[nodiscard]] bool transponderOn() const {return transponderOn_;}
    [[nodiscard]] bool radarOn() const {return radarOn_;}
    void setRadar(bool radarOn) { radarOn_=radarOn;}

    [[nodiscard]] const std::shared_ptr<const TexWrap>& getNameTexture() const {return nameTexture_;}

    void render(SDL_Renderer *renderer, double mapTopLeftX, double mapTopLeftY, double scale) const;

    void update(double dt);

    void sailTowards(glm::dvec2 point);
    void setHeading(double newHeading) {
        desiredHeading_=newHeading;
    }

    [[nodiscard]] glm::dvec2 getPosition() const {return currentState_.position_;};
    [[nodiscard]] double getSpeed() const {return glm::length(currentState_.velocity_);}


    ///Used to store current state or derivatives
    struct ShipState {
        glm::dvec2 position_;//m if state or m/s if derivative
        double heading_;//rad if state rad/s if derivative
        glm::dvec2 velocity_;//m/s if state m/s^2 if derivative
        double omega_;//rad/s if state rad/s^2 if derivative

        ShipState(glm::dvec2 position, double heading, glm::dvec2 velocity, double omega):position_(position),heading_(heading),velocity_(velocity),omega_(omega){}
    };

    [[nodiscard]] double getMaxSpeed() const {return maxSpeed_;};
    [[nodiscard]] double getCruiseSpeed() const {return cruiseSpeed_;};
    [[nodiscard]] double getDesiredHeading() const {return desiredHeading_;};
    [[nodiscard]] double getHeading() const {return currentState_.heading_;};
    [[nodiscard]] glm::dvec2 getVelocity() const {return currentState_.velocity_;};

    [[nodiscard]] int getHealth() const {return health_;}
    [[nodiscard]] int getMaxHealth() const {return maxHealth_;};
    [[nodiscard]] double getLength() const {return length_;};

    [[nodiscard]] bool identified() const {return identified_;};

    void setIdentified(bool newId) {
        identified_ = newId;
    }
    [[nodiscard]] double getRadarCoefficient() const {return radarCoefficient_;};
    [[nodiscard]] const std::shared_ptr<const TexWrap>& getCardTexture() const {return cardTexture_;};

    [[nodiscard]] double getGunSeaRange() const {
        double out=0;
        for (const auto &gun : guns_)
            out = std::max(out,gun.seaRange_);
        return out;
    }
    [[nodiscard]] double getGunAirRange() const {
        double out=0;
        for (const auto &gun : guns_)
            out = std::max(out,gun.airRange_);
        return out;
    }
private:

    std::vector<Gun> guns_;

    bool transponderOn_=false;
    bool radarOn_=false;
    ///How far away, in meters, can we detect a 1m^2 object (ignoring the radar horizon)
    double radarCoefficient_=0.0;
    int maxHealth_=6;
    int health_ = 6;
    ///Gets the derivative of the state of the ship, used by stepper
    [[nodiscard]] ShipState computeDerivative(const ShipState& S) const;

    //Stats
    ///Name of the class of ship
    std::string className_;
    ///What side is this thing on
    Side side_;
    ///What type is this, effects chosen symbol
    NATOSymbolManager::ShipType type_;
    ///Max speed, in m/s
    double maxSpeed_;
    ///Max speed, in m/s
    double cruiseSpeed_;
    ///Friction coefficient for forward/backwards component when travelling forward (very low)
    double frontalFrictionC_;
    ///Friction coefficient for forward/backwards component when reversing (is higher than forward)
    double backwardFrictionC_;
    ///Friction coefficient for sideways travel component (is very high)
    double sideFrictionC_;
    ///Rotational friction coefficient
    double angularFrictionC_;
    ///Rudder coefficient
    double rudderC_;
    ///Derived from max speed and friction coefficient, max engine power in N
    double maxThrust_;
    ///Pre-calculated thrust for cruise and slow speed
    double cruiseThrust_,slowThrust_;
    ///Mass of the ship in kg
    double mass_;
    ///Length of the ship in m
    double length_;
    ///Height in meters, used to calculate radar cross-section
    double height_;
    ///Square root of the height, used in radar calculations
    double sqrtHeight_;
    /// radar cross-section to the 1/4 power, used in radar calculations
    double radarCrossSectionFourthRoot_;
    ///Moment of inertia in m^2 kg
    double I_;


    /// Kp: how hard we react to heading error
    double rudderAuthority_=1.0;
    /// Kd: how hard we react to existing turn rate (this is what kills the oscillation)
    double rudderDamping_=0.0;

    ///Current physics data
    ShipState currentState_;

    ///Current thrust from the engine right now in N
    double currentThrust_;

    ///What direction are we supposed to go in, in radians
    double desiredHeading_;

    ///Have we been spotted by the players intel manager
    bool identified_;

    //Textures
    std::shared_ptr<const TexWrap> texture_;
    std::shared_ptr<const TexWrap> cardTexture_;
    std::shared_ptr<const TexWrap> headingTexture_;
    std::shared_ptr<const TexWrap> targetTexture_;
    std::shared_ptr<const TexWrap> velocityTexture_;
    std::shared_ptr<const TexWrap> nameTexture_;
};


#endif //WHIRLWINDSOFDANGERSKETCH_SHIP_H