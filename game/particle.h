//
// Created by nikolaj on 7/19/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_PARTICLE_H
#define WHIRLWINDSOFDANGERSKETCH_PARTICLE_H
#include <memory>
#include <utility>
#include <SDL2/SDL_render.h>

#include "../TexWrap.h"
#include "../glm/vec2.hpp"

struct ParticleTemplate {
    std::shared_ptr<const TexWrap> texture_=nullptr;
    int frames_=0;
    double lifeTime_ = 0;
    double scale_ = 1;

    ParticleTemplate()=default;
};

class Particle {
public:
    void render(SDL_Renderer* renderer, int mapTopLeftX_, int mapTopLeftY_, int screenWidth, int screenHeight, double scale) const;
    void update(double dt);
    [[nodiscard]] bool isDead() const {return lifeTime_ < 0;}
    Particle(const ParticleTemplate& template_,glm::dvec2 position, glm::dvec2 velocity);
private:
    std::shared_ptr<const TexWrap> texture_;
    int frames_;
    double lifeTime_ = 0;
    double maxLifeTime_ = 0;
    double invMaxLifeTime_ = 0;
    double scale_ = 1;
    glm::dvec2 position_;
    glm::dvec2 velocity_;
};


#endif //WHIRLWINDSOFDANGERSKETCH_PARTICLE_H