//
// Created by nikolaj on 5/14/26.
//

#include "textureManager.h"

#include <iostream>
#include <ranges>
#include <set>

void TextureManager::uploadTexturesToGPU(SDL_Renderer *renderer) {
    for (auto &textures: textures_ | std::views::values) {
        textures->uploadToGPU(renderer);
    }
}

void TextureManager::launchTextureLoading(const std::vector<fs::path> &textureRequests, const fs::path &assetsPath, ThreadPool &pool) {
    //A list of the names we are loading, used to detect duplicates
    std::set<std::string> namesLoading;
    assetsToLoad_=0;
    for (const auto &path: textureRequests) {
        std::string name = path.string();
        if (!namesLoading.contains(name)) {
            namesLoading.insert(name);
            pool.enqueue(
                [this, assetsPath, path,name, &pool]() {
                    if (!cancelled_.load(std::memory_order_relaxed))
                        try {
                            auto t = std::make_shared<TexWrap>(assetsPath/path);

                            std::lock_guard<std::mutex> lock(textureMutex_);
                            textures_.emplace(name, t);
                            loadedAssets_.fetch_add(1);
                        } catch (std::exception &e) {
                            //The result mutex pulls double duty, and also locks the exception message
                            std::lock_guard<std::mutex> lock(textureMutex_);
                            //It was certainly processed, even if the outcome isn't what we wanted
                            loadedAssets_.fetch_add(1);
                            bool expected = false;
                            if (cancelled_.compare_exchange_strong(expected, true)) {
                                errorMessage_= e.what();//We only save the first error message
                                pool.cancel(); //stop new tasks immediately
                            }
                        }
                });
            ++assetsToLoad_;
        }
    }
}

