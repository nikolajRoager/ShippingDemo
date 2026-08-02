//
// Created by nikolaj on 6/1/26.
//

#include "app.h"

#include <fstream>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "mainMenu/titleMenu.h"
#include "getAssets.h"
#include "inputData.h"
//#include "Game/game.h"
#include "game/game.h"
#include "nlohmann/json.hpp"

//Little helper for erasing characters in text input mode
void popUtf8Char(std::string& s) {
    if (s.empty()) return;
    do {
        s.pop_back();
    } while (!s.empty() && (s.back() & 0xC0) == 0x80);
}

App::App() {
    window_ = nullptr;
    renderer_ = nullptr;
    smallFont_= nullptr;

    //Fail-safe if I forgot to mark the executable as DPI aware
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");

    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        throw std::runtime_error( "SDL_Init Error: " + std::string( SDL_GetError() ) );
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    windowWidthPx_ = DM.w;
    windowHeightPx_ = DM.h;


    window_ = SDL_CreateWindow( "Whirlwinds of Danger", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidthPx_, windowHeightPx_, SDL_WINDOW_SHOWN | SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    SDL_GetWindowSize(window_, &windowWidthPx_, &windowHeightPx_);
    if( window_ == nullptr)
        throw std::runtime_error( "Window could not be created! SDL Error:" + std::string( SDL_GetError() ) );
    //Create vsynced renderer for window
    renderer_ = SDL_CreateRenderer( window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( renderer_ == nullptr)
    {
        throw std::runtime_error( "Renderer could not be created! SDL Error: "+ std::string(SDL_GetError()) );
    }

    //Initialize renderer color
    SDL_SetRenderDrawColor( renderer_, 0xFF, 0xFF, 0xFF, 0xFF );

    //Initialize PNG loading
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) )
    {
        throw std::runtime_error( "SDL_image could not initialize! SDL Error:" + std::string( SDL_GetError() ) );
    }

    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) {
        throw std::runtime_error( "SDL_mixer could not initialize! SDL Error:" + std::string( Mix_GetError() ) );
    }


    if (TTF_Init()==-1) {
        throw std::runtime_error( "SDL_TTF could not initialize! SDL Error:" + std::string( SDL_GetError() ) );
    }

    //Load font sizes
    {
        std::ifstream userSettingsFile(assetsPath()/"usersettings.json");

        if (!userSettingsFile.is_open()) {
            throw std::runtime_error( "Could not open usersettings.json" );
        }

        nlohmann::json json;
        userSettingsFile >> json;

        smallFontSize_ = json["smallFontSize"].get<int>();
        midFontSize_ = json["midFontSize"].get<int>();
        largeFontSize_ = json["largeFontSize"].get<int>();

        userSettingsFile.close();
    }

    fs::path path = assetsPath()/"AtkinsonHyperlegible-Regular.ttf";
    smallFont_=TTF_OpenFont( path.string().c_str(), smallFontSize_);
    if (smallFont_==nullptr) {
        throw std::runtime_error("Failed to load font "+path.string()+", error "+std::string(TTF_GetError())+"");
    }
    midFont_=TTF_OpenFont( path.string().c_str(), midFontSize_);
    if (midFont_==nullptr) {
        throw std::runtime_error("Failed to load font "+path.string()+", error "+std::string(TTF_GetError())+"");
    }
    largeFont_=TTF_OpenFont( path.string().c_str(), largeFontSize_);
    if (largeFont_==nullptr) {
        throw std::runtime_error("Failed to load font "+path.string()+", error "+std::string(TTF_GetError())+"");
    }
    //Load music
    {
        std::ifstream musicFile(assetsPath()/"music"/"music.json");
        if (!musicFile.is_open()) {
            throw std::runtime_error("Couldn't find music/music.json");
        }
        nlohmann::json musicJsonList;
        musicFile >> musicJsonList;
        for (const auto &entry : musicJsonList) {
            std::string name = entry["name"].get<std::string>();
            music_.emplace_back(assetsPath()/"music"/name,name);
        }
        musicFile.close();
        selectedMusicTrack_=0;

        if (music_.empty())
            throw std::runtime_error("No music file found");
        Mix_PlayMusic(music_[selectedMusicTrack_].getMusic(),0);
    }

    //currentScene_ = new TitleMenu(renderer_,windowWidthPx_,windowHeightPx_,smallFont_,midFont_,largeFont_,smallFontSize_,midFontSize_,largeFontSize_);
    currentScene_=new Game(0,true,"tutorial",renderer_,windowWidthPx_,windowHeightPx_,smallFont_,midFont_,largeFont_);
}

App::~App() {
    std::cout<<"Delete current scene"<<std::endl;
    //Must be called before we destroy the renderer, which it hereby is
    delete currentScene_;

    //Delete music
    std::cout<<"Delete music"<<std::endl;
    music_.clear();

    std::cout<<"Delete renderer"<<std::endl;
    if (renderer_!=nullptr)
        SDL_DestroyRenderer(renderer_);
    std::cout<<"Delete window"<<std::endl;
    if (window_!=nullptr)
        SDL_DestroyWindow(window_);
    std::cout<<"Delete fonts"<<std::endl;
    if (smallFont_!=nullptr)
        TTF_CloseFont(smallFont_);
    if (midFont_!=nullptr)
        TTF_CloseFont(midFont_);
    if (largeFont_!=nullptr)
        TTF_CloseFont(largeFont_);
    std::cout<<"Quit TTF..."<<std::endl;
    TTF_Quit();
    std::cout<<"Quit IMG..."<<std::endl;
    IMG_Quit();
    std::cout<<"Quit SDL"<<std::endl;
    SDL_Quit();
}

void App::run() {
    bool quit = false;

    InputData currentInput;

    uint32_t pmillis=SDL_GetTicks();

    while (!quit) {
        currentInput.scroll=0;

        unsigned int millis = SDL_GetTicks();


        currentInput.sizeChanged=false;

        if (Mix_PlayingMusic()!=1) {
            Mix_PlayMusic(music_[selectedMusicTrack_].getMusic(),0);
        }

        SDL_Event event;
        if (currentInput.typingIsActive) {
            //When typing mode is active, individual keypresses don't light up their currentInput fields
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    std::cout<<"Quitting..."<<std::endl;
                    quit = true;
                }
                if (event.type == SDL_WINDOWEVENT) {
                    switch( event.window.event ) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            windowWidthPx_ = event.window.data1;
                            windowHeightPx_ = event.window.data2;
                            break;
                        default:
                            break;
                    }
                }
                if (event.type == SDL_MOUSEWHEEL) {
                    currentInput.scroll=event.wheel.y;
                }
                if (event.type == SDL_MOUSEMOTION) {
                    SDL_GetMouseState( &currentInput.mouseXPx, &currentInput.mouseYPx );
                }
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        currentInput.leftMouseDown=true;
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) {
                        currentInput.rightMouseDown=true;
                    }
                }
                if (event.type == SDL_MOUSEBUTTONUP) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        currentInput.leftMouseDown=false;
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) {
                        currentInput.rightMouseDown=false;
                    }
                }
                else if (event.type == SDL_TEXTINPUT) {
                    // e.text.text is a null-terminated UTF-8 string
                    currentInput.typingText += event.text.text;
                    currentInput.typingTextUpdated=true;
                }
                if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        case SDLK_BACKSPACE: popUtf8Char(currentInput.typingText);
                            currentInput.typingTextUpdated=true;
                            break;
                        case SDLK_RETURN:
                        case SDLK_ESCAPE:
                            currentInput.typingIsActive=false;
                            SDL_StopTextInput();
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        else {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    std::cout<<"Quitting..."<<std::endl;
                    quit = true;
                }
                if (event.type == SDL_WINDOWEVENT) {
                    switch( event.window.event ) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            windowWidthPx_ = event.window.data1;
                            windowHeightPx_ = event.window.data2;
                            currentInput.sizeChanged=true;
                            break;
                        default:
                            break;
                    }
                }
                if (event.type == SDL_MOUSEWHEEL) {
                    currentInput.scroll=event.wheel.y;
                }
                if (event.type == SDL_MOUSEMOTION) {
                    SDL_GetMouseState( &currentInput.mouseXPx, &currentInput.mouseYPx );
                }
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        currentInput.leftMouseDown=true;
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) {
                        currentInput.rightMouseDown=true;
                    }
                }
                if (event.type == SDL_MOUSEBUTTONUP) {
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        currentInput.leftMouseDown=false;
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) {
                        currentInput.rightMouseDown=false;
                    }
                }
                if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_LEFT) {
                        currentInput.leftPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_RIGHT) {
                        currentInput.rightPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_UP) {
                        currentInput.upPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_DOWN) {
                        currentInput.downPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_q) {
                        currentInput.qPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_w) {
                        currentInput.wPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_e) {
                        currentInput.ePressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_a) {
                        currentInput.aPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_r) {
                        currentInput.rPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_s) {
                        currentInput.sPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_d) {
                        currentInput.dPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_z) {
                        currentInput.zoomInPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_x) {
                        currentInput.zoomOutPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_n) {
                        currentInput.nPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_m) {
                        currentInput.mPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_v) {
                        currentInput.vPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_g) {
                        currentInput.gPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_HOME) {
                        currentInput.homePressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN) {
                        currentInput.enterPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        currentInput.escapePressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_SPACE) {
                        currentInput.spacePressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_1) {
                        currentInput.onePressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_2) {
                        currentInput.twoPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_3) {
                        currentInput.threePressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_4) {
                        currentInput.fourPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_5) {
                        currentInput.fivePressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_6) {
                        currentInput.sixPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_COMMA) {
                        currentInput.commaPressed=true;
                    }
                    else if (event.key.keysym.sym == SDLK_PERIOD) {
                        currentInput.periodPressed=true;
                    }
                }
                if (event.type == SDL_KEYUP) {
                    if (event.key.keysym.sym == SDLK_LEFT) {
                        currentInput.leftPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_RIGHT) {
                        currentInput.rightPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_UP) {
                        currentInput.upPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_DOWN) {
                        currentInput.downPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_z) {
                        currentInput.zoomInPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_x) {
                        currentInput.zoomOutPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_n) {
                        currentInput.nPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_m) {
                        currentInput.mPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_v) {
                        currentInput.vPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_q) {
                        currentInput.qPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_w) {
                        currentInput.wPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_e) {
                        currentInput.ePressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_a) {
                        currentInput.aPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_r) {
                        currentInput.rPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_s) {
                        currentInput.sPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_d) {
                        currentInput.dPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_g) {
                        currentInput.gPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_HOME) {
                        currentInput.homePressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN) {
                        currentInput.enterPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        currentInput.escapePressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_SPACE) {
                        currentInput.spacePressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_1) {
                        currentInput.onePressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_2) {
                        currentInput.twoPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_3) {
                        currentInput.threePressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_4) {
                        currentInput.fourPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_5) {
                        currentInput.fivePressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_6) {
                        currentInput.sixPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_COMMA) {
                        currentInput.commaPressed=false;
                    }
                    else if (event.key.keysym.sym == SDLK_PERIOD) {
                        currentInput.periodPressed=false;
                    }
                }
            }
            SDL_Keymod modState = SDL_GetModState();
            if (modState & KMOD_SHIFT)
                currentInput.shiftPressed=true;
            else
                currentInput.shiftPressed=false;

            if (modState & KMOD_CTRL)
                currentInput.ctrlPressed=true;
            else
                currentInput.ctrlPressed=false;
        }
        //At day 47 of running, millis overflows, just pretend no time passed.
        Uint32 dmillis = millis>pmillis ? millis - pmillis : 0;
        if (auto transition = currentScene_->update(renderer_,windowWidthPx_,windowHeightPx_,currentInput,millis,dmillis,smallFont_,midFont_,largeFont_)) {
            if (transition->first==Scene::QUIT_GAME) {
                break;
            }
            else if (transition->first==Scene::QUIT_TO_MENU) {

                delete currentScene_;

                selectedMusicTrack_=0;
                std::cout<<"playing "<<selectedMusicTrack_<<std::endl;
                Mix_HaltMusic();
                Mix_PlayMusic(music_[selectedMusicTrack_].getMusic(),0);

                currentScene_ = new TitleMenu(renderer_,windowWidthPx_,windowHeightPx_,smallFont_,midFont_,largeFont_,smallFontSize_,midFontSize_,largeFontSize_);
            }
            else if (transition->first==Scene::RELOAD_FONTS) {
                //Reload fonts
                if (smallFont_!=nullptr)
                    TTF_CloseFont(smallFont_);
                if (midFont_!=nullptr)
                    TTF_CloseFont(midFont_);
                if (largeFont_!=nullptr)
                    TTF_CloseFont(largeFont_);
                //Load font sizes
                {
                    std::ifstream userSettingsFile(assetsPath()/"usersettings.json");

                    if (!userSettingsFile.is_open()) {
                        throw std::runtime_error( "Could not open usersettings.json" );
                    }

                    nlohmann::json json;
                    userSettingsFile >> json;

                    smallFontSize_ = json["smallFontSize"].get<int>();
                    midFontSize_ = json["midFontSize"].get<int>();
                    largeFontSize_ = json["largeFontSize"].get<int>();

                    userSettingsFile.close();
                }

                fs::path path = assetsPath()/"AtkinsonHyperlegible-Regular.ttf";
                smallFont_=TTF_OpenFont( path.string().c_str(), smallFontSize_);
                if (smallFont_==nullptr) {
                    throw std::runtime_error("Failed to load font "+path.string()+", error "+std::string(TTF_GetError())+"");
                }
                midFont_=TTF_OpenFont( path.string().c_str(), midFontSize_);
                if (midFont_==nullptr) {
                    throw std::runtime_error("Failed to load font "+path.string()+", error "+std::string(TTF_GetError())+"");
                }
                largeFont_=TTF_OpenFont( path.string().c_str(), largeFontSize_);
                if (largeFont_==nullptr) {
                    throw std::runtime_error("Failed to load font "+path.string()+", error "+std::string(TTF_GetError())+"");
                }
                //Reload the menu
                delete currentScene_;
                //Make sure to start on the settings page
                currentScene_ = new TitleMenu(renderer_,windowWidthPx_,windowHeightPx_,smallFont_,midFont_,largeFont_,smallFontSize_,midFontSize_,largeFontSize_,true);
            }
            else if (transition->first==Scene::START_GAME) {

                selectedMusicTrack_=0;
                std::cout<<"playing "<<selectedMusicTrack_<<std::endl;
                Mix_HaltMusic();
                Mix_PlayMusic(music_[selectedMusicTrack_].getMusic(),0);

               delete currentScene_;
               currentScene_=new Game(millis,transition->second.boolData_,transition->second.stringData_,renderer_,windowWidthPx_,windowHeightPx_,smallFont_,midFont_,largeFont_);
            }
            else if (transition->first==Scene::START_TYPING) {
                currentInput.typingIsActive=true;
                currentInput.typingText=transition->second.stringData_;
                SDL_StartTextInput();
            }
            else if (transition->first==Scene::STOP_TYPING) {
                currentInput.typingIsActive=false;
                currentInput.typingText="";
                SDL_StopTextInput();
            }
            else if (transition->first==Scene::SET_MUSIC)
            {
                selectedMusicTrack_=transition->second.intData_;
                std::cout<<"playing "<<selectedMusicTrack_<<std::endl;
                Mix_HaltMusic();
                Mix_PlayMusic(music_[selectedMusicTrack_].getMusic(),0);
            }
            else if (transition->first==Scene::SET_VOLUME)
            {
                Mix_Volume(-1,transition->second.intData_*10);
            }
            else if (transition->first==Scene::SET_MUSIC_VOLUME)
            {
                Mix_VolumeMusic(transition->second.intData_*10);
            }
        }

        SDL_SetRenderDrawColor(renderer_, 0x50, 0x50, 0x50, 0x00);
        SDL_RenderClear( renderer_ );

        currentScene_->render(renderer_,windowWidthPx_,windowHeightPx_,currentInput,millis,pmillis);

        SDL_RenderPresent( renderer_ );


        pmillis=millis;

        currentInput.prevLeftMouseDown=currentInput.leftMouseDown;
        currentInput.prevRightMouseDown=currentInput.rightMouseDown;
        currentInput.prevEnterPressed=currentInput.enterPressed;
        currentInput.prevEscapePressed=currentInput.escapePressed;
        currentInput.prevSPressed=currentInput.sPressed;
        currentInput.prevVPressed=currentInput.vPressed;
        currentInput.prevSpacePressed=currentInput.spacePressed;
        currentInput.prevHomePressed=currentInput.homePressed;
        currentInput.prevQPressed=currentInput.qPressed;
        currentInput.prevWPressed=currentInput.wPressed;
        currentInput.prevEPressed=currentInput.ePressed;
        currentInput.prevAPressed=currentInput.aPressed;
        currentInput.prevSPressed=currentInput.sPressed;
        currentInput.prevDPressed=currentInput.dPressed;
        currentInput.prevGPressed=currentInput.gPressed;
        currentInput.prevOnePressed=currentInput.onePressed;
        currentInput.prevTwoPressed=currentInput.twoPressed;
        currentInput.prevThreePressed=currentInput.threePressed;
        currentInput.prevFourPressed=currentInput.fourPressed;
        currentInput.prevFivePressed=currentInput.fivePressed;
        currentInput.prevSixPressed=currentInput.sixPressed;
        currentInput.prevSixPressed=currentInput.sixPressed;
        currentInput.prevSixPressed=currentInput.sixPressed;
        currentInput.prevSixPressed=currentInput.sixPressed;
        currentInput.prevCommaPressed=currentInput.commaPressed;
        currentInput.prevPeriodPressed=currentInput.periodPressed;
        currentInput.prevRPressed=currentInput.rPressed;
        currentInput.typingTextUpdated=false;
    }

    std::cout<<"While loop has ended"<<std::endl;
}

