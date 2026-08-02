//
// Created by nikolaj on 7/17/26.
//

#include "game.h"

#include <fstream>
#include <SDL2/SDL_image.h>

#include "../getAssets.h"
#include "../MIGUI/emptyControl.h"
#include "../MIGUI/stackControl.h"
#include "../MIGUI/tableControl.h"
#include "../MIGUI/textureControl.h"
#include "../nlohmann/json.hpp"

#define MS_TO_KN 1.943844


Game::Game(size_t seed,bool enableParticles,const std::string& levelName, SDL_Renderer *renderer, int screenWidth, int screenHeight, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont):
rng_(seed),
midNumberRenderer_(0,midFont,renderer),
timeWarpNames_{
    std::make_shared<TexWrap>("Pause",renderer,midFont),
    std::make_shared<TexWrap>("Real-time",renderer,midFont),
    std::make_shared<TexWrap>("x5",renderer,midFont),
    std::make_shared<TexWrap>("x10",renderer,midFont),
    std::make_shared<TexWrap>("x50",renderer,midFont),
    std::make_shared<TexWrap>("x100",renderer,midFont)
},
timeWarpIndicator_(std::make_shared<TexWrap>("Time-warp: ",renderer,midFont))
{
    enableParticles_=enableParticles;
    //First launch the threadpool and start loading everything
    //Using a threadpool is CRAZY overkill for this project, but I always use a threadpool for loading for easy scalability
    std::vector<fs::path> textureRequests
    {
        fs::path("levels")/levelName/"map.png",
        fs::path("NATOSymbols")/"targetIndicator.png",
        fs::path("NATOSymbols")/"velocityIndicator.png",
        fs::path("menu")/"plus.png",
        fs::path("menu")/"minus.png",
        fs::path("menu")/"expand.png",
        fs::path("particles")/"smoke.png",
        fs::path("particles")/"explosion.png",
        fs::path("particles")/"foam.png",
        fs::path("shell.png"),
    };

    NATOSymbolManager::requestTextures(textureRequests);

    std::ifstream levelDataFile(assetsPath()/"levels"/levelName/"data.json");
    if (!levelDataFile.is_open()) {
        throw std::runtime_error("Could open data.json for level "+levelName);
    }
    nlohmann::json levelDataJson;
    levelDataFile >> levelDataJson;
    levelDataFile.close();

    std::ifstream playerDataFile(assetsPath()/"levels"/levelName/"playerForces.json");
    if (!playerDataFile.is_open()) {
        throw std::runtime_error("Could open data.json for level "+levelName);
    }
    nlohmann::json playerDataJson;
    playerDataFile >> playerDataJson;
    playerDataFile.close();


    int nShips=0;
    //Make a first pass through the level data, and the player forces, to load ship texture
    for (const auto &shipJson : playerDataJson) {
        std::string name = shipJson["name"].get<std::string>();

        std::ifstream shipFile (assetsPath()/"ships"/(name+".json"));
        if (!shipFile.is_open()) {
            throw std::runtime_error("Could open ships.json for ship "+name);
        }
        nlohmann::json shipDataJson;
        shipFile >> shipDataJson;
        shipFile.close();
        if (shipDataJson.contains("guns")) {
            for (const auto& gunJson : shipDataJson["guns"]) {
                textureRequests.emplace_back(fs::path("ships")/(gunJson["name"].get<std::string>()+".png"));
            }
        }

        textureRequests.emplace_back(fs::path("ships")/(name+".png"));
        textureRequests.emplace_back(fs::path("ships")/(name+"_card.png"));
        ++nShips;
    }
    //Also make a pass through enemy and civilian ships to load their textures
    for (const auto &formationJson : levelDataJson["enemyFormations"]) {
        for (const auto &shipJson : formationJson["ships"]) {
            std::string name = shipJson["name"].get<std::string>();

            std::ifstream shipFile (assetsPath()/"ships"/(name+".json"));
            if (!shipFile.is_open()) {
                throw std::runtime_error("Could open ships.json for ship "+name);
            }
            nlohmann::json shipDataJson;
            shipFile >> shipDataJson;
            shipFile.close();
            if (shipDataJson.contains("guns")) {
                for (const auto& gunJson : shipDataJson["guns"]) {
                    textureRequests.emplace_back(fs::path("ships")/(gunJson["name"].get<std::string>()+".png"));
                }
            }
            textureRequests.emplace_back(fs::path("ships")/(name+".png"));
        }
    }
    for (const auto &formationJson : levelDataJson["civilianFormations"]) {
        for (const auto &shipJson : formationJson["ships"]) {
            std::string name = shipJson["name"].get<std::string>();

            std::ifstream shipFile (assetsPath()/"ships"/(name+".json"));
            if (!shipFile.is_open()) {
                throw std::runtime_error("Could open ships.json for ship "+name);
            }
            nlohmann::json shipDataJson;
            shipFile >> shipDataJson;
            shipFile.close();
            if (shipDataJson.contains("guns")) {
                for (const auto& gunJson : shipDataJson["guns"]) {
                    textureRequests.emplace_back(fs::path("ships")/(gunJson["name"].get<std::string>()+".png"));
                }
            }
            textureRequests.emplace_back(fs::path("ships")/(name+".png"));
        }
    }
    textureRequests.emplace_back(fs::path("ships")/("fallback_card.png"));

    ThreadPool loadingPool(std::thread::hardware_concurrency());
    //We can do other stuff while our textures are loading in the background
    textureManager_.launchTextureLoading(textureRequests, assetsPath(),loadingPool);

    clickSound_=std::make_shared<SoundWrap>(assetsPath()/"sounds"/"click.mp3");
    splashSound_=std::make_shared<SoundWrap>(assetsPath()/"sounds"/"splash.wav");
    artillerySound_=std::make_shared<SoundWrap>(assetsPath()/"sounds"/"artillery.wav");
    explosionSound_=std::make_shared<SoundWrap>(assetsPath()/"sounds"/"explosion.wav");

    //Read mission briefing
    std::ifstream briefingFile(assetsPath()/"levels"/levelName/"briefing.txt");
    if (!briefingFile.is_open()) {
        throw std::runtime_error("Could open briefing.txt");
    }
    briefingText_="";
    std::string line;
    while (std::getline(briefingFile, line)) {
        briefingText_+=line+"\n";
    }
    briefingFile.close();


    //Read game guide
    std::ifstream guideFile(assetsPath()/"guide"/"guide.txt");
    if (!guideFile.is_open()) {
        throw std::runtime_error("Could open guide.txt");
    }

    std::string guideText;
    while (std::getline(guideFile, line)) {
        if (line=="NEWPAGE") {
            //Save the page and reset reader
            guideTexts_.emplace_back(guideText);
            guideText="";
        }
        else
            guideText+=line+"\n";
    }
    guideTexts_.emplace_back(guideText);
    guideFile.close();

    double playerForceX = levelDataJson["playerSpawn"][0].get<double>();
    double playerForceY = levelDataJson["playerSpawn"][1].get<double>();
    chunkSize = levelDataJson["chunkSize"].get<int>();
    isDay_ = levelDataJson["day"].get<bool>();
    isRain_ = levelDataJson["rain"].get<bool>();
    std::deque<glm::dvec2> playerWaypoints;
    for (const auto& waypoints : levelDataJson["playerWaypoints"]) {
        playerWaypoints.emplace_back(waypoints[0].get<double>(),waypoints[1].get<double>());
    }
    if (playerWaypoints.empty()) {
        throw std::runtime_error("No player waypoints");
    }

    mapTopLeftX_=-playerForceX +screenWidth/2;
    mapTopLeftY_=-playerForceY +screenHeight/2;
    oldMouseMapX_=mapTopLeftX_;
    oldMouseMapY_=mapTopLeftY_;
    zoomLevel_ =0;
    scale_=1.0;

    int landMapWidth;
    int landMapHeight;
    {
        fs::path path = assetsPath()/"levels"/levelName/"landmap.png";
        SDL_Surface* loaded = IMG_Load((path.string()).c_str());
        if (!loaded) {
            throw std::runtime_error(std::string("IMG_Load failed to load landmap.png: ") + IMG_GetError());
        }

        // Normalize to a known format regardless of how the PNG was encoded
        // (palette, grayscale, 16-bit, with/without alpha, etc.)
        SDL_Surface* surface = SDL_ConvertSurfaceFormat(loaded, SDL_PIXELFORMAT_RGBA32, 0);
        SDL_FreeSurface(loaded);
        if (!surface) {
            throw std::runtime_error(std::string("SDL_ConvertSurfaceFormat failed: ") + SDL_GetError());
        }

        landMapWidth = surface->w;
        landMapHeight = surface->h;
        landMap.resize(static_cast<size_t>(landMapWidth) * landMapHeight);

        if (SDL_LockSurface(surface) != 0) {
            SDL_FreeSurface(surface);
            throw std::runtime_error(std::string("SDL_LockSurface failed: ") + SDL_GetError());
        }

        const Uint8* pixels = static_cast<const Uint8*>(surface->pixels);
        const int pitch = surface->pitch; // bytes per row, may include padding

        for (int y = 0; y < landMapHeight; ++y) {
            const Uint8* row = pixels + y * pitch;
            for (int x = 0; x < landMapWidth; ++x) {
                const Uint8* p = row + x * 4; // RGBA32 = 4 bytes/pixel
                int brightness = (p[0] + p[1] + p[2]) / 3;
                landMap[static_cast<size_t>(y) * landMapWidth + x] = brightness >= 128;
            }
        }

        SDL_UnlockSurface(surface);
        SDL_FreeSurface(surface);
    }



    {
        std::ifstream smokeFile(assetsPath()/"particles"/"smoke.json");
        if (!smokeFile.is_open()) {
            throw std::runtime_error("Could open smoke.json");
        }
        nlohmann::json smokeJson;
        smokeFile >> smokeJson;
        smokeFile.close();

        meanSmoke_= smokeJson["mean"].get<double>();
        smokeTemplate_.frames_=smokeJson["frames"].get<int>();
        smokeTemplate_.lifeTime_=smokeJson["lifetime"].get<double>();
        if (smokeJson.contains("scale"))
            smokeTemplate_.scale_=smokeJson["scale"].get<double>();
    }

    {
        std::ifstream foamFile(assetsPath()/"particles"/"foam.json");
        if (!foamFile.is_open()) {
            throw std::runtime_error("Could open foam.json");
        }
        nlohmann::json foamJson;
        foamFile >> foamJson;
        foamFile.close();

        meanFoam_ = foamJson["mean"].get<double>();
        foamTemplate_.frames_=foamJson["frames"].get<int>();
        foamTemplate_.lifeTime_=foamJson["lifetime"].get<double>();
        if (foamJson.contains("scale"))
            foamTemplate_.scale_=foamJson["scale"].get<double>();
    }

    {
        std::ifstream explosionFile(assetsPath()/"particles"/"explosion.json");
        if (!explosionFile.is_open()) {
            throw std::runtime_error("Could open explosion.json");
        }
        nlohmann::json explosionJson;
        explosionFile >> explosionJson;
        explosionFile.close();

        explosionTemplate_.frames_=explosionJson["frames"].get<int>();
        explosionTemplate_.lifeTime_=explosionJson["lifetime"].get<double>();
        if (explosionJson.contains("scale"))
            explosionTemplate_.scale_=explosionJson["scale"].get<double>();
    }

    //Coefficient C in the equation radarHorizon = C*(sqrt(own height)+sqrt(target height)), which is a reasonable approximation for radar horizon distance
    double radarHorizonCoefficient = 3500.0;
    //What fraction of radar horizon range can we see in clear weather day
    double clearDayVisionFraction=0.75;
    //What fraction of radar horizon range can we see in clear weather at night
    double clearNightVisionFraction=0.5;
    //What fraction of radar horizon range can we see in rainy weather at day
    double rainyDayVisionFraction=0.5;
    //What fraction of radar horizon range can we see in rainy weather at night
    double rainyNightVisionFraction=0.25;
    //What height target object is used as a reference for the displayed radar horizon range
    double referenceRadarHorizonHeight=30;
    //What is the 4th root of the reference radar cross section we will use to display radar range
    double referenceSqrtSqrtRadarCrossSection=0.4;

    double formationDistance=2000.0;

    ///How much further can ESM see than the radar can see
    double ESMRangeFactor;
    {
        std::ifstream statsFile(assetsPath()/"stats.json");
        if (!statsFile.is_open()) {
            throw std::runtime_error("Could open stats.json");
        }
        nlohmann::json statsJson;
        statsFile >> statsJson;
        statsFile.close();
        formationDistance=statsJson["formationDistance"].get<double>();
        radarHorizonCoefficient=statsJson["radarHorizonCoefficient"].get<double>();
        clearDayVisionFraction=statsJson["clearDayWeatherVisionFraction"].get<double>();
        clearNightVisionFraction=statsJson["clearNightVisionFraction"].get<double>();
        rainyDayVisionFraction=statsJson["rainyDayVisionFraction"].get<double>();
        rainyNightVisionFraction=statsJson["rainyNightVisionFraction"].get<double>();
        referenceRadarHorizonHeight=statsJson["referenceRadarHorizonHeight"].get<double>();
        referenceSqrtSqrtRadarCrossSection=statsJson["sqrtSqrtAShMCrossSection"].get<double>();
        ESMRangeFactor=statsJson["ESMRangeFactor"].get<double>();
    }


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
    smokeTemplate_.texture_=textureManager_.getTexWrap(fs::path("particles")/"smoke.png");
    explosionTemplate_.texture_=textureManager_.getTexWrap(fs::path("particles")/"explosion.png");
    foamTemplate_.texture_=textureManager_.getTexWrap(fs::path("particles")/"foam.png");
    shellTexture_=textureManager_.getTexWrap(fs::path("shell.png"));
    mapTexture_=textureManager_.getTexWrap(fs::path("levels")/levelName/"map.png");
    mapScaleFactor_=static_cast<double>(chunkSize)/static_cast<double>(mapTexture_->getWidth());

    if (landMapHeight!=mapTexture_->getHeight() || landMapWidth!=mapTexture_->getWidth()) {
        throw std::runtime_error("The landmap.png and map.png height and map width mismatch!");
    }

    speedValue=180;


    //Now we can initialize the ships
    //Start with friendly ships

    std::vector<std::shared_ptr<Formation>> playerFormations;
    std::vector<std::shared_ptr<Formation>> enemyFormations;
    std::vector<std::shared_ptr<Formation>> civilianFormations;

    int shipI=0;
    double shipAngle = nShips>1 ? 2.0*M_PI/(nShips-1.0) : 0;
    for (const auto &shipJson : playerDataJson) {
    glm::dvec2 shipPosition=glm::dvec2(playerForceX,playerForceY);
        if (shipI>0) {
            shipPosition+=glm::dvec2(cos(shipI*(shipAngle)),sin(shipI*(shipAngle)))*formationDistance*0.8;
        }
        glm::dvec2 direction = playerWaypoints[0]-shipPosition;
        double heading = atan2(direction.y, direction.x);
        std::string name = shipJson["name"].get<std::string>();

        std::ifstream shipFile (assetsPath()/"ships"/(name+".json"));
        if (!shipFile.is_open()) {
            throw std::runtime_error("Could open ships.json for ship "+name);
        }
        nlohmann::json shipDataJson;
        shipFile >> shipDataJson;
        shipFile.close();

        bool transponderOn = shipDataJson.contains("transponder") && shipDataJson["transponder"].get<bool>();
        double radarCoefficient = shipDataJson["radarCoefficient"].get<double>();
        std::string typeStr = shipDataJson["type"].get<std::string>();
        NATOSymbolManager::ShipType shipType=NATOSymbolManager::CONTAINER_SHIP;
        if (typeStr=="Container Ship")
            shipType = NATOSymbolManager::CONTAINER_SHIP;
        else if (typeStr=="Oil Tanker")
            shipType = NATOSymbolManager::OIL_TANKER;
        else if (typeStr=="Fisher")
            shipType = NATOSymbolManager::FISHER;
        else if (typeStr=="Passenger Ship")
            shipType = NATOSymbolManager::PASSENGER_SHIP;
        else if (typeStr=="Corvette")
            shipType = NATOSymbolManager::CORVETTE;
        else if (typeStr=="Frigate")
            shipType = NATOSymbolManager::FRIGATE;
        else if (typeStr=="Destroyer")
            shipType = NATOSymbolManager::DESTROYER;
        else if (typeStr=="Cruiser")
            shipType = NATOSymbolManager::CRUISER;
        else if (typeStr=="Carrier")
            shipType = NATOSymbolManager::CARRIER;
        else
            throw std::runtime_error("Unknown ship type: " + typeStr);

        std::vector<Ship::Gun> guns;

        if (shipDataJson.contains("guns")) {
            for (const auto& gunJson : shipDataJson["guns"]) {

                guns.emplace_back(gunJson["location"].get<double>(),gunJson["omega"].get<double>(),gunJson["restAngle"].get<double>(),gunJson["reloadTime"].get<double>(),gunJson["seaRange"].get<double>(),gunJson["airRange"].get<double>(),gunJson["muzzleVelocity"].get<double>(),gunJson["shellsPerBurst"].get<int>(),gunJson["burstReloadTime"].get<double>(),textureManager_.getTexWrap(fs::path("ships")/(gunJson["name"].get<std::string>()+".png")));
            }
        }

        int shells = shipDataJson.contains("shells")? shipDataJson["shells"].get<int>():0;
        int HAShMs = shipDataJson.contains("HAShM")? shipDataJson["HAShM"].get<int>():0;
        int AShMs = shipDataJson.contains("AShM")? shipDataJson["AShM"].get<int>():0;
        int SAMs = shipDataJson.contains("SAM")? shipDataJson["SAM"].get<int>():0;

        playerShips_.emplace_back(std::make_shared<Ship>(shipDataJson["className"].get<std::string>(),Ship::FRIEND,shipType,guns,shipDataJson["health"].get<double>(),shipDataJson["maxSpeed"].get<double>(),shipDataJson["cruiseSpeed"].get<double>(),shipDataJson["mass"].get<double>(),shipDataJson["length"].get<double>(),shipDataJson["height"].get<double>(),shipPosition,heading,transponderOn,radarCoefficient ,shells,HAShMs,AShMs,SAMs,
        textureManager_.getTexWrap(fs::path("ships")/(name+".png")),
        textureManager_.getTexWrap(fs::path("ships")/(name+"_card.png")),
        textureManager_.getTexWrap(fs::path("NATOSymbols")/"targetIndicator.png"),
        textureManager_.getTexWrap(fs::path("NATOSymbols")/"velocityIndicator.png"),
        renderer,smallFont
        ));
        ++shipI;
    }
    playerFormations.push_back(std::make_unique<Formation>(formationDistance,playerShips_,playerWaypoints,false));
    selectedFormationId_=0;
    //Now hostile ships
    missionCounter.emplace("total",enemyShipCounter());
    for (const auto &formationJson : levelDataJson["enemyFormations"]) {

        std::deque<glm::dvec2> enemyWaypoints;
        for (const auto& waypoints : formationJson["waypoints"]) {
            enemyWaypoints.emplace_back(waypoints[0].get<double>(),waypoints[1].get<double>());
        }
        if (enemyWaypoints.empty()) {
            throw std::runtime_error("Formation had no waypoints");
        }

        shipI=0;
        nShips = formationJson["ships"].size();
        bool radarOn = formationJson["radarOn"].get<bool>();
        shipAngle = nShips>1 ? 2.0*M_PI/(nShips-1.0) : 0;
        glm::dvec2 formationPosition = glm::dvec2(formationJson["spawn"][0].get<double>(),formationJson["spawn"][1].get<double>());

        std::vector<std::shared_ptr<Ship>> formationShips;
        for (const auto &shipJson : formationJson["ships"]) {
            glm::dvec2 shipPosition=formationPosition;
            if (shipI>0) {
                shipPosition+=glm::dvec2(cos(shipI*(shipAngle)),sin(shipI*(shipAngle)))*formationDistance*0.8;
            }
            glm::dvec2 direction = enemyWaypoints[0]-shipPosition;
            double heading = atan2(direction.y, direction.x);
            std::string name = shipJson["name"].get<std::string>();

            std::ifstream shipFile (assetsPath()/"ships"/(name+".json"));
            if (!shipFile.is_open()) {
                throw std::runtime_error("Could open ships.json for ship "+name);
            }
            nlohmann::json shipDataJson;
            shipFile >> shipDataJson;
            shipFile.close();

            bool transponderOn = shipDataJson.contains("transponder") && shipDataJson["transponder"].get<bool>();
            double radarCoefficient = shipDataJson["radarCoefficient"].get<double>();

            std::string typeStr = shipDataJson["type"].get<std::string>();
            NATOSymbolManager::ShipType shipType=NATOSymbolManager::CONTAINER_SHIP;
            if (typeStr=="Container Ship")
                shipType = NATOSymbolManager::CONTAINER_SHIP;
            else if (typeStr=="Oil Tanker")
                shipType = NATOSymbolManager::OIL_TANKER;
            else if (typeStr=="Fisher")
                shipType = NATOSymbolManager::FISHER;
            else if (typeStr=="Passenger Ship")
                shipType = NATOSymbolManager::PASSENGER_SHIP;
            else if (typeStr=="Corvette")
                shipType = NATOSymbolManager::CORVETTE;
            else if (typeStr=="Frigate")
                shipType = NATOSymbolManager::FRIGATE;
            else if (typeStr=="Destroyer")
                shipType = NATOSymbolManager::DESTROYER;
            else if (typeStr=="Cruiser")
                shipType = NATOSymbolManager::CRUISER;
            else if (typeStr=="Carrier")
                shipType = NATOSymbolManager::CARRIER;
            else
                throw std::runtime_error("Unknown ship type: " + typeStr);

            std::vector<Ship::Gun> guns;

            if (shipDataJson.contains("guns")) {
                for (const auto& gunJson : shipDataJson["guns"]) {
                    guns.emplace_back(gunJson["location"].get<double>(),gunJson["omega"].get<double>(),gunJson["restAngle"].get<double>(),gunJson["reloadTime"].get<double>(),gunJson["seaRange"].get<double>(),gunJson["airRange"].get<double>(),gunJson["muzzleVelocity"].get<double>(),gunJson["shellsPerBurst"].get<int>(),gunJson["burstReloadTime"].get<double>(),textureManager_.getTexWrap(fs::path("ships")/(gunJson["name"].get<std::string>()+".png")));
                }
            }

            int shells = shipDataJson.contains("shells")? shipDataJson["shells"].get<int>():0;
            int HAShMs = shipDataJson.contains("HAShM")? shipDataJson["HAShM"].get<int>():0;
            int AShMs = shipDataJson.contains("AShM")? shipDataJson["AShM"].get<int>():0;
            int SAMs = shipDataJson.contains("SAM")? shipDataJson["SAM"].get<int>():0;

            auto shipPtr = std::make_shared<Ship>(shipDataJson["className"].get<std::string>(),Ship::FOE,shipType,guns,shipDataJson["health"].get<double>(),shipDataJson["maxSpeed"].get<double>(),shipDataJson["cruiseSpeed"].get<double>(),shipDataJson["mass"].get<double>(),shipDataJson["length"].get<double>(),shipDataJson["height"].get<double>(),shipPosition,heading,transponderOn,radarCoefficient,shells,HAShMs,AShMs,SAMs,
            textureManager_.getTexWrap(fs::path("ships")/(name+".png")),
            textureManager_.getTexWrap(fs::path("ships")/("fallback_card.png")),
            textureManager_.getTexWrap(fs::path("NATOSymbols")/"targetIndicator.png"),
            textureManager_.getTexWrap(fs::path("NATOSymbols")/"velocityIndicator.png"),
            renderer,smallFont
            );
            formationShips.emplace_back(shipPtr);
            enemyShips_.emplace_back(shipPtr);
            ++shipI;

            missionCounter["total"].spawned++;
            if (!missionCounter.contains(name)) {
                missionCounter.emplace(name, enemyShipCounter());
            }
            missionCounter[name].spawned++;
        }
        enemyFormations.emplace_back(std::make_unique<Formation>(formationDistance,formationShips,enemyWaypoints,radarOn));
    }
    //Finally neutral ships
    for (const auto &formationJson : levelDataJson["civilianFormations"]) {

        std::deque<glm::dvec2> neutralWaypoints;
        for (const auto& waypoints : formationJson["waypoints"]) {
            neutralWaypoints.emplace_back(waypoints[0].get<double>(),waypoints[1].get<double>());
        }
        if (neutralWaypoints.empty()) {
            throw std::runtime_error("Formation had no waypoints");
        }

        shipI=0;
        nShips = formationJson["ships"].size();
        bool radarOn = formationJson["radarOn"].get<bool>();
        shipAngle = nShips>1 ? 2.0*M_PI/(nShips-1.0) : 0;
        glm::dvec2 formationPosition = glm::dvec2(formationJson["spawn"][0].get<double>(),formationJson["spawn"][1].get<double>());

        std::vector<std::shared_ptr<Ship>> formationShips;
        for (const auto &shipJson : formationJson["ships"]) {
            glm::dvec2 shipPosition=formationPosition;
            if (shipI>0) {
                shipPosition+=glm::dvec2(cos(shipI*(shipAngle)),sin(shipI*(shipAngle)))*formationDistance*0.8;
            }
            glm::dvec2 direction = neutralWaypoints[0]-shipPosition;
            double heading = atan2(direction.y, direction.x);
            std::string name = shipJson["name"].get<std::string>();

            std::ifstream shipFile (assetsPath()/"ships"/(name+".json"));
            if (!shipFile.is_open()) {
                throw std::runtime_error("Could open ships.json for ship "+name);
            }
            nlohmann::json shipDataJson;
            shipFile >> shipDataJson;
            shipFile.close();

            bool transponderOn = shipJson.contains("transponder") && shipJson["transponder"].get<bool>();
            double radarCoefficient = shipDataJson["radarCoefficient"].get<double>();

            std::string typeStr = shipDataJson["type"].get<std::string>();
            NATOSymbolManager::ShipType shipType=NATOSymbolManager::CONTAINER_SHIP;
            if (typeStr=="Container Ship")
                shipType = NATOSymbolManager::CONTAINER_SHIP;
            else if (typeStr=="Oil Tanker")
                shipType = NATOSymbolManager::OIL_TANKER;
            else if (typeStr=="Fisher")
                shipType = NATOSymbolManager::FISHER;
            else if (typeStr=="Passenger Ship")
                shipType = NATOSymbolManager::PASSENGER_SHIP;
            else if (typeStr=="Corvette")
                shipType = NATOSymbolManager::CORVETTE;
            else if (typeStr=="Frigate")
                shipType = NATOSymbolManager::FRIGATE;
            else if (typeStr=="Destroyer")
                shipType = NATOSymbolManager::DESTROYER;
            else if (typeStr=="Cruiser")
                shipType = NATOSymbolManager::CRUISER;
            else if (typeStr=="Carrier")
                shipType = NATOSymbolManager::CARRIER;
            else
                throw std::runtime_error("Unknown ship type: " + typeStr);

            std::vector<Ship::Gun> guns;

            if (shipDataJson.contains("guns")) {
                for (const auto& gunJson : shipDataJson["guns"]) {
                    guns.emplace_back(gunJson["location"].get<double>(),gunJson["omega"].get<double>(),gunJson["restAngle"].get<double>(),gunJson["reloadTime"].get<double>(),gunJson["seaRange"].get<double>(),gunJson["airRange"].get<double>(),gunJson["muzzleVelocity"].get<double>(),gunJson["shellsPerBurst"].get<int>(),gunJson["burstReloadTime"].get<double>(),textureManager_.getTexWrap(fs::path("ships")/(gunJson["name"].get<std::string>()+".png")));
                }
            }

            int shells = shipDataJson.contains("shells")? shipDataJson["shells"].get<int>():0;
            int HAShMs = shipDataJson.contains("HAShM")? shipDataJson["HAShM"].get<int>():0;
            int AShMs = shipDataJson.contains("AShM")? shipDataJson["AShM"].get<int>():0;
            int SAMs = shipDataJson.contains("SAM")? shipDataJson["SAM"].get<int>():0;

            auto shipPtr = std::make_shared<Ship>(shipDataJson["className"].get<std::string>(),Ship::NEUTRAL,shipType,guns,shipDataJson["health"].get<double>(),shipDataJson["maxSpeed"].get<double>(),shipDataJson["cruiseSpeed"].get<double>(),shipDataJson["mass"].get<double>(),shipDataJson["length"].get<double>(),shipDataJson["height"].get<double>(),shipPosition,heading,transponderOn,radarCoefficient,shells,HAShMs,AShMs,SAMs,
            textureManager_.getTexWrap(fs::path("ships")/(name+".png")),
            textureManager_.getTexWrap(fs::path("ships")/("fallback_card.png")),
            textureManager_.getTexWrap(fs::path("NATOSymbols")/"targetIndicator.png"),
            textureManager_.getTexWrap(fs::path("NATOSymbols")/"velocityIndicator.png"),
            renderer,smallFont
            );
            formationShips.emplace_back(shipPtr);
            civilianShips_.emplace_back(shipPtr);
            ++shipI;
        }
        civilianFormations.emplace_back(std::make_unique<Formation>(formationDistance,formationShips,neutralWaypoints,radarOn));
    }

    playerFormations_=std::make_unique<FormationManager>(formationDistance,playerFormations);
    enemyFormations_=std::make_unique<FormationManager>(formationDistance,enemyFormations);
    civilianFormations_=std::make_unique<FormationManager>(formationDistance,civilianFormations);

    auto sinkingText = std::make_shared<TexWrap>("Sinking!",renderer,smallFont);
    auto unknownShipName = std::make_shared<TexWrap>("unidentified ship",renderer,smallFont);
    auto transponderText= std::make_shared<TexWrap>("Transponder",renderer,smallFont);
    auto radarText= std::make_shared<TexWrap>("Radar echo: ",renderer,smallFont);
    auto esmText= std::make_shared<TexWrap>("Emmision",renderer,smallFont);
    auto visionText= std::make_shared<TexWrap>("Visual",renderer,smallFont);
    std::vector<std::shared_ptr<const TexWrap>> sizeTexts{
        std::make_shared<TexWrap>("Tiny",renderer,smallFont),
        std::make_shared<TexWrap>("Very Small",renderer,smallFont),
        std::make_shared<TexWrap>("Small",renderer,smallFont),
        std::make_shared<TexWrap>("Medium Small",renderer,smallFont),
        std::make_shared<TexWrap>("Medium",renderer,smallFont),
        std::make_shared<TexWrap>("Large",renderer,smallFont),
        std::make_shared<TexWrap>("Very Large",renderer,smallFont),
        std::make_shared<TexWrap>("Huge",renderer,smallFont),
    };
    friendlyIntelligence_=std::make_unique<IntelligenceManager>(
        unknownShipName ,
        sinkingText,
        transponderText ,
        visionText ,
        radarText ,
        esmText ,
        sizeTexts,
         enemyShips_.size(),civilianShips_.size(),
         radarHorizonCoefficient,
         clearDayVisionFraction,
         clearNightVisionFraction,
         rainyDayVisionFraction,
         rainyNightVisionFraction,
         referenceRadarHorizonHeight,
         referenceSqrtSqrtRadarCrossSection,
         ESMRangeFactor
    );

    enemyIntelligence_=std::make_unique<IntelligenceManager>(
        unknownShipName ,
        sinkingText ,
        transponderText ,
        visionText ,
        radarText ,
        esmText ,
        sizeTexts,
         playerShips_.size(),civilianShips_.size(),
         radarHorizonCoefficient,
         clearDayVisionFraction,
         clearNightVisionFraction,
         rainyDayVisionFraction,
         rainyNightVisionFraction,
         referenceRadarHorizonHeight,
         referenceSqrtSqrtRadarCrossSection,
         ESMRangeFactor
    );

    natoSymbolManager_=std::make_unique<NATOSymbolManager>(textureManager_);

    setupGui(renderer,screenWidth,screenHeight,smallFont,midFont,largeFont);

    //Open mission briefing
    auto TC = std::make_shared<textureControl>(std::make_shared<TexWrap>(briefingText_,renderer,smallFont,std::max(screenWidth-512,512)));
    gui_->addDialogue("Briefing",TC,screenHeight,renderer,smallFont);

    //One first intel pass, which will be used on the first update
    friendlyIntelligence_->update(playerShips_,enemyShips_,civilianShips_,isDay_,isRain_,true/*This will update whether ships are displayed*/);
    enemyIntelligence_->update(enemyShips_,playerShips_,civilianShips_,isDay_,isRain_,false);
}



void Game::setupGui(SDL_Renderer *renderer, int screenWidth, int screenHeight, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont) {
    //Buttons used by the guide
    auto nextGuideTC = std::make_shared<textureControl>(std::make_shared<TexWrap>("Next Page",renderer,midFont));
    nextGuideButton_ = std::make_shared<buttonControl>(nextGuideTC);
    auto prevGuideTC = std::make_shared<textureControl>(std::make_shared<TexWrap>("Previous Page",renderer,midFont));
    prevGuideButton_ = std::make_shared<buttonControl>(prevGuideTC);



    //The screen we will be playing on
    auto mainScreen = std::make_shared<emptyControl>();

    //---Everything for the bottom bar---
    auto escTC = std::make_shared<textureControl>(std::make_shared<TexWrap>("settings",renderer,midFont));
    escButton_=std::make_shared<buttonControl>(escTC);

    auto guideTC = std::make_shared<textureControl>(std::make_shared<TexWrap>(" ? ",renderer,midFont));
    guideButton_=std::make_shared<buttonControl>(guideTC);

    auto briefingTC = std::make_shared<textureControl>(std::make_shared<TexWrap>("  !  ",renderer,midFont));
    briefingButton_=std::make_shared<buttonControl>(briefingTC);


    auto speed0TC= std::make_shared<textureControl>(std::make_shared<TexWrap>("Stop",renderer,midFont));
    auto speed1TC= std::make_shared<textureControl>(std::make_shared<TexWrap>("Slow",renderer,midFont));
    auto speed2TC= std::make_shared<textureControl>(std::make_shared<TexWrap>("Cruise",renderer,midFont));
    auto speed3TC= std::make_shared<textureControl>(std::make_shared<TexWrap>("Full",renderer,midFont));

    std::vector<std::shared_ptr<control>> speedControls{
    speed0TC,speed1TC,speed2TC,speed3TC};

    speedControlMenu_=std::make_shared<DropdownMenu>(speedControls,expandButtonTexture_);
    speedControlMenu_->setSelection(commandSpeed_);


    auto speedTC = std::make_shared<textureControl>(std::make_shared<TexWrap>(" Speed: ",renderer,midFont));
    speedFloorIndicator_=std::make_shared<numberControl>(midNumberRenderer_,18);
    speedFractIndicator_=std::make_shared<numberControl>(midNumberRenderer_,0);
    auto knotsTC = std::make_shared<textureControl>(std::make_shared<TexWrap>("kn",renderer,midFont));
    auto periodTC= std::make_shared<textureControl>(std::make_shared<TexWrap>(".",renderer,midFont));

    auto shellsTC = std::make_shared<textureControl>(std::make_shared<TexWrap>(" Shells: ",renderer,midFont));
    shellsIndicator_ = std::make_shared<numberControl>(midNumberRenderer_,0);

    auto SAMsTC = std::make_shared<textureControl>(std::make_shared<TexWrap>(" SAMs: ",renderer,midFont));
    SAMsIndicator_ = std::make_shared<numberControl>(midNumberRenderer_,0);

    auto AShMsTC = std::make_shared<textureControl>(std::make_shared<TexWrap>(" AShMs: ",renderer,midFont));
    AShMsIndicator_ = std::make_shared<numberControl>(midNumberRenderer_,0);

    auto HAShMsTC = std::make_shared<textureControl>(std::make_shared<TexWrap>(" HAShMs: ",renderer,midFont));
    HAShMsIndicator_ = std::make_shared<numberControl>(midNumberRenderer_,0);

    //The bar with all the stuff
    auto barStack = std::make_shared<stackControl>(stackControl::HORIZONTAL,std::vector<std::shared_ptr<control>>{escButton_,guideButton_,briefingButton_,speedControlMenu_,speedTC,speedFloorIndicator_,periodTC,speedFractIndicator_,knotsTC,shellsTC,shellsIndicator_,SAMsTC,SAMsIndicator_,AShMsTC,AShMsIndicator_,HAShMsTC,HAShMsIndicator_});

    auto gameplayTable = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight),tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth)},std::vector<std::shared_ptr<control> >{barStack,mainScreen},std::vector<tableControl::background>{tableControl::background(150,150,150),tableControl::background()});


    //---SETTINGS PAGE---
    std::shared_ptr<control> settingsMenu;
    {
        std::shared_ptr<control> nothingRight0=std::make_shared<emptyControl>();

        std::shared_ptr<control> settingsTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Settings",renderer,largeFont));

        std::shared_ptr<control> goBackTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Back",renderer,midFont));
        goBackFromSettingsButton_=std::make_shared<buttonControl>(goBackTC,128,128,128,false);
        toggleParticlesTC_=std::make_shared<textureControl> (std::make_shared<TexWrap>("Disable particle effects",renderer,midFont));
        toggleParticlesButton_=std::make_shared<buttonControl>(toggleParticlesTC_);


        std::shared_ptr<control> quitTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Quit (loose mission)",renderer,midFont));
        quitAndLooseButton_=std::make_shared<buttonControl>(quitTC,128,128,128,false);
        std::shared_ptr<control> reallyQuitTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Quit",renderer,midFont));
        reallyQuitButton_=std::make_shared<buttonControl>(reallyQuitTC,128,128,128,false);
        std::shared_ptr<control> dontReallyQuitTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("No",renderer,midFont));
        dontReallyQuitButton_=std::make_shared<buttonControl>(dontReallyQuitTC,128,128,128,false);


        std::shared_ptr<control> soundTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Sound volume:",renderer,midFont));
        soundVolumeControl_=std::make_shared<NumberInputControl>(midNumberRenderer_,10,0,10,plusButtonTexture_,minusButtonTexture_);

        std::shared_ptr<control> musicTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Music volume:",renderer,midFont));
        musicVolumeControl_=std::make_shared<NumberInputControl>(midNumberRenderer_,10,0,10,plusButtonTexture_,minusButtonTexture_);

        std::shared_ptr<control> soundPair = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2)},std::vector<std::shared_ptr<control>>{soundTC,soundVolumeControl_});
        std::shared_ptr<control> musicPair = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2)},std::vector<std::shared_ptr<control>>{musicTC,musicVolumeControl_});

        std::shared_ptr<stackControl> buttonStack = std::make_shared<stackControl>(stackControl::VERTICAL,std::vector<std::shared_ptr<control> >{settingsTC,soundPair,musicPair,toggleParticlesButton_,goBackFromSettingsButton_,quitAndLooseButton_});
        settingsMenu = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth/2)},std::vector<std::shared_ptr<control> >{buttonStack,nothingRight0});
    }

    std::shared_ptr<control> missionFailedMenu;
    {

        std::shared_ptr<control> nothingRight0=std::make_shared<emptyControl>();

        std::shared_ptr<control> missionFailedTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Mission Failed",renderer,largeFont));
        missionFailedReasonTC_ =std::make_shared<textureControl> (std::make_shared<TexWrap>("Placeholder",renderer,smallFont,512));

        std::shared_ptr<control> quitTC=std::make_shared<textureControl> (std::make_shared<TexWrap>("Main menu",renderer,midFont));
        quitFromFailButton_=std::make_shared<buttonControl>(quitTC,128,128,128,false);

        std::shared_ptr<stackControl> stack = std::make_shared<stackControl>(stackControl::VERTICAL,std::vector<std::shared_ptr<control> >{missionFailedTC,missionFailedReasonTC_,quitFromFailButton_});
        missionFailedMenu= std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth/2),tableControl::rowOrCol(tableControl::rowOrCol::EXPAND,screenWidth/2)},std::vector<std::shared_ptr<control> >{stack,nothingRight0});
    }


    menuSlides_ =std::make_shared<SlideControl>(std::vector<std::shared_ptr<control>>{gameplayTable,settingsMenu,missionFailedMenu});
    menuSlides_->setActiveSlide(gameplaySlide_);
    gui_ = std::make_unique<GUIManager>(menuSlides_ );

}

Game::~Game() = default;

void Game::render(SDL_Renderer *renderer, int screenWidth, int screenHeight, const InputData &userInputs, unsigned int millis, unsigned int pmillis) const {

    mapTexture_->render(mapTopLeftX_,mapTopLeftY_,renderer,scale_*mapScaleFactor_);

    for (auto &foam : foamParticles_) {
        foam.render(renderer,mapTopLeftX_,mapTopLeftY_,screenWidth,screenHeight,scale_);
    }
    for (const auto &ship : playerShips_) {
        ship->render(renderer,mapTopLeftX_,mapTopLeftY_,scale_);
    }
    for (const auto &ship : enemyShips_) {
        ship->render(renderer,mapTopLeftX_,mapTopLeftY_,scale_);
    }
    for (const auto &ship : civilianShips_) {
        ship->render(renderer,mapTopLeftX_,mapTopLeftY_,scale_);
    }
    for (const auto &smoke : smokeParticles_) {
        smoke.render(renderer,mapTopLeftX_,mapTopLeftY_,screenWidth,screenHeight,scale_);
    }
    for (const auto &explosion : explosionParticles_) {
        explosion.render(renderer,mapTopLeftX_,mapTopLeftY_,screenWidth,screenHeight,scale_);
    }

    friendlyIntelligence_->render(playerShips_,enemyShips_,civilianShips_,natoSymbolManager_,renderer,mapTopLeftX_,mapTopLeftY_,scale_,userInputs.mouseXPx,userInputs.mouseYPx,isDay_,isRain_);


    playerFormations_->render(renderer,mapTopLeftX_,mapTopLeftY_,scale_);

    timeWarpIndicator_->render(0,screenHeight-timeWarpIndicator_->getHeight(),255,0,0,renderer);
    timeWarpNames_[timeWarpId_]->render(timeWarpIndicator_->getWidth(),screenHeight-timeWarpIndicator_->getHeight(),255,0,0,renderer);

    playerFormations_->renderGUI(renderer,screenWidth,screenHeight,userInputs);
    gui_->render(renderer,screenWidth,screenHeight);
}

bool Game::isOnLand(double x, double y) const {
    if (x>0 && x<chunkSize && y>0 && y<chunkSize) {
        int pxX = x/mapScaleFactor_;
        int pxY = y/mapScaleFactor_;

        if (pxX>0 && pxX<mapTexture_->getWidth() && pxY>0 && pxY<mapTexture_->getHeight())
            return landMap[pxX+pxY*mapTexture_->getWidth()];
    }
    return false;
}


std::optional<std::pair<Scene::SceneInfo, SceneOutput> > Game::update(SDL_Renderer *renderer, int screenWidth, int screenHeight, const InputData &userInputs, unsigned int millis, unsigned int dmillis, TTF_Font *smallFont, TTF_Font *midFont, TTF_Font *largeFont) {
    //--UPDATE GUI--
    gui_->update(userInputs,screenWidth,screenHeight);


    if (menuSlides_->getActiveSlide()==gameplaySlide_) {
        bool mouseOverFormationGui = playerFormations_->updateGraphical(screenWidth,screenHeight,userInputs);
        if ((userInputs.escapePressed && !userInputs.prevEscapePressed) || escButton_->isClicked() ) {
            clickSound_->play();
            gui_->closeAllDialogues();
            menuSlides_->setActiveSlide(settingsSlide_);
        }
        if (guideButton_->isClicked() || nextGuideButton_->isClicked() || prevGuideButton_->isClicked()) {
            timeWarpId_=0;
            if (prevGuideButton_->isClicked()) {
                selectedGuide_--;
            }
            if (selectedGuide_<0) {
                selectedGuide_=guideTexts_.size()-1;
            }
            if (nextGuideButton_->isClicked()) {
                selectedGuide_=(selectedGuide_+1)%(guideTexts_.size());
            }
            gui_->closeAllDialogues();
            //Open guide on the last page used
            auto TC = std::make_shared<textureControl>(std::make_shared<TexWrap>(guideTexts_[selectedGuide_],renderer,smallFont,std::max(screenWidth-512,512)));

            //The bar with all the stuff
            auto prevNextStack = std::make_shared<stackControl>(stackControl::HORIZONTAL,std::vector<std::shared_ptr<control>>{prevGuideButton_,nextGuideButton_});
            auto upDownTable = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth)},std::vector<std::shared_ptr<control> >{TC,prevNextStack},std::vector<tableControl::background>{tableControl::background(),tableControl::background(150,150,150)});
            gui_->addDialogue("Guide",upDownTable,screenHeight,renderer,smallFont);
            clickSound_->play();
        }
        if (briefingButton_->isClicked()) {
            timeWarpId_=0;
            gui_->closeAllDialogues();
            //Open mission briefing
            auto TC = std::make_shared<textureControl>(std::make_shared<TexWrap>(briefingText_,renderer,smallFont,std::max(screenWidth-512,512)));
            gui_->addDialogue("Briefing",TC,screenHeight,renderer,smallFont);
            clickSound_->play();
        }

        // --- CAMERA CONTROLS ---
        if (userInputs.scroll!=0 && ! gui_->hoverSomething()) {
            zoomLevel_+=userInputs.scroll;
            double newScale =pow(1.1,zoomLevel_);

            //When scaling, the mouse position on the map, mx and my, should remain unchanged, the map top left x/y should be shifted accordingly
            //This should remain unchanged when we move the mouse or scale
            oldMouseMapX_ = (userInputs.mouseXPx-mapTopLeftX_)/scale_;
            oldMouseMapY_ = (userInputs.mouseYPx-mapTopLeftY_)/scale_;

            scale_=newScale;
            attenuateSounds();

            //That way oldMouseMapX_ , oldMouseMapY_ is held constant
            mapTopLeftX_=userInputs.mouseXPx- oldMouseMapX_*scale_;
            mapTopLeftY_=userInputs.mouseYPx- oldMouseMapY_*scale_;
        }

        if (userInputs.leftMouseDown && !userInputs.prevLeftMouseDown && !gui_->hoverSomething() && !mouseOverFormationGui) {
            movingMap_=true;
            //This should remain unchanged when we move the mouse or scale
            oldMouseMapX_ = (userInputs.mouseXPx-mapTopLeftX_)/scale_;
            oldMouseMapY_ = (userInputs.mouseYPx-mapTopLeftY_)/scale_;
        }
        else if (userInputs.leftMouseDown && movingMap_ && !gui_->hoverSomething()) {
            //That way oldMouseMapX_ , oldMouseMapY_ is held constant
            mapTopLeftX_=userInputs.mouseXPx- oldMouseMapX_*scale_;
            mapTopLeftY_=userInputs.mouseYPx- oldMouseMapY_*scale_;
        }
        else {
            movingMap_=false;
        }
        if (userInputs.homePressed && !userInputs.prevHomePressed) {
            auto flagship = playerFormations_->getSelectedFlagship();
            if (flagship!=nullptr) {
                scale_=1.0;
                attenuateSounds();
                zoomLevel_=0;
                mapTopLeftX_=-flagship->getPosition().x+screenWidth/2;
                mapTopLeftY_=-flagship->getPosition().y+screenHeight/2;
            }
        }

        //-- TIME-warp CONTROLS --
        if (userInputs.onePressed && !userInputs.prevOnePressed) {
            timeWarpId_=0;
        }
        if (userInputs.spacePressed && !userInputs.prevSpacePressed) {
            timeWarpId_=0;
        }
        if (userInputs.twoPressed && !userInputs.prevTwoPressed) {
            timeWarpId_=1;
        }
        if (userInputs.threePressed&& !userInputs.prevThreePressed) {
            timeWarpId_=2;
        }
        if (userInputs.fourPressed&& !userInputs.prevFourPressed) {
            timeWarpId_=3;
        }
        if (userInputs.fivePressed&& !userInputs.prevFivePressed) {
            timeWarpId_=4;
        }
        if (userInputs.sixPressed&& !userInputs.prevSixPressed) {
            timeWarpId_=5;
        }
        if (userInputs.enterPressed && !userInputs.prevEnterPressed) {
            timeWarpId_=5;
        }
        if (userInputs.commaPressed && !userInputs.prevCommaPressed) {
            timeWarpId_=std::clamp(timeWarpId_-1,0,5);
        }
        if (userInputs.periodPressed && !userInputs.prevPeriodPressed) {
            timeWarpId_=std::clamp(timeWarpId_+1,0,5);
        }

        // -- PLAYER CONTROLS --
        auto selectedFormation = playerFormations_->getSelectedFormation();
        if (selectedFormation==nullptr) {
            commandSpeed_=Ship::STOP;
            speedControlMenu_->setSelection(commandSpeed_);
        }
        else {
            //Change heading
            if (userInputs.rightMouseDown && !userInputs.prevRightMouseDown && !gui_->hoverSomething() && !mouseOverFormationGui) {
                glm::dvec2 waypoint =glm::dvec2( (userInputs.mouseXPx-mapTopLeftX_)/scale_ ,(userInputs.mouseYPx-mapTopLeftY_)/scale_);
                if (userInputs.shiftPressed)
                    selectedFormation->addWaypoint(waypoint);
                else
                    selectedFormation->overwriteWaypoint(waypoint);
                if (commandSpeed_==Ship::STOP) {
                    commandSpeed_=Ship::CRUISE;
                    speedControlMenu_->setSelection(commandSpeed_);
                    selectedFormation->setSpeed(commandSpeed_);
                }
            }

            if (userInputs.rPressed && !userInputs.prevRPressed) {
                selectedFormation->toggleRadar();
            }
            if (speedControlMenu_->getChangedSelection()) {
                switch (speedControlMenu_->getSelection()) {
                    default:
                    case 0:
                        commandSpeed_=Ship::STOP;
                        break;
                    case 1:
                        commandSpeed_=Ship::SLOW;
                        break;
                    case 2:
                        commandSpeed_=Ship::CRUISE;
                        break;
                    case 3:
                        commandSpeed_=Ship::FULL;
                        break;
                }
                selectedFormation->setSpeed(commandSpeed_);
            }
            if (userInputs.qPressed && !userInputs.prevQPressed && commandSpeed_>Ship::STOP) {
                commandSpeed_=static_cast<Ship::Speed>(commandSpeed_-1);
                speedControlMenu_->setSelection(commandSpeed_);
                selectedFormation->setSpeed(commandSpeed_);
            }
            if (userInputs.wPressed && !userInputs.prevWPressed && commandSpeed_<Ship::FULL) {
                commandSpeed_=static_cast<Ship::Speed>(commandSpeed_+1);
                speedControlMenu_->setSelection(commandSpeed_);
                selectedFormation->setSpeed(commandSpeed_);
            }
            auto newSpeed = selectedFormation->getSpeed();
            if (newSpeed!=commandSpeed_) {
                commandSpeed_=newSpeed;
                speedControlMenu_->setSelection(commandSpeed_);
            }

            if (userInputs.gPressed) {
                selectedFormation->shootAt(glm::dvec2((userInputs.mouseXPx-mapTopLeftX_)/scale_,(userInputs.mouseYPx-mapTopLeftY_)/scale_));
            }
            shellsIndicator_->setValue(selectedFormation->getShells());
            SAMsIndicator_->setValue(selectedFormation->getSAMs());
            AShMsIndicator_->setValue(selectedFormation->getAShMs());
            HAShMsIndicator_->setValue(selectedFormation->getHAShMs());
        }

        //-- PHYSICS AND MANEUVER CALCULATIONS --
        enemyFormations_->update();
        civilianFormations_->update();
        playerFormations_->update();

        //Skip physics calculation if paused
        if (timeWarpId_!=0) {
            //Calculate simulation time-step in seconds, from real-world milliseconds since last frame (dmillis)
            double dt = dmillis*0.001*timeWarpFactors_[timeWarpId_];//The array returns 0,1,5,10,50, or 100

            //TODO, consider upgrading to a threadpool
            for (auto &smoke : smokeParticles_) {
                smoke.update(dt);
            }
            for (auto &foam : foamParticles_) {
                foam.update(dt);
            }
            for (auto &explosion : explosionParticles_) {
                explosion.update(dt);
            }
            for (auto &ship : playerShips_) {
                ship->updateMotion(dt);
            }
            for (auto &ship : enemyShips_) {
                ship->updateMotion(dt);
            }
            for (auto &ship : civilianShips_) {
                ship->updateMotion(dt);
            }
            for (auto &shell : shells_) {
                shell.update(dt);
            }

            //Ship interactions with land, particles, shells or missiles happen on main thread
            for (auto &ship : playerShips_) {
                updateShipWorldEffects(ship,dt,screenWidth,screenHeight);
                ship->updateGuns(dt,artillerySound_,mapTopLeftX_,mapTopLeftY_,scale_,screenWidth,screenHeight,shells_,rng_);
            }
            for (auto &ship : enemyShips_) {
                updateShipWorldEffects(ship,dt,screenWidth,screenHeight);
                ship->updateGuns(dt,artillerySound_,mapTopLeftX_,mapTopLeftY_,scale_,screenWidth,screenHeight,shells_,rng_);
            }
            for (auto &ship : civilianShips_) {
                updateShipWorldEffects(ship,dt,screenWidth,screenHeight);
                ship->updateGuns(dt,artillerySound_,mapTopLeftX_,mapTopLeftY_,scale_,screenWidth,screenHeight,shells_,rng_);
            }

            for (const auto& shell : shells_) {
                if (shell.getTimeToImpact()<=0) {
                    auto pos = shell.getPosition();
                    int screenX =static_cast<int>(pos.x*scale_+mapTopLeftX_);
                    int screenY =static_cast<int>(pos.y*scale_+mapTopLeftY_);

                    //Shell has hit the water or air
                    if (shell.targetSurface()) {
                        bool hit = false;
                        for (const auto& ship :playerShips_) {
                            glm::dvec2 D = pos-ship->getPosition();
                            double D2 = D.x*D.x+D.y*D.y;
                            //Simple radius of impact, is computationally easier
                            if (D2<ship->getLength()*ship->getLength()*0.5) {
                                ship->takeDamage(1,shell.isPlayer());
                                hit=true;
                            }
                        }
                        for (const auto& ship : civilianShips_) {
                            glm::dvec2 D = pos-ship->getPosition();
                            double D2 = D.x*D.x+D.y*D.y;
                            //Simple radius of impact, is computationally easier
                            if (D2<ship->getLength()*ship->getLength()*0.5) {
                                ship->takeDamage(1,shell.isPlayer());
                                hit=true;
                            }
                        }
                        for (const auto& ship : enemyShips_) {
                            glm::dvec2 D = pos-ship->getPosition();
                            double D2 = D.x*D.x+D.y*D.y;
                            //Simple radius of impact, is computationally easier
                            if (D2<ship->getLength()*ship->getLength()*0.5) {
                                ship->takeDamage(1,shell.isPlayer());
                                hit=true;
                            }
                        }
                        //Todo check surfaced submarines

                        if (hit) {
                            explosionSound_->play(screenX,screenY,screenWidth,screenHeight,scale_);
                            explosionParticles_.emplace_back(explosionTemplate_,pos,glm::dvec2(0));
                        }
                        else {
                            splashSound_->play(screenX,screenY,screenWidth,screenHeight,scale_);
                            foamParticles_.emplace_back(foamTemplate_,pos,glm::dvec2(0));
                        }
                    }
                    else {
                        //TODO check if we hit flying targets
                        explosionSound_->play(screenX,screenY,screenWidth,screenHeight,scale_);
                        smokeParticles_.emplace_back(smokeTemplate_,pos,glm::dvec2(0));
                    }
                }
            }
            std::erase_if(shells_,[](const Shell& s){ return s.getTimeToImpact() <= 0; });

            //Update the intelligence management systems
            friendlyIntelligence_->update(playerShips_,enemyShips_,civilianShips_,isDay_,isRain_,true/*This will update whether ships are displayed*/);
            enemyIntelligence_->update(enemyShips_,playerShips_,civilianShips_,isDay_,isRain_,false);

            //If the time-step was too large, slow down the simulation
            if (dt>4.0 && timeWarpId_>1) {
                --timeWarpId_;
            }
        }



        while (!smokeParticles_.empty() &&smokeParticles_.front().isDead()) {
            smokeParticles_.pop_front();
        }
        while (!foamParticles_.empty() &&foamParticles_.front().isDead()) {
            foamParticles_.pop_front();
        }
        while (!explosionParticles_.empty() && explosionParticles_.front().isDead()) {
            explosionParticles_.pop_front();
        }

        int newSpeedValue =0;
        auto flagShip = playerFormations_->getSelectedFlagship();
        if (flagShip !=nullptr) {
            newSpeedValue=static_cast<int>( flagShip->getSpeed()*MS_TO_KN*10);
        }

        if (newSpeedValue!=speedValue) {
            speedValue=newSpeedValue;
            speedFloorIndicator_->setValue(speedValue/10);
            speedFractIndicator_->setValue(speedValue%10);
        }

        //--CHECK MISSION END CONDITIONS--
        int livingPlayerShips =0;
        for (const auto& ship: playerShips_) {
            if (ship->getHealth()>0)
                livingPlayerShips++;
        }
        for (const auto& ship : civilianShips_) {
            if (ship->getHealth()<=0 && ship->sunkByPlayer()) {
                menuSlides_->setActiveSlide(missionFailedSlide_);
                missionFailedReasonTC_->setTexture(std::make_shared<TexWrap>("You sank a civilian ship",renderer,midFont));
                return std::make_pair(SET_MUSIC,SceneOutput(1));
            }
        }
        if (livingPlayerShips==0) {
            menuSlides_->setActiveSlide(missionFailedSlide_);
            missionFailedReasonTC_->setTexture(std::make_shared<TexWrap>("All your ships are sinking",renderer,midFont));
            return std::make_pair(SET_MUSIC,SceneOutput(1));
        }
        else {
            bool anyInside=false;
            for (const auto &ship:playerShips_) {
                auto pos = ship->getPosition();
                if ( !(pos.x<0 || pos.y<0 || pos.x>chunkSize || pos.y>chunkSize)) {
                    anyInside=true;
                    break;
                }
            }
            if (!anyInside) {
                //TODO, in some missions you may win by escaping
                menuSlides_->setActiveSlide(missionFailedSlide_);
                missionFailedReasonTC_->setTexture(std::make_shared<TexWrap>("All your ships have sailed out of the mission",renderer,midFont));
                return std::make_pair(SET_MUSIC,SceneOutput(1));
            }

        }

    }

    else if (menuSlides_->getActiveSlide()==settingsSlide_){
        movingMap_=false;
        if (userInputs.escapePressed && !userInputs.prevEscapePressed) {
            gui_->closeAllDialogues();
            menuSlides_->setActiveSlide(gameplaySlide_);
        }
        if (goBackFromSettingsButton_->isClicked()) {
            clickSound_->play();
            gui_->closeAllDialogues();
            menuSlides_->setActiveSlide(gameplaySlide_);
        }
        if (toggleParticlesButton_->isClicked()) {
            clickSound_->play();
            enableParticles_=!enableParticles_;
            toggleParticlesTC_->setTexture(std::make_shared<TexWrap>(enableParticles_?"Disable particle effects":"Enable particle effects",renderer,midFont));
            if (!enableParticles_) {
                explosionParticles_.clear();
                smokeParticles_.clear();
                foamParticles_.clear();
            }
        }
        if (soundVolumeControl_->isClicked()) {
            clickSound_->play();
            return std::make_pair(SET_VOLUME,SceneOutput(soundVolumeControl_->getValue()));
        }
        if (musicVolumeControl_->isClicked()) {
            clickSound_->play();
            return std::make_pair(SET_MUSIC_VOLUME,SceneOutput(musicVolumeControl_->getValue()));
        }
        if (quitAndLooseButton_->isClicked()) {
            clickSound_->play();

            gui_->closeAllDialogues();
            //Open guide on the last page used
            auto TC = std::make_shared<textureControl>(std::make_shared<TexWrap>("Really quit? progress is NOT saved!",renderer,midFont));

            //The bar with all the stuff
            auto prevNextStack = std::make_shared<stackControl>(stackControl::HORIZONTAL,std::vector<std::shared_ptr<control>>{dontReallyQuitButton_,reallyQuitButton_});
            auto upDownTable = std::make_shared<tableControl>(screenWidth,screenHeight,std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight),tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenHeight)},std::vector<tableControl::rowOrCol>{tableControl::rowOrCol(tableControl::rowOrCol::SHRINK,screenWidth)},std::vector<std::shared_ptr<control> >{TC,prevNextStack},std::vector<tableControl::background>{tableControl::background(),tableControl::background(150,150,150)});
            gui_->addDialogue("Quit",upDownTable,screenHeight,renderer,smallFont);
        }
        if (dontReallyQuitButton_->isClicked()) {
            clickSound_->play();
            gui_->closeAllDialogues();
        }
        if (reallyQuitButton_->isClicked()) {
            return std::make_pair(QUIT_TO_MENU,SceneOutput(""));
        }
    }
    else if (menuSlides_->getActiveSlide()==missionFailedSlide_) {
        if (quitFromFailButton_->isClicked()) {
            return std::make_pair(QUIT_TO_MENU,SceneOutput(""));
        }
    }


    return std::nullopt;
}

void Game::attenuateSounds() {
    double factor = std::clamp((scale_-SOUND_SCALE_LIMIT)/(1-SOUND_SCALE_LIMIT),0.0,1.0);
    explosionSound_->attenuate(factor);
    artillerySound_->attenuate(factor);
}

void Game::updateShipWorldEffects(std::shared_ptr<Ship> &ship, double dt, int screenWidth, int screenHeight) {
        //Used by particles
        glm::dvec2 shipVelocity = ship->getVelocity();
        glm::dvec2 shipParticleOffset = glm::dvec2(cos(ship->getHeading()),sin(ship->getHeading()))*ship->getLength()*0.3;
        glm::dvec2 pos = ship->getPosition();
        if (ship->getHealth()>0 && isOnLand(pos.x,pos.y)) {
            ship->destroy();
            //Grounding halts the ship
            ship->setSpeedExact(0.0);
            //Play explosion sound
            int texX = static_cast<int>(pos.x*scale_+mapTopLeftX_);
            int texY = static_cast<int>(pos.y*scale_+mapTopLeftY_);

            explosionParticles_.emplace_back(explosionTemplate_,pos+shipParticleOffset,shipVelocity);
            explosionParticles_.emplace_back(explosionTemplate_,pos-shipParticleOffset,shipVelocity);
            explosionParticles_.emplace_back(explosionTemplate_,pos,shipVelocity);
            explosionSound_->play(texX, texY,screenWidth,screenHeight,scale_);

        }

        if (enableParticles_ && ship->identified()) {
        int health = ship->getHealth();
        int maxHealth = ship->getMaxHealth();
        if (health<maxHealth) {
            //Spawn smoke, as the ship gets more damaged, up to three pillars of smoke spawn,

            int threeDamage =((maxHealth-health)*3);
            //If damage is between 0 and maxHealth/3, spawn 1 pillar in the front
            //If damage is between maxHealth/3 and 2*maxHealth/3, spawn 2 pillars front and back
            //If damage is above 2*maxHealth/3, spawn 3 pillar front, back and midships


            //Poisson distribution
            double mean = dt*meanSmoke_;
            if (mean>0) {
                std::poisson_distribution<int> dist(dt*meanSmoke_);

                //Midship damage
                if (threeDamage>maxHealth*2) {
                    int nParticles = dist(rng_);
                    for (int i = 0; i < nParticles; i++) {
                        smokeParticles_.emplace_back(smokeTemplate_,pos,shipVelocity);
                    }
                }

                //Stern damage
                if (threeDamage>maxHealth) {
                    int nParticles = dist(rng_);
                    for (int i = 0; i < nParticles; i++) {
                        smokeParticles_.emplace_back(smokeTemplate_,pos-shipParticleOffset ,shipVelocity);
                    }
                }

                //Front damage
                int nParticles = dist(rng_);
                for (int i = 0; i < nParticles; i++) {
                    smokeParticles_.emplace_back(smokeTemplate_,pos+shipParticleOffset ,shipVelocity);
                }
            }

        }

        //Ships always spawn foam when moving
        if (std::abs(shipVelocity.x)>1 || std::abs(shipVelocity.y)>1) {
            //Poisson distribution
            double mean = dt*meanFoam_;
            if (mean>0) {
                std::poisson_distribution<int> dist(mean);
                int nParticles = dist(rng_);
                for (int i = 0; i < nParticles; i++) {
                    foamParticles_.emplace_back(foamTemplate_,pos-shipParticleOffset ,glm::dvec2(0,0));
                }
            }

        }
    }
}
