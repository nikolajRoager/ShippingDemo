//
// Created by nikolaj on 7/6/26.
//

#ifndef PLATFORMERSKETCH_TITLEMENU_H
#define PLATFORMERSKETCH_TITLEMENU_H
#include <memory>
#include <utility>

#include "../scene.h"
#include "../soundWrap.h"
#include "../textureManager.h"
#include "../TexWrap.h"
#include "../MIGUI/buttonControl.h"
#include "../MIGUI/emptyControl.h"
#include "../MIGUI/guiManager.h"
#include "../MIGUI/numberInputControl.h"
#include "../MIGUI/slideControl.h"
#include "../MIGUI/textureControl.h"

class TitleMenu : public Scene {
public:
    TitleMenu(SDL_Renderer* renderer, int screenWidth, int screenHeight, TTF_Font* smallFont, TTF_Font* midFont, TTF_Font* largeFont, int smallFontSize, int midFontSize, int largeFontSize, bool startAtSettings=false);
    ~TitleMenu() override;

    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight,const InputData& userInputs, unsigned int millis, unsigned int pmillis) const override;
    std::optional<std::pair<SceneInfo,SceneOutput>> update(SDL_Renderer* renderer, int screenWidth, int screenHeight,const InputData& userInputs,  unsigned int millis, unsigned int dmillis, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont) override;
private:

    void setupGui(SDL_Renderer *renderer, int screenWidth, int screenHeight, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont);
    void saveFontSize() const;

    //Different pages in the menu slides
    const int splash0Page_=0;
    const int splash1Page_=1;
    const int mainMenuPage_=2;
    const int settingsPage_=3;
    const int creditsPage_=4;
    const int levelSelectPage_=5;

    //Assets
    std::shared_ptr<const SoundWrap> clickSound_;
    std::string splashText_;

    //For fade-in
    int transitionTimer_;
    bool transitionIn_;
    const int maxTransitionTimer_=10000;

    //Used by GUI:
    std::shared_ptr<const TexWrap> plusButtonTexture_;
    std::shared_ptr<const TexWrap> minusButtonTexture_;
    std::shared_ptr<const TexWrap> expandButtonTexture_;

    //main Menu
    std::shared_ptr<buttonControl> playGameButton_;
    std::shared_ptr<buttonControl> creditsButton_;
    std::shared_ptr<buttonControl> settingsButton_;
    std::shared_ptr<buttonControl> quitButton_;

    //Credits page
    std::shared_ptr<buttonControl> goBackFromCreditsButton_;
    std::shared_ptr<emptyControl> creditsGoHere_;

    //Assets for credits page
    std::vector<std::string> creditsText_;
    std::vector<std::shared_ptr<TexWrap>> creditsTextures_;
    int creditsY=0;

    //Settings menu
    std::shared_ptr<buttonControl> goBackFromSettingsButton_;
    int smallFontSize_;
    int midFontSize_;
    int largeFontSize_;
    std::shared_ptr<NumberInputControl> smallFontSizeButton_;
    std::shared_ptr<NumberInputControl> largeFontSizeButton_;
    std::shared_ptr<NumberInputControl> midFontSizeButton_;
    std::shared_ptr<buttonControl> applySettingsButton_;
    std::shared_ptr<textureControl> toggleParticlesTC_;
    bool enableParticles_=true;
    std::shared_ptr<buttonControl> toggleParticlesButton_;
    std::shared_ptr<NumberInputControl> soundVolumeControl_;
    std::shared_ptr<NumberInputControl> musicVolumeControl_;

    //Level select screen
    std::shared_ptr<buttonControl> goBackFromLevelSelect_;

    struct LevelSelectData {
        std::string levelName;
        bool unlocked = false;
        std::string description;
        std::shared_ptr<buttonControl> selectLevelButtons_;

        LevelSelectData(const std::string& levelName,const std::string& description, bool unlocked, std::shared_ptr<buttonControl> selectLevelButtons) {
            this->levelName = levelName;
            this->unlocked = unlocked;
            this->description = description;
            this->selectLevelButtons_ = std::move(selectLevelButtons);
        };
    };

    std::vector<LevelSelectData> levelSelectData_;
    int selectedLevelIndex_=0;
    std::shared_ptr<buttonControl> playSelectedLevelButton_;



    //The control which handles all the slides of the menu
    std::shared_ptr<SlideControl> menuSlides_;

    std::unique_ptr<GUIManager> gui_;

    TextureManager textureManager_;
    NumberRenderer midNumberRenderer_;
};


#endif //PLATFORMERSKETCH_TITLEMENU_H