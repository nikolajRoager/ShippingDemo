//
// Created by nikolaj on 5/14/26.
//

#ifndef LCCRENDERINGDEMO_TEXTUREMANAGER_H
#define LCCRENDERINGDEMO_TEXTUREMANAGER_H
#include <iostream>
#include <map>
#include <ranges>
#include <string>
#include <vector>

#include "TexWrap.h"
#include "ThreadPool.h"


class TextureManager {
public:
    TextureManager()=default;
    ~TextureManager()=default;

    ///Launches threads loading the textures onto the CPU, if loading fails these threads cancel and the error message is updated
    void launchTextureLoading(const std::vector<fs::path>& textureRequests, const fs::path& assetsPath, ThreadPool& pool);
    ///MUST BE DONE ON MAIN THREAD, upload all textures to the graphics card
    void uploadTexturesToGPU(SDL_Renderer* renderer);

    ///This should only be called if the threadpool has stopped
    [[nodiscard]] const std::string& getErrorMessage() const {
        //No lock guard since this is a const function, also this should only be called if the threadpool has been cancelled
        return errorMessage_;
    }
    ///Texture users are expected to ask for their textures by name in their own constructor, the shared ptr makes sure it doesn't matter if the texture manager is deleted before the users
    [[nodiscard]] std::shared_ptr<const TexWrap> getTexWrap(const fs::path& path) const {
        if (textures_.contains(path.string()))
            return textures_.at(path.string());
        else
            throw std::runtime_error("No such texture: " + path.string());
    }

    [[nodiscard]] size_t getAssetsToLoad() const {return assetsToLoad_;}
    [[nodiscard]] size_t getLoadedAssets() const {return loadedAssets_.load();}
    [[nodiscard]] bool isCanceled() const {return cancelled_.load();}
private:
    std::map<std::string,std::shared_ptr<TexWrap>> textures_;
    size_t assetsToLoad_ = 0;
    std::atomic<size_t> loadedAssets_ = 0;
    std::atomic<bool> cancelled_ = false;

    //Mutex used when writing either to the textures or the error message
    std::mutex textureMutex_;
    std::string errorMessage_;
};


#endif //LCCRENDERINGDEMO_TEXTUREMANAGER_H