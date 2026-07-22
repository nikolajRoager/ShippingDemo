//
// Created by nikolaj on 7/19/26.
//

#include "particle.h"

#include <algorithm>

Particle::Particle(const ParticleTemplate &template_, glm::dvec2 position, glm::dvec2 velocity):
position_(position), velocity_(velocity){
    texture_= template_.texture_;
    lifeTime_ = template_.lifeTime_;
    maxLifeTime_ = template_.lifeTime_;
    scale_ = template_.scale_;
    invMaxLifeTime_ = 1.0/maxLifeTime_;
    frames_ = template_.frames_;
}

void Particle::update(double dt) {
    //Euler update is fine for particles, they have no effect on gameplay
    position_ += velocity_*dt;
    //Particles experience friction
    velocity_-= velocity_*0.1;
    lifeTime_ -= dt;
}

void Particle::render(SDL_Renderer* renderer, int mapTopLeftX_, int mapTopLeftY_, int screenWidth, int screenHeight, double scale) const {
    int texX = position_.x*scale+mapTopLeftX_;
    int texY = position_.y*scale+mapTopLeftY_;

    double thisScale =scale*scale_;
    if (texX<0 || texY<0 || texX>screenWidth || texY>screenWidth || (thisScale*texture_->getHeight()<4)) {
        return;
    }

    int frame = std::clamp( static_cast<int>(frames_*(maxLifeTime_-lifeTime_)*invMaxLifeTime_),0,frames_-1);

    texture_->render(texX,texY,renderer,scale*scale_,true,true,false,frames_,frame);
}



