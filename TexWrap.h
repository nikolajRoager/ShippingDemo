//
// Created by Nikolaj Christensen on 17/09/2025.
//

#ifndef COUNTRYBRAWL_TEXWRAP_H
#define COUNTRYBRAWL_TEXWRAP_H
#include <filesystem>
#include <mutex>
#include <string>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>


namespace fs = std::filesystem;

///Texture wrapper class
class TexWrap {
public:
    ///Thread safe Constructor which loads the image on the CPU but not the GPU (finalization required)
    explicit TexWrap(const fs::path& path);
    ///Thread safe Constructor which creates the image from words on the CPU but not the GPU (finalization required)
    TexWrap(const std::string& words, TTF_Font* _font,int wrapLength=-1);
    ///Should only be called on the main thread! constructor which loads the image directly onto the GPU
    TexWrap(const fs::path& path, SDL_Renderer* renderer);
    ///Should only be called on the main thread! constructor which creates the image from words directly onto the GPU
    TexWrap(const std::string& words, SDL_Renderer* renderer, TTF_Font* _font,int wrapLength=-1);

    void reset(const std::string& words, SDL_Renderer* renderer, TTF_Font* _font,int wrapLength=-1);
    TexWrap(TexWrap&& tex) noexcept ;
    TexWrap& operator=(TexWrap&& other) noexcept ;
    ~TexWrap();

    ///Should only be called on the main thread! uploads the texture from cpu to gpu and clears the image from the cpu
    void uploadToGPU(SDL_Renderer* renderer);


    //===A lot of different render functions, since clipping and rotating don't work well together, we also have options for tinting the image or not===
    ///Should only be called on the main thread!
    void render(int x, int y, SDL_Renderer* renderer,  double scale=1.0, bool centerX=false, bool centerY=false,  bool flip=false, unsigned int frames=1, unsigned int frame=0,double angle=0) const;
    ///Should only be called on the main thread!
    void render(int x, int y, Uint8 r, Uint8 g, Uint8 b, SDL_Renderer* renderer, double scale=1.0,  bool centerX=false, bool centerY=false, bool flip=false, unsigned int frames=1, unsigned int frame=0,double angle=0) const;
    ///Should only be called on the main thread!
    void render(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Renderer* renderer, double scale=1.0, bool centerX=false, bool centerY=false, bool flip=false, unsigned int frames=1, unsigned int frame=0,double angle=0) const;

    void renderRotated(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Renderer* renderer, int rcenterX, int rcenterY, double scale=1.0, bool centerX=false, bool centerY=false, bool flip=false, unsigned int frames=1, unsigned int frame=0,double angle=0) const;

    ///Should only be called on the main thread!
    void render(int x, int y, SDL_Renderer* renderer, const SDL_Rect& clip,  double scale=1.0, bool centerX=false, bool centerY=false,  bool flip=false, unsigned int frames=1, unsigned int frame=0) const;
    ///Should only be called on the main thread!
    void render(int x, int y, Uint8 r, Uint8 g, Uint8 b, SDL_Renderer* renderer, const SDL_Rect& clip, double scale=1.0,  bool centerX=false, bool centerY=false, bool flip=false, unsigned int frames=1, unsigned int frame=0) const;
    ///Should only be called on the main thread!
    void render(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Renderer* renderer, const SDL_Rect& clip, double scale=1.0, bool centerX=false, bool centerY=false, bool flip=false, unsigned int frames=1, unsigned int frame=0) const;
    void renderClipRotate(int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a, SDL_Renderer* renderer, const SDL_Rect& clip, double scale=1.0, bool centerX=false, bool centerY=false, bool flip=false, unsigned int frames=1, unsigned int frame=0, double angle = 0) const;


    ///Get width of image in pixels
    [[nodiscard]] int getWidth() const {return width_;}
    ///Get height of image in pixels
    [[nodiscard]] int getHeight() const {return height_;}
private:
    //data on the GPU
    SDL_Texture* gpuTexture_;
    //The data on the CPU
    SDL_Surface* cpuTexture_;
    //Width in pixels
    int width_;
    //height in pixels
    int height_;

    ///Mutex to ensure we don't try to access the cpu texture or width, height in multiple threads
    std::mutex textureMutex;
};

#endif //COUNTRYBRAWL_TEXWRAP_H