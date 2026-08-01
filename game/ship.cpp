//
// Created by nikolaj on 7/18/26.
//

#include "ship.h"

#include <algorithm>
#include <iostream>
#include <utility>

Ship::Ship(const std::string &className,Side side,NATOSymbolManager::ShipType type, const std::vector<Gun>& guns, int health,double maxSpeed,double cruiseSpeed,double mass, double length, double height,glm::dvec2 position, double heading,bool transponder,double radarCoefficient,std::shared_ptr<const TexWrap> texture,std::shared_ptr<const TexWrap> cardTexture,std::shared_ptr<const TexWrap> targetTexture,std::shared_ptr<const TexWrap> velocityTexture,SDL_Renderer *renderer,TTF_Font* smallFont):
currentState_(position,heading,glm::dvec2(cos(heading)*cruiseSpeed,sin(heading)*cruiseSpeed),0)
    {
    guns_ = guns;
    radarCoefficient_=radarCoefficient;
    type_=type;
    transponderOn_ = transponder;
    className_ = className;
    side_ = side;
    health_ = health;
    maxHealth_ = health;
    maxSpeed_ = maxSpeed;
    cruiseSpeed_ = cruiseSpeed;
    mass_ = mass;
    length_ = length;
    height_ = height;
    sqrtHeight_=std::sqrt(height);
    double crossSection = length_*height_;
    radarCrossSectionFourthRoot_ = std::sqrt(std::sqrt(crossSection));
    desiredHeading_= heading;

    texture_ = std::move(texture);
    cardTexture_ = std::move(cardTexture);
    targetTexture_ = std::move(targetTexture);
    velocityTexture_ = std::move(velocityTexture);

    nameTexture_=std::make_shared<TexWrap>(className,renderer,smallFont);

    //Moment of inertia of a rod, maybe not the best approximation, but an approximation nevertheless
    I_ = mass_*length_*length_*0.5;
    //Some random guesses which feels right to me
    //The friction is proportional to an area, so lets just multiply by length^2
    frontalFrictionC_ = 0.1*length_*length_;
    backwardFrictionC_ = 0.3*length_*length_;
    sideFrictionC_ = 2.0*length_*length_;
    //I don't know why, but this just has to be massive to stop the ship from rotating around itself
    angularFrictionC_ = 1000000*length_*length_;
    //Tune this to modify ship turning circle
    rudderC_ = 1.0*length_*length_;

    rudderAuthority_=1.0;
    //Higher rudder damping makes the ship more sluggish, but also dampens oscillations making for a smoother ride
    rudderDamping_ = 50*std::sqrt(rudderAuthority_);

    identified_ = false;

    //Max speed is achieved when max thrust is equal to drag.
    maxThrust_=frontalFrictionC_*maxSpeed_*maxSpeed_;
    cruiseThrust_=frontalFrictionC_*cruiseSpeed_*cruiseSpeed_;
    slowThrust_=frontalFrictionC_*cruiseSpeed_*cruiseSpeed_*0.25;
    currentThrust_=cruiseThrust_;
}

// Wrap to [-pi, pi]
static double wrapAngle(double a) {
    a = std::fmod(a + M_PI, 2.0 * M_PI);
    if (a < 0) a += 2.0 * M_PI;
    return a - M_PI;
}


void Ship::render(SDL_Renderer *renderer, double mapTopLeftX, double mapTopLeftY, double scale) const {
    if (!identified_)
        return;
    int screenX =static_cast<int>(currentState_.position_.x*scale+mapTopLeftX);
    int screenY =static_cast<int>(currentState_.position_.y*scale+mapTopLeftY);

    double screenAngle = M_PI*0.5+currentState_.heading_;
    double targetScreenAngle = M_PI*0.5+desiredHeading_;

    texture_->render(screenX,screenY,renderer,scale,true,true,false,1,0,screenAngle);

    if (side_==FRIEND) {
        double velocityDirection = std::atan2(currentState_.velocity_.y,currentState_.velocity_.x);
        double velocityScreenAngle = M_PI*0.5+velocityDirection;
        velocityTexture_->render(screenX,screenY,renderer,1.0,true,true,false,1,0,velocityScreenAngle);
        targetTexture_->render(screenX,screenY,renderer,1.0,true,true,false,1,0,targetScreenAngle);
    }

}

// Linear blend of two states (needed by RK4)
static Ship::ShipState blend(const Ship::ShipState& a, const Ship::ShipState& d, double h)
{
    return {
        a.position_+ h * d.position_,
        a.heading_  + h * d.heading_,
        a.velocity_ + h * d.velocity_,
        a.omega_    + h * d.omega_
    };
}

///Sum states using RK4 method to get final state
static Ship::ShipState weightedRK4Sum(
    const Ship::ShipState& k1, const Ship::ShipState& k2,
    const Ship::ShipState& k3, const Ship::ShipState& k4)
{
    return Ship::ShipState {
        (k1.position_ + 2.0*k2.position_ + 2.0*k3.position_ + k4.position_) / 6.0,
        (k1.heading_  + 2.0*k2.heading_  + 2.0*k3.heading_  + k4.heading_ ) / 6.0,
        (k1.velocity_ + 2.0*k2.velocity_ + 2.0*k3.velocity_ + k4.velocity_) / 6.0,
        (k1.omega_    + 2.0*k2.omega_    + 2.0*k3.omega_    + k4.omega_   ) / 6.0
    };
}



void Ship::update(double dt) {
    //DOES NOT NEED TO CHECK SHIP HEALTH, even sunk ships can move

    //Calculate number of substeps to use
    const double maxSubStep = 0.05; // smaller = safer but more CPU
    int steps = std::max(1, (int)std::ceil(dt / maxSubStep));
    double h = dt / steps;

    for (int i = 0; i < steps; ++i) {
        // --- RK4  ---
        ShipState k1 = computeDerivative(currentState_);
        ShipState k2 = computeDerivative(blend(currentState_, k1, h/2));
        ShipState k3 = computeDerivative(blend(currentState_, k2, h/2));
        ShipState k4 = computeDerivative(blend(currentState_, k3, h));
        ShipState netDerivative = weightedRK4Sum(k1, k2, k3, k4);
        currentState_ = blend(currentState_, netDerivative, h);
    }
}

Ship::ShipState Ship::computeDerivative(const ShipState &s) const {
    ShipState d(glm::dvec2(0,0),0,glm::dvec2(0,0),0);

    //Get forward and sideways vectors, and decompose velocity
    glm::dvec2 forward(cos(s.heading_),sin(s.heading_));
    glm::dvec2 right(cos(s.heading_-M_PI*0.5),sin(s.heading_-M_PI*0.5));

    double vFwd  = glm::dot(s.velocity_, forward);// signed, m/s
    double vSide = glm::dot(s.velocity_, right);// signed, m/s

    //Get friction force and torque
    double fricFwd  = (vFwd >= 0.0 ? frontalFrictionC_ : backwardFrictionC_);
    //Quadratic drag, opposing motion, decomposed by axis
    glm::dvec2 F_friction =
        - fricFwd  * std::abs(vFwd)  * vFwd  * forward
        - sideFrictionC_ * std::abs(vSide) * vSide * right;

    //Quadratic rotational drag
    double frictionTorque = -angularFrictionC_ * s.omega_ * std::abs(s.omega_);

    //Thrust from engine
    glm::dvec2 F_thrust =forward*currentThrust_;


    //Get rudder torque
    double headingError = wrapAngle(desiredHeading_ - s.heading_);

    // Kp: how hard we react to heading error
    // Kd: how hard we react to existing turn rate (this is what kills the oscillation)
    double rudderCommand = rudderAuthority_ * headingError - rudderDamping_ * s.omega_;
    rudderCommand = std::clamp(rudderCommand, -1.0, 1.0); // max rudder deflection

    double rudderForce = -rudderC_ * vFwd * rudderCommand;

    // Torque = force × moment arm (half ship length)
    // Positive rudderForce pushes stern to right → bow turns right → positive omega
    double rudderTorque = -rudderForce * (length_ * 0.5);

    // The rudder force also pushes the ship sideways (reaction at stern)
    // It acts opposite to the torque direction at the stern position
    glm::dvec2 F_rudder = rudderForce * right;

    //Sum up all forces and torques
    glm::dvec2 F_total = F_friction+F_thrust+F_rudder;

    double totalTorque = frictionTorque +rudderTorque;

    //Get final derivatives
    d.position_ = s.velocity_;
    d.heading_  = s.omega_;
    d.velocity_ = F_total/mass_;
    d.omega_ = totalTorque/I_;

    return d;
}

void Ship::sailTowards(glm::dvec2 point) {
    auto direction = point-currentState_.position_;
    //The minus corrects the fact that the screen actually is a left-handed coordinate system
    desiredHeading_ = std::atan2(direction.y, direction.x);
}

void Ship::setSpeed(Speed newSpeed) {
    if (health_<=0) {
        currentThrust_=0;
        return;
    }
    switch (newSpeed) {
        default:
        case STOP:
            currentThrust_=0;
            break;
        case SLOW:
            currentThrust_=slowThrust_;
            break;
        case CRUISE:
            currentThrust_=cruiseThrust_;
            break;
        case FULL:
            currentThrust_=maxThrust_;
            break;
    }
}
