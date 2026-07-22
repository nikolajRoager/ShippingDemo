//
// Created by Nikolaj Christensen on 17/09/2025.
//

#include "TexWrap.h"

#include <iostream>
#include <ostream>
#include <stdexcept>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

TexWrap::~TexWrap() {
    if (cpuTexture_!=nullptr) {
        SDL_FreeSurface(cpuTexture_);
    }
    if (gpuTexture_!=nullptr) {
        SDL_DestroyTexture(gpuTexture_);
    }
}

TexWrap::TexWrap(TexWrap&& other) noexcept {
    gpuTexture_=other.gpuTexture_;
    other.gpuTexture_=nullptr;
    cpuTexture_=other.cpuTexture_;
    other.cpuTexture_=nullptr;
    width_=other.width_;
    height_=other.height_;
}

TexWrap&  TexWrap::operator=(TexWrap&& other)  noexcept {
    gpuTexture_=other.gpuTexture_;
    other.gpuTexture_=nullptr;
    cpuTexture_=other.cpuTexture_;
    other.cpuTexture_=nullptr;
    width_=other.width_;
    height_=other.height_;
    return *this;
}

TexWrap::TexWrap(const std::string& words, TTF_Font* _font,int wrapLength) {
    //We will use the option to tint the image at render time, so leave the text white
    const SDL_Color color = {255,255,255,255};
    cpuTexture_ =nullptr;
    if (wrapLength<=0)
        cpuTexture_ = TTF_RenderUTF8_Blended( _font,!words.empty()? words.c_str() :" ", color);
    else
        cpuTexture_ = TTF_RenderUTF8_Blended_Wrapped( _font,!words.empty()? words.c_str() :" ", color,wrapLength);
    if (cpuTexture_ == nullptr) {
        throw std::runtime_error("Unable to create text texture: " + std::string(SDL_GetError()));
    }

    width_ = cpuTexture_->w;
    height_ = cpuTexture_->h;

    gpuTexture_=nullptr;
}

TexWrap::TexWrap(const fs::path& path) {

    cpuTexture_ = IMG_Load(path.string().c_str());
    if (cpuTexture_ == nullptr) {
        throw std::runtime_error("Unable to load image: " + std::string(SDL_GetError()));
    }

    width_ = cpuTexture_->w;
    height_ = cpuTexture_->h;

    gpuTexture_=nullptr;
}

void TexWrap::reset(const std::string& words, SDL_Renderer* renderer, TTF_Font* _font,int wrapLength) {
    if (cpuTexture_!=nullptr) {
        SDL_FreeSurface(cpuTexture_);
    }
    if (gpuTexture_!=nullptr) {
        SDL_DestroyTexture(gpuTexture_);
    }

    //We will use the option to tint the image at render time, so leave the text white
    const SDL_Color color = {255,255,255,255};
    cpuTexture_ =nullptr;
    if (wrapLength<=0)
        cpuTexture_ = TTF_RenderUTF8_Blended( _font,!words.empty()? words.c_str() :" ", color);
    else
        cpuTexture_ = TTF_RenderUTF8_Blended_Wrapped( _font,!words.empty()? words.c_str() :" ", color,wrapLength);
    if (cpuTexture_ == nullptr) {
        throw std::runtime_error("Unable to create text texture: " + std::string(SDL_GetError()));
    }

    width_ = cpuTexture_->w;
    height_ = cpuTexture_->h;

    gpuTexture_= SDL_CreateTextureFromSurface(renderer, cpuTexture_);
    SDL_FreeSurface(cpuTexture_);
    cpuTexture_ = nullptr;

    if (gpuTexture_ == nullptr) {
        throw std::runtime_error("Unable to create texture: " + std::string(SDL_GetError()));
    }
}

TexWrap::TexWrap(const std::string& words, SDL_Renderer* renderer, TTF_Font* _font,int wrapLength) {
    //We will use the option to tint the image at render time, so leave the text white
    const SDL_Color color = {255,255,255,255};
    cpuTexture_ =nullptr;
    if (wrapLength<=0)
        cpuTexture_ = TTF_RenderUTF8_Blended( _font,!words.empty()? words.c_str() :" ", color);
    else
        cpuTexture_ = TTF_RenderUTF8_Blended_Wrapped( _font,!words.empty()? words.c_str() :" ", color,wrapLength);
    if (cpuTexture_ == nullptr) {
        throw std::runtime_error("Unable to create text texture: " + std::string(SDL_GetError()));
    }

    width_ = cpuTexture_->w;
    height_ = cpuTexture_->h;

    gpuTexture_= SDL_CreateTextureFromSurface(renderer, cpuTexture_);
    SDL_FreeSurface(cpuTexture_);
    cpuTexture_ = nullptr;

    if (gpuTexture_ == nullptr) {
        throw std::runtime_error("Unable to create texture: " + std::string(SDL_GetError()));
    }
}

TexWrap::TexWrap(const fs::path& path, SDL_Renderer* renderer) {
    cpuTexture_ = IMG_Load(path.string().c_str());
    if (cpuTexture_ == nullptr) {
        throw std::runtime_error("Unable to load image: " + std::string(SDL_GetError()));
    }

    width_ = cpuTexture_->w;
    height_ = cpuTexture_->h;

    gpuTexture_= SDL_CreateTextureFromSurface(renderer, cpuTexture_);

    SDL_FreeSurface(cpuTexture_);
    cpuTexture_ = nullptr;
    if (gpuTexture_ == nullptr) {
        throw std::runtime_error("Unable to create texture: " + std::string(SDL_GetError()));
    }
}

void TexWrap::render(int x, int y, SDL_Renderer *renderer, double scale,bool centerX,bool centerY,bool flip, unsigned int frames,unsigned int frame,double angle) const {
    render(x,y,255,255,255,255,renderer,scale,centerX,centerY,flip,frames,frame,angle);
}

void TexWrap::render(int x, int y, Uint8 r, Uint8 g, Uint8 b, SDL_Renderer *renderer, double scale, bool centerX, bool centerY,bool flip, unsigned int frames,unsigned  int frame,double angle) const {
    render(x,y,r,g,b,255,renderer,scale,centerX,centerY,flip,frames,frame,angle);
}

void TexWrap::renderRotated(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Renderer *renderer, int rcenterX, int rcenterY, double scale, bool centerX, bool centerY, bool flip, unsigned int frames, unsigned int frame, double angle) const {
    if (gpuTexture_!=nullptr)
    {
        //Set rendering space and render to screen
        //The texture data may be sprites with multiple frames in the x direction, if there are multiple frames w is the width of the single frame
        int frameWidth = (width_)/frames;

        SDL_Rect renderQuad = { static_cast<int>(x), static_cast<int>(y), frameWidth, height_};

        //Scale the destination
        renderQuad.w *= scale;
        renderQuad.h *= scale;

        //Plot relative to the center rather than top left corner if the user wants that
        if (centerX)
            renderQuad.x -= renderQuad.w/2;
        if (centerY)
            renderQuad.y -= renderQuad.h/2;

        SDL_Rect srect = { frameWidth*(int)frame, 0, frameWidth, height_ };

        //Set image tint
        SDL_SetTextureColorMod(gpuTexture_, r,g,b);
        SDL_SetTextureAlphaMod(gpuTexture_,a);
        //Point to rotate around
        SDL_Point centerPoint = {rcenterX-renderQuad.x, rcenterY-renderQuad.y};

        SDL_RenderCopyEx( renderer, gpuTexture_, &srect, &renderQuad ,angle*180.0/(M_PI),&centerPoint ,flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
}


void TexWrap::render(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Renderer *renderer, double scale, bool centerX, bool centerY,bool flip,unsigned int frames,unsigned int frame,double angle) const {
    if (gpuTexture_!=nullptr)
    {
        //Set rendering space and render to screen
        //The texture data may be sprites with multiple frames in the x direction, if there are multiple frames w is the width of the single frame
        int frameWidth = (width_)/frames;

        SDL_Rect renderQuad = { static_cast<int>(x), static_cast<int>(y), frameWidth, height_};

        //Scale the destination
        renderQuad.w *= scale;
        renderQuad.h *= scale;

        //Plot relative to the center rather than top left corner if the user wants that
        if (centerX)
            renderQuad.x -= renderQuad.w/2;
        if (centerY)
            renderQuad.y -= renderQuad.h/2;

        SDL_Rect srect = { frameWidth*(int)frame, 0, frameWidth, height_ };

        //Set image tint
        SDL_SetTextureColorMod(gpuTexture_, r,g,b);
        SDL_SetTextureAlphaMod(gpuTexture_,a);
        //Point to rotate around
        SDL_Point centerPoint = {renderQuad.w/2, renderQuad.h/2};

        SDL_RenderCopyEx( renderer, gpuTexture_, &srect, &renderQuad ,angle*180.0/(M_PI),&centerPoint ,flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
}

void TexWrap::render(int x, int y, SDL_Renderer *renderer, const SDL_Rect& clip, double scale,bool centerX,bool centerY,bool flip, unsigned int frames,unsigned int frame) const {
    render(x,y,255,255,255,255,renderer,clip,scale,centerX,centerY,flip,frames,frame);
}

void TexWrap::render(int x, int y, Uint8 r, Uint8 g, Uint8 b, SDL_Renderer *renderer, const SDL_Rect& clip, double scale, bool centerX, bool centerY,bool flip, unsigned int frames,unsigned  int frame) const {

    render(x,y,r,g,b,255,renderer,clip,scale,centerX,centerY,flip,frames,frame);
}

void TexWrap::render(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Renderer *renderer, const SDL_Rect& clip, double scale, bool centerX, bool centerY,bool flip,unsigned int frames,unsigned int frame) const {
    if (gpuTexture_!=nullptr)
    {
        //Set rendering space and render to screen
        //The texture data may be sprites with multiple frames in the x direction, if there are multiple frames w is the width of the single frame
        int frameWidth = (width_)/frames;

        SDL_Rect renderQuad = { static_cast<int>(x), static_cast<int>(y), frameWidth, height_};

        //Scale the destination
        renderQuad.w *= scale;
        renderQuad.h *= scale;

        //Plot relative to the center rather than top left corner if the user wants that
        if (centerX)
            renderQuad.x -= renderQuad.w/2;
        if (centerY)
            renderQuad.y -= renderQuad.h/2;

        SDL_Rect srect = { frameWidth*(int)frame, 0, frameWidth, height_ };

        // --- Clipping logic ---
        // Find how many pixels are clipped off each edge of the destination rect,
        // then adjust the source rect proportionally so we only sample the
        // texels that correspond to the visible portion of the destination.

        // Pixels clipped from each side
        int clipLeft   = std::max(0, clip.x - renderQuad.x);
        int clipTop    = std::max(0, clip.y - renderQuad.y);
        int clipRight  = std::max(0, (renderQuad.x + renderQuad.w) - (clip.x + clip.w));
        int clipBottom = std::max(0, (renderQuad.y + renderQuad.h) - (clip.y + clip.h));

        // If the quad is entirely outside the clip rect, nothing to draw
        if (clipLeft + clipRight >= renderQuad.w || clipTop + clipBottom >= renderQuad.h)
            return;

        // Texels per destination pixel (inverse of scale)
        double texelsPerPixel = 1.0 / scale;

        // Adjust source rect
        srect.x += (int)(clipLeft   * texelsPerPixel);
        srect.y += (int)(clipTop    * texelsPerPixel);
        srect.w -= (int)((clipLeft + clipRight)  * texelsPerPixel);
        srect.h -= (int)((clipTop  + clipBottom) * texelsPerPixel);

        // Adjust destination rect
        renderQuad.x += clipLeft;
        renderQuad.y += clipTop;
        renderQuad.w -= clipLeft + clipRight;
        renderQuad.h -= clipTop  + clipBottom;

        //Set image tint
        SDL_SetTextureColorMod(gpuTexture_, r,g,b);
        SDL_SetTextureAlphaMod(gpuTexture_,a);
        //Point to rotate around
        SDL_Point centerPoint = {renderQuad.w/2, renderQuad.h/2};

        SDL_RenderCopyEx( renderer, gpuTexture_, &srect, &renderQuad ,0.f,&centerPoint ,flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
}

void TexWrap::renderClipRotate(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Renderer *renderer, const SDL_Rect &clip, double scale, bool centerX, bool centerY, bool flip, unsigned int frames, unsigned int frame, double angle) const {
    if (gpuTexture_!=nullptr)
    {
        //Set rendering space and render to screen
        //The texture data may be sprites with multiple frames in the x direction, if there are multiple frames w is the width of the single frame
        int frameWidth = (width_)/frames;

        SDL_Rect renderQuad = { static_cast<int>(x), static_cast<int>(y), frameWidth, height_};

        //Scale the destination
        renderQuad.w *= scale;
        renderQuad.h *= scale;

        //Plot relative to the center rather than top left corner if the user wants that
        if (centerX)
            renderQuad.x -= renderQuad.w/2;
        if (centerY)
            renderQuad.y -= renderQuad.h/2;

        SDL_Rect srect = { frameWidth*(int)frame, 0, frameWidth, height_ };

        //Point to rotate around
        SDL_Point centerPoint = {renderQuad.w/2, renderQuad.h/2};
        // --- Clipping logic ---
        // Find how many pixels are clipped off each edge of the destination rect,
        // then adjust the source rect proportionally so we only sample the
        // texels that correspond to the visible portion of the destination.

        // Pixels clipped from each side
        int clipLeft   = std::max(0, clip.x - renderQuad.x);
        int clipTop    = std::max(0, clip.y - renderQuad.y);
        int clipRight  = std::max(0, (renderQuad.x + renderQuad.w) - (clip.x + clip.w));
        int clipBottom = std::max(0, (renderQuad.y + renderQuad.h) - (clip.y + clip.h));

        // If the quad is entirely outside the clip rect, nothing to draw
        if (clipLeft + clipRight >= renderQuad.w || clipTop + clipBottom >= renderQuad.h)
            return;

        // Texels per destination pixel (inverse of scale)
        double texelsPerPixel = 1.0 / scale;

        // Adjust source rect
        srect.x += (int)(clipLeft   * texelsPerPixel);
        srect.y += (int)(clipTop    * texelsPerPixel);
        srect.w -= (int)((clipLeft + clipRight)  * texelsPerPixel);
        srect.h -= (int)((clipTop  + clipBottom) * texelsPerPixel);

        // Adjust destination rect
        renderQuad.x += clipLeft;
        renderQuad.y += clipTop;
        renderQuad.w -= clipLeft + clipRight;
        renderQuad.h -= clipTop  + clipBottom;

        //Set image tint
        SDL_SetTextureColorMod(gpuTexture_, r,g,b);
        SDL_SetTextureAlphaMod(gpuTexture_,a);

        SDL_RenderCopyEx( renderer, gpuTexture_, &srect, &renderQuad ,angle*180.0/(M_PI),&centerPoint ,flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    }
}


void TexWrap::uploadToGPU(SDL_Renderer* renderer) {
    //In practice, I only expect this to be called once the threadpool has finished and been destroyed
    std::lock_guard<std::mutex> lock(textureMutex);
    //We can only upload if the CPU texture exists, and if we have already uploaded, there is no point
    if (cpuTexture_ != nullptr && gpuTexture_ == nullptr) {
        gpuTexture_= SDL_CreateTextureFromSurface(renderer, cpuTexture_);
        if (gpuTexture_ == nullptr) {
            throw std::runtime_error("Unable to create texture: " + std::string(SDL_GetError()));
        }
        SDL_FreeSurface(cpuTexture_);
        cpuTexture_ = nullptr;
    }
}