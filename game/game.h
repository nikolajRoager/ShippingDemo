//
// Created by nikolaj on 7/17/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_GAME_H
#define WHIRLWINDSOFDANGERSKETCH_GAME_H
#include <random>

#include "intelligenceManager.h"
#include "Formation.h"
#include "FormationManager.h"
#include "particle.h"
#include "ship.h"
#include "../NumberRenderer.h"
#include "../scene.h"
#include "../soundWrap.h"
#include "../textureManager.h"
#include "../MIGUI/dropdownMenu.h"
#include "../MIGUI/guiManager.h"
#include "../MIGUI/numberControl.h"
#include "../MIGUI/numberInputControl.h"
#include "../MIGUI/slideControl.h"
#include "../MIGUI/textureControl.h"


class Game : public Scene {
public:
    Game(size_t seed,bool enableParticles,const std::string& levelName,SDL_Renderer* renderer, int screenWidth, int screenHeight, TTF_Font* smallFont, TTF_Font* midFont, TTF_Font* largeFont);
    ~Game() override;

    void render(SDL_Renderer* renderer, int screenWidth, int screenHeight,const InputData& userInputs, unsigned int millis, unsigned int pmillis) const override;
    std::optional<std::pair<SceneInfo,SceneOutput>> update(SDL_Renderer* renderer, int screenWidth, int screenHeight,const InputData& userInputs,  unsigned int millis, unsigned int dmillis, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont) override;
private:
    void updateShipWorldEffects(std::shared_ptr<Ship>& ship, double dt, int screenWidth, int screenHeight);
    bool enableParticles_;
    //Change sound volume based on scale
    void attenuateSounds();
    [[nodiscard]] bool isOnLand(double x, double y) const;

    void setupGui(SDL_Renderer *renderer, int screenWidth, int screenHeight, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont);

    ///Number of meters per ocean chunk width/height
    int chunkSize = 185200;
    ///Location of the camera, in meters
    double mapTopLeftX_, mapTopLeftY_;
    ///What these were before we started moving the camera
    double oldMouseMapX_, oldMouseMapY_;
    bool movingMap_ = false;
    int zoomLevel_ =0;
    ///Scale factor to go from in-game meters to pixels
    double scale_;
    ///How much does the map image need to be scaled up, so 1 meter corresponds to 1 pixel (it will be a lot)
    double mapScaleFactor_;

    //Ships and their formations
    std::vector<std::shared_ptr<Ship>> playerShips_;
    int selectedFormationId_=0;

    std::vector<std::shared_ptr<Ship>> enemyShips_;

    std::vector<std::shared_ptr<Ship>> civilianShips_;

    std::unique_ptr<FormationManager> playerFormations_;
    std::unique_ptr<FormationManager> enemyFormations_;
    std::unique_ptr<FormationManager> civilianFormations_;

    //intel management software for both sides
    std::unique_ptr<IntelligenceManager> friendlyIntelligence_;
    std::unique_ptr<IntelligenceManager> enemyIntelligence_;

    std::unique_ptr<NATOSymbolManager> natoSymbolManager_;

    //For keeping track of mission progress
    struct enemyShipCounter {
        int spawned=0;
        int despawned=0;
        int sunk=0;
        int boarded=0;
        int available() const {return spawned-despawned-sunk-boarded;}
        int defeated() const {return sunk+boarded;}
    };
    std::map<std::string,enemyShipCounter> missionCounter;


    Ship::Speed commandSpeed_ = Ship::CRUISE;


    //Physics
    int timeWarpId_=0;//Start paused
    const std::array<double,6> timeWarpFactors_{
        0,
        1,
        5,
        10,
        50,
        100
    };
    const std::array<std::shared_ptr<TexWrap>,6> timeWarpNames_;
    std::shared_ptr<TexWrap> timeWarpIndicator_;


    std::shared_ptr<const TexWrap> mapTexture_;
    //Collection of bools which are true if this pixel is on land
    std::vector<bool> landMap;
    bool isDay_ = true;
    bool isRain_ = false;

    TextureManager textureManager_;
    NumberRenderer midNumberRenderer_;

    std::unique_ptr<GUIManager> gui_;

    //GUI:
    std::shared_ptr<const TexWrap> plusButtonTexture_;
    std::shared_ptr<const TexWrap> minusButtonTexture_;
    std::shared_ptr<const TexWrap> expandButtonTexture_;

    std::shared_ptr<buttonControl> guideButton_;
    std::shared_ptr<buttonControl> briefingButton_;

    std::shared_ptr<buttonControl> nextGuideButton_;
    std::shared_ptr<buttonControl> prevGuideButton_;

    std::shared_ptr<DropdownMenu> speedControlMenu_;

    std::shared_ptr<numberControl> speedFloorIndicator_;
    std::shared_ptr<numberControl> speedFractIndicator_;
    int speedValue;//Current speed in increments of .1 knot

    std::shared_ptr<const SoundWrap> clickSound_;

    std::shared_ptr<SoundWrap> explosionSound_;

    std::string briefingText_;

    std::vector<std::string> guideTexts_;
    int selectedGuide_=0;

    std::shared_ptr<buttonControl> escButton_;

    //Settings menu
    std::shared_ptr<buttonControl> goBackFromSettingsButton_;
    std::shared_ptr<buttonControl> quitAndLooseButton_;
    std::shared_ptr<buttonControl> reallyQuitButton_;
    std::shared_ptr<buttonControl> dontReallyQuitButton_;
    std::shared_ptr<textureControl> toggleParticlesTC_;
    std::shared_ptr<buttonControl> toggleParticlesButton_;
    std::shared_ptr<NumberInputControl> soundVolumeControl_;
    std::shared_ptr<NumberInputControl> musicVolumeControl_;

    //Mission failed slide
    std::shared_ptr<textureControl> missionFailedReasonTC_;
    std::shared_ptr<buttonControl> quitFromFailButton_;
    const int gameplaySlide_=0;
    const int settingsSlide_=1;
    const int missionFailedSlide_=2;


    //The control which handles all the slides of the menu
    std::shared_ptr<SlideControl> menuSlides_;

    //--PARTICLE EFFECTS --
    std::mt19937 rng_;

    ///Mean number of smoke particles spawned per second when damaged
    double meanSmoke_;
    ParticleTemplate smokeTemplate_;
    std::deque<Particle> smokeParticles_;
    double meanFoam_;
    ParticleTemplate foamTemplate_;
    std::deque<Particle> foamParticles_;
    ParticleTemplate explosionTemplate_;
    std::deque<Particle> explosionParticles_;

};


#endif //WHIRLWINDSOFDANGERSKETCH_GAME_H