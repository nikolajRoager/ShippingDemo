//
// Created by nikolaj on 7/6/26.
//

#include "titleMenu.h"

#include <fstream>

#include "../getAssets.h"
#include "../MIGUI/mouseOverControl.h"
#include "../MIGUI/multilineStackControl.h"
#include "../MIGUI/stackControl.h"
#include "../MIGUI/tableControl.h"
#include "../MIGUI/textureControl.h"
#include "../nlohmann/json.hpp"

TitleMenu::TitleMenu(SDL_Renderer *renderer, int screenWidth, int screenHeight, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont, int smallFontSize, int midFontSize, int largeFontSize, bool startAtSettings):midNumberRenderer_(0,midFont,renderer) {
    //First launch the threadpool and start loading everything
    //Using a threadpool is CRAZY overkill for this project, but I always use a t hreadpool for loading for easy scalability
    std::vector<fs::path> textureRequests
    {
        fs::path("menu")/"plus.png",
        fs::path("menu")/"minus.png",
        fs::path("menu")/"expand.png",
    };

    ThreadPool loadingPool(std::thread::hardware_concurrency());
    //We can do other stuff while our textures are loading in the background
    textureManager_.launchTextureLoading(textureRequests, assetsPath(),loadingPool);

    smallFontSize_ = smallFontSize;
    midFontSize_ = midFontSize;
    largeFontSize_ = largeFontSize;

    clickSound_=std::make_shared<SoundWrap>(assetsPath()/"sounds"/"click.mp3");

    {
        std::ifstream splashStream(assetsPath()/"menu"/"splashscreen.txt");
        if (!splashStream.is_open()) {
            throw std::runtime_error("Could not open splashscreen.txt");
        }
        std::string line;
        while (std::getline(splashStream,line)) {
            splashText_.append(line+"\n");
        }
        splashStream.close();
    }

    {
        std::ifstream creditsStream(assetsPath()/"menu"/"credits.txt");
        if (!creditsStream.is_open()) {
            throw std::runtime_error("Could not open credits.txt");
        }
        std::string line;
        bool first = true;
        while (std::getline(creditsStream,line)) {
            creditsTextures_.emplace_back(std::make_shared<TexWrap>(line,renderer,first?largeFont:midFont,screenWidth));
            creditsText_.emplace_back(line);
            first = false;
        }
        creditsStream.close();
        creditsY=creditsTextures_.empty()? 0 : creditsTextures_.front()->getHeight();
    }

    transitionIn_=true;
    transitionTimer_=0;

    //Now lets wait for the texture loading to finish and display a loading bar
    while (textureManager_.getLoadedAssets()< textureManager_.getAssetsToLoad()&& !textureManager_.isCanceled()) {
        //Respond to window resize events;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_WINDOWEVENT) {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        screenWidth= event.window.data1;
                        screenHeight= event.window.data2;
                        break;
                    default:
                        break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        SDL_Rect loadingBarRect = {
            0, screenHeight/ 2, static_cast<int>((screenWidth* textureManager_.getLoadedAssets()) / textureManager_.getAssetsToLoad()), screenHeight/ 4
        };
        SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
        SDL_RenderFillRect(renderer, &loadingBarRect);
        SDL_RenderPresent(renderer);
    }

    if (textureManager_.isCanceled()) {
        throw std::runtime_error("The loading cancelled with the following error: " + textureManager_.getErrorMessage());
    }

    //Upload textures to the GPU, this MUST be done on the main thread, that is SUPER IMPORTANT
    textureManager_.uploadTexturesToGPU(renderer);

    //Get the textures we ordered
    plusButtonTexture_=textureManager_.getTexWrap(fs::path("menu")/"plus.png");
    minusButtonTexture_=textureManager_.getTexWrap(fs::path("menu")/"minus.png");
    expandButtonTexture_=textureManager_.getTexWrap(fs::path("menu")/"expand.png");

    //The GUI setup must be called after the textures have been loaded since we use the textures
    setupGui(renderer,screenWidth,screenHeight,smallFont,midFont,largeFont);

    if (startAtSettings) {
        transitionIn_=true;
        transitionTimer_=maxTransitionTimer_;
        menuSlides_->setActiveSlide(3);
    }
}

TitleMenu::~TitleMenu() =default;

void TitleMenu::saveFontSize() const {
    // 1. Open and parse the JSON file
    std::ifstream inputFile(assetsPath()/"usersettings.json");
    if (!inputFile.is_open()) {
        throw std::runtime_error("Could not open file usersettings.json");
    }

    nlohmann::json settings;
    inputFile >> settings;
    inputFile.close();

    // 2. Modify the values + enforce sanity limits
    settings["smallFontSize"] = std::min(std::max(smallFontSizeButton_->getValue(),8),128);
    settings["midFontSize"] = std::min(std::max(midFontSizeButton_->getValue(),8),128);
    settings["largeFontSize"] = std::min(std::max(largeFontSizeButton_->getValue(),8),128);

    // 3. Save back to file
    std::ofstream outputFile(assetsPath()/"usersettings.json");
    if (!outputFile.is_open()) {
        throw std::runtime_error("Could not open file usersettings.json for writing");
    }

    outputFile << settings.dump(4) << std::endl;
    outputFile.close();
}

void TitleMenu::setupGui(SDL_Renderer *renderer, int screenWidth, int screenHeight, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont) {
    //First splashscreen, a warning to set the size correctly
    std::shared_ptr<control> splash0Control;
    {
        std::shared_ptr<control> nothingAbove=std::make_shared<emptyControl>();
        std::shared_ptr<control> nothingLeft0=std::make_shared<emptyControl>();
        std::shared_ptr<textureControl> splashTexture = std::make_shared<textureControl>(std::make_shared<TexWrap>(splashText_,renderer,smallFont,1024));
        std::shared_ptr<control> nothingRight0=std::make_shared<emptyControl>();
        std::shared_ptr<control> nothingBelow=std::make_shared<emptyControl>();

        auto leftRight =std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth/3),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/3),tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth/3)},std::vector<std::shared_ptr<control> >{nothingLeft0,splashTexture,nothingRight0});
        splash0Control=std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight/3),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight/3),tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight/3)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth)},std::vector<std::shared_ptr<control> >{nothingAbove,leftRight,nothingBelow});
    }

    //Second splash screen, who made this and logo (if any)
    std::shared_ptr<control> splash1Control;
    {
        std::shared_ptr<control> nothingAbove=std::make_shared<emptyControl>();
        std::shared_ptr<control> nothingLeft0=std::make_shared<emptyControl>();
        std::shared_ptr<textureControl> splashTexture = std::make_shared<textureControl>(std::make_shared<TexWrap>("A game by Nikolaj Roager Christensen",renderer,midFont));
        std::shared_ptr<control> nothingRight0=std::make_shared<emptyControl>();
        std::shared_ptr<control> nothingBelow=std::make_shared<emptyControl>();

        auto leftRight =std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth/3),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/3),tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth/3)},std::vector<std::shared_ptr<control> >{nothingLeft0,splashTexture,nothingRight0});
        splash1Control=std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight/3),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight/3),tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight/3)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth)},std::vector<std::shared_ptr<control> >{nothingAbove,leftRight,nothingBelow});
    }

    //---MAIN MENU PAGE----
    std::shared_ptr<control> mainTable ;
    {
        std::shared_ptr<control> gameTitle =std::make_shared<textureControl> (std::make_shared<TexWrap>("Whirlwinds of Danger",renderer,largeFont));
        std::shared_ptr<control> nothingLeft0=std::make_shared<emptyControl>();
        std::shared_ptr<control> nothingRight0=std::make_shared<emptyControl>();
        std::shared_ptr<control> nothingDown0=std::make_shared<emptyControl>();
        std::shared_ptr<control> nothingDown1=std::make_shared<emptyControl>();
        std::shared_ptr<control> createPartyTextureControl=std::make_shared<textureControl> (std::make_shared<TexWrap>("Start Game",renderer,midFont));
        std::shared_ptr<control> settingsTextureControl=std::make_shared<textureControl> (std::make_shared<TexWrap>("Settings",renderer,midFont));
        std::shared_ptr<control> creditsTextureControl=std::make_shared<textureControl> (std::make_shared<TexWrap>("View Credits",renderer,midFont));
        std::shared_ptr<control> quitGameTextureControl=std::make_shared<textureControl> (std::make_shared<TexWrap>("Quit Game",renderer,midFont));
        playGameButton_=std::make_shared<buttonControl>(createPartyTextureControl,128,128,128,false);
        settingsButton_=std::make_shared<buttonControl>(settingsTextureControl,128,128,128,false);
        creditsButton_=std::make_shared<buttonControl>(creditsTextureControl,128,128,128,false);
        quitButton_=std::make_shared<buttonControl>(quitGameTextureControl,128,128,128,false);
        std::shared_ptr<stackControl> buttonStack = std::make_shared<stackControl>(stackControl::VERTICAL,std::vector<std::shared_ptr<control> >{playGameButton_,creditsButton_,settingsButton_,quitButton_});
        std::shared_ptr<control> middleMenu= std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight/3),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight/3),tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight/3)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth)},std::vector<std::shared_ptr<control> >{gameTitle,buttonStack ,nothingDown1});
        mainTable = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth/2)},std::vector<std::shared_ptr<control> >{middleMenu,nothingRight0});
    }

    //---SETTINGS PAGE---
    std::shared_ptr<control> settingsMenu;
    {
        std::shared_ptr<control> nothingRight0=std::make_shared<emptyControl>();

        std::shared_ptr<control> settingsTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Settings",renderer,largeFont));
        std::shared_ptr<control> smallFontSampleTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("That was large font, this is small font btw.",renderer,smallFont));
        std::shared_ptr<control> smallFontSizeTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Small font size:",renderer,midFont));
        std::shared_ptr<control> midFontSizeTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Medium font size:",renderer,midFont));
        std::shared_ptr<control> largeFontSizeTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Large font size:",renderer,midFont));
        smallFontSizeButton_=std::make_shared<NumberInputControl>(midNumberRenderer_,smallFontSize_,8,128,plusButtonTexture_,minusButtonTexture_);
        midFontSizeButton_=std::make_shared<NumberInputControl>(midNumberRenderer_,midFontSize_,8,128,plusButtonTexture_,minusButtonTexture_);
        largeFontSizeButton_=std::make_shared<NumberInputControl>(midNumberRenderer_,largeFontSize_,8,128,plusButtonTexture_,minusButtonTexture_);

        std::shared_ptr<control> smallFontPair = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2)},std::vector<std::shared_ptr<control>>{smallFontSizeTC,smallFontSizeButton_});
        std::shared_ptr<control> midFontPair = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2)},std::vector<std::shared_ptr<control>>{midFontSizeTC,midFontSizeButton_});
        std::shared_ptr<control> largeFontPair = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2)},std::vector<std::shared_ptr<control>>{largeFontSizeTC,largeFontSizeButton_});

        std::shared_ptr<control> goBackTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Back",renderer,midFont));
        std::shared_ptr<control> applyTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Apply fonts (forces reload)",renderer,midFont));
        goBackFromSettingsButton_=std::make_shared<buttonControl>(goBackTC,128,128,128,false);
        toggleParticlesTC_=std::make_shared<textureControl> (std::make_shared<TexWrap>("Disable particle effects",renderer,midFont));
        toggleParticlesButton_=std::make_shared<buttonControl>(toggleParticlesTC_);

        applySettingsButton_=std::make_shared<buttonControl>(applyTC,128,128,128,false);

        std::shared_ptr<control> soundTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Sound volume:",renderer,midFont));
        soundVolumeControl_=std::make_shared<NumberInputControl>(midNumberRenderer_,10,0,10,plusButtonTexture_,minusButtonTexture_);

        std::shared_ptr<control> musicTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Music volume:",renderer,midFont));
        musicVolumeControl_=std::make_shared<NumberInputControl>(midNumberRenderer_,10,0,10,plusButtonTexture_,minusButtonTexture_);

        std::shared_ptr<control> soundPair = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2)},std::vector<std::shared_ptr<control>>{soundTC,soundVolumeControl_});
        std::shared_ptr<control> musicPair = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2)},std::vector<std::shared_ptr<control>>{musicTC,musicVolumeControl_});

        std::shared_ptr<stackControl> buttonStack = std::make_shared<stackControl>(stackControl::VERTICAL,std::vector<std::shared_ptr<control> >{settingsTC,smallFontSampleTC,smallFontPair,midFontPair,largeFontPair,applySettingsButton_,soundPair,musicPair,toggleParticlesButton_,goBackFromSettingsButton_});
        settingsMenu = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth/2)},std::vector<std::shared_ptr<control> >{buttonStack,nothingRight0});

    }
    std::shared_ptr<control> creditsPage;
    {
        std::shared_ptr<control> goBackTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Back",renderer,midFont));
        goBackFromCreditsButton_ = std::make_shared<buttonControl>(goBackTC,128,128,128,false);
        auto buttonStack = std::make_shared<stackControl>(stackControl::HORIZONTAL,std::vector<std::shared_ptr<control> >{goBackFromCreditsButton_});
        creditsGoHere_=std::make_shared<emptyControl>();
        creditsPage = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight/2),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight/2)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth)},std::vector<std::shared_ptr<control> >{creditsGoHere_,buttonStack});
    }
    std::shared_ptr<control> levelSelect;
    {
        std::shared_ptr<control> goBackTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Back",renderer,midFont));
        goBackFromLevelSelect_= std::make_shared<buttonControl>(goBackTC,128,128,128,false);

        auto playLevelText= std::make_shared<TexWrap>(" Select a level to play",renderer,midFont);
        auto playLevelTC= std::make_shared<textureControl>(playLevelText);

        auto buttonStack = std::make_shared<stackControl>(stackControl::HORIZONTAL,std::vector<std::shared_ptr<control> >{goBackFromLevelSelect_,playLevelTC});

        //Load levels
        std::ifstream inputFile(assetsPath()/"levels"/"levels.json");
        if (!inputFile.is_open()) {
            throw std::runtime_error("Could not open file levels/levels.json");
        }

        nlohmann::json levelsData;
        inputFile >> levelsData;
        inputFile.close();

        levelSelectData_.clear();
        std::vector<std::shared_ptr<control> > levelControls;
        for (const auto& levelData : levelsData) {
            std::string name = levelData["name"].get<std::string>();

            bool unlocked = fs::exists(assetsPath()/"levels"/name/"playerForces.json");
            auto thumbnail = std::make_shared<TexWrap>(assetsPath()/"levels"/name/"thumbnail.png",renderer);
            auto thumbnailTC = std::make_shared<textureControl>(thumbnail);
            auto mouseOverText = std::make_shared<TexWrap>(levelData["displayName"].get<std::string>()+(unlocked?"":" LOCKED"),renderer,midFont);
            auto thumbnailMTC = std::make_shared<textureControl>(mouseOverText);
            auto mo = std::make_shared<mouseOverControl>(thumbnailTC,thumbnailMTC);
            auto button = std::make_shared<buttonControl>(mo);


            levelSelectData_.emplace_back(name,levelData["description"].get<std::string>(),unlocked,button);
            levelControls.push_back(button);
        }

        auto playTC = std::make_shared<textureControl>(std::make_shared<TexWrap>("Play",renderer,midFont));
        playSelectedLevelButton_=std::make_shared<buttonControl>(playTC);

        auto levelsContainer = std::make_shared<MultilineStackControl>(levelControls);
        levelSelect = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight/2),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight/2)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth)},std::vector<std::shared_ptr<control> >{levelsContainer,buttonStack});
    }


    menuSlides_ =std::make_shared<SlideControl>(std::vector<std::shared_ptr<control>>{splash0Control,splash1Control,mainTable,settingsMenu,creditsPage,levelSelect});

    gui_ = std::make_unique<GUIManager>(menuSlides_ );

}

void TitleMenu::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, const InputData &userInputs, unsigned int millis, unsigned int pmillis) const {
    gui_->render(renderer,screenWidth,screenHeight,255,255,255,transitionTimer_<maxTransitionTimer_?(255*transitionTimer_)/maxTransitionTimer_:255);
    if (menuSlides_->getActiveSlide()==creditsPage_) {//Roll credits
        int y = creditsGoHere_->getY0()+creditsGoHere_->getHeight()-creditsY;
        int x = creditsGoHere_->getX0();
        for (const auto &text: creditsTextures_) {
            int diff =creditsGoHere_->getY0()+creditsGoHere_->getHeight()-(y+text->getHeight());
            if (diff>0) {
                text->render(x,y,255,255,255,diff<255?diff:255, renderer);
            }
            y+=std::max(text->getHeight(),12);
        }
    }
}

std::optional<std::pair<Scene::SceneInfo, SceneOutput> > TitleMenu::update(SDL_Renderer *renderer, int screenWidth, int screenHeight, const InputData &userInputs, unsigned int millis, unsigned int dmillis, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont) {
    gui_->update(userInputs,screenWidth,screenHeight);

    if (transitionIn_ && transitionTimer_ < maxTransitionTimer_) {
        transitionTimer_+=static_cast<int>(std::max(dmillis,static_cast<uint32_t>(100)));
        transitionTimer_ = std::min(transitionTimer_,maxTransitionTimer_);
    }
    if (!transitionIn_ && transitionTimer_ > 0) {
        transitionTimer_-=static_cast<int>(std::max(dmillis,static_cast<uint32_t>(100)));
        transitionTimer_ = std::max(transitionTimer_,0);
    }

    if (userInputs.typingIsActive) {
        //For the time being, there isn't any typing required in the menu, if that changes in the future, add it here

        return std::make_pair(STOP_TYPING, SceneOutput(""));
    }
    else {
        //Update functions for each of the menus:
        if (menuSlides_->getActiveSlide()==splash0Page_) {
            //Start smooth transition out
            if (userInputs.enterPressed && !userInputs.prevEnterPressed) {
                transitionIn_=false;
            }
            if (!transitionIn_ && transitionTimer_==0) {
                transitionIn_=true;
                menuSlides_->setActiveSlide(splash1Page_);
            }
        }
        else if (menuSlides_->getActiveSlide()==splash1Page_) {
            if ( (userInputs.enterPressed && !userInputs.prevEnterPressed) || transitionTimer_==maxTransitionTimer_) {
                transitionIn_=false;
            }
            if (!transitionIn_ && transitionTimer_==0) {
                transitionIn_=true;
                menuSlides_->setActiveSlide(mainMenuPage_);
            }
        }
        else if (menuSlides_->getActiveSlide()==mainMenuPage_) {
            if (quitButton_->isClicked()) {
                return std::make_pair(QUIT_GAME,SceneOutput(""));
            }
            if (playGameButton_->isClicked()) {
                clickSound_->play();
                menuSlides_->setActiveSlide(levelSelectPage_);
            }
            if (settingsButton_->isClicked()) {
                clickSound_->play();
                smallFontSizeButton_->setValue(smallFontSize_);
                midFontSizeButton_->setValue(midFontSize_);
                largeFontSizeButton_->setValue(largeFontSize_);
                menuSlides_->setActiveSlide(settingsPage_);
            }
            if (creditsButton_->isClicked()) {
                clickSound_->play();
                menuSlides_->setActiveSlide(creditsPage_);
                creditsY=creditsTextures_.empty()? 0 : creditsTextures_.front()->getHeight();
            }
        }
        else if (menuSlides_->getActiveSlide()==settingsPage_) {
            if (goBackFromSettingsButton_->isClicked()) {
                smallFontSizeButton_->setValue(smallFontSize_);
                midFontSizeButton_->setValue(midFontSize_);
                largeFontSizeButton_->setValue(largeFontSize_);
                clickSound_->play();
                menuSlides_->setActiveSlide(mainMenuPage_);
            }
            if (toggleParticlesButton_->isClicked()) {
                clickSound_->play();
                enableParticles_=!enableParticles_;
                toggleParticlesTC_->setTexture(std::make_shared<TexWrap>(enableParticles_?"Disable particle effects":"Enable particle effects",renderer,midFont));
            }
            if (soundVolumeControl_->isClicked()) {
                clickSound_->play();
                return std::make_pair(SET_VOLUME,SceneOutput(soundVolumeControl_->getValue()));
            }
            if (musicVolumeControl_->isClicked()) {
                clickSound_->play();
                return std::make_pair(SET_MUSIC_VOLUME,SceneOutput(musicVolumeControl_->getValue()));
            }
            if (applySettingsButton_->isClicked()) {
                saveFontSize();
                return std::make_pair(RELOAD_FONTS,SceneOutput(""));
            }
            if (smallFontSizeButton_->isClicked()) {
                clickSound_->play();
            }
            if (midFontSizeButton_->isClicked()) {
                clickSound_->play();
            }
            if (largeFontSizeButton_->isClicked()) {
                clickSound_->play();
            }
        }
        else if (menuSlides_->getActiveSlide()==creditsPage_) {
            //Reload credits textures
            if (userInputs.sizeChanged) {
                for (int i = 0; i < creditsText_.size();++i) {
                    creditsTextures_[i]->reset(creditsText_[i],renderer,i==0?largeFont:midFont,screenWidth);
                }
            }
            if (goBackFromCreditsButton_->isClicked()) {
                clickSound_->play();
                menuSlides_->setActiveSlide(mainMenuPage_);
            }
            creditsY+=dmillis/16;
            //Check if credits are out of bounds
            int totalCreditsY=creditsGoHere_->getY0()+creditsGoHere_->getHeight()-creditsY;
            for (const auto &text : creditsTextures_) {
                totalCreditsY+=std::max(text->getHeight(),12);
            }
            //Loop around
            if (totalCreditsY<0) {
                creditsY=0;
            }
        }
        else if (menuSlides_->getActiveSlide()==levelSelectPage_) {
            if (goBackFromLevelSelect_->isClicked()) {
                clickSound_->play();
                menuSlides_->setActiveSlide(mainMenuPage_);
            }

            for (int i = 0; i < levelSelectData_.size();++i) {
                const auto& level = levelSelectData_[i];
                if (level.selectLevelButtons_->isClicked()) {
                    clickSound_->play();
                    gui_->closeAllDialogues();
                    if (!level.unlocked) {
                        auto TC = std::make_shared<textureControl>(std::make_shared<TexWrap>("Not unlocked, win the previous level to unlock",renderer,midFont));
                        gui_->addDialogue("Invalid",TC,screenHeight,renderer,smallFont);
                    }
                    else {
                        auto TC0 = std::make_shared<textureControl>(std::make_shared<TexWrap>(level.description,renderer,smallFont,1024));
                        auto TC1 = std::make_shared<textureControl>(std::make_shared<TexWrap>("Hint, any losses you took in the previous level carries over, if you can't beat this level, try to beat the previous level with fewer losses",renderer,smallFont,1024));

                        auto stack = std::make_shared<stackControl>(stackControl::VERTICAL,std::vector<std::shared_ptr<control>>{TC0,TC1,playSelectedLevelButton_});
                        selectedLevelIndex_=i;
                        gui_->addDialogue("Mission briefing",stack,screenHeight,renderer,smallFont);
                    }
                }
            }
            if (playSelectedLevelButton_->isClicked()) {
                return std::make_pair(START_GAME,SceneOutput(levelSelectData_[selectedLevelIndex_].levelName,selectedLevelIndex_,enableParticles_));
            }
        }
    }

    return std::nullopt;
}

