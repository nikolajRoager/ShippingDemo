//
// Created by nikolaj on 7/31/26.
//

#ifndef WHIRLWINDSOFDANGERSKETCH_FORMATIONMANAGER_H
#define WHIRLWINDSOFDANGERSKETCH_FORMATIONMANAGER_H
#include <memory>
#include <vector>

#include "Formation.h"
#include "../inputData.h"


///A manager for formations, with the option to edit the formations graphically
class FormationManager {
public:
    FormationManager(double desiredDistance,std::vector<std::shared_ptr<Formation>> & formations);

    void removeEmptyFormations();

    ///Render the formation information, should only be called for the player
    void render(SDL_Renderer* renderer, double mapTopLeftX, double mapTopLeftY, double scale) const;
    ///Render the formation GUI overlay
    void renderGUI(SDL_Renderer* renderer, int screenWidth, int screenHeight,const InputData& userInputs) const;
    ///Let the player update the formation graphically, returns true if the mouse is over the UI (and shouldn't be applied to the map below
    bool updateGraphical(int screenWidth, int screenHeight,const InputData& userInputs);
    ///Update the formations
    void update();

    [[nodiscard]] std::shared_ptr<Formation> getSelectedFormation() {return selectedFormation_<formations_.size() && selectedFormation_>=0 ? formations_[selectedFormation_] : nullptr;}
    [[nodiscard]] std::shared_ptr<Ship> getSelectedFlagship() {
        auto formation = getSelectedFormation();
        return formation!=nullptr && !formation->getShips().empty() ? formation->getShips()[0] : nullptr;
    }
private:
    double desiredDistance_;
    ///Create a new formation, and drop the selection of into it
    void createNewFormation();
    ///Move teh selection to this new formation
    void moveToFormation(int newFormation);

    std::vector<std::shared_ptr<Formation>> formations_;
    int selectedFormation_;
    ///We just assume all cards have the same size
    int cardWidth_,cardHeight_;
    int nShips=0;

    bool pickCard_=false;
    int selectedCardFormation_=0;
    int selectedCardShip_=0;
};


#endif //WHIRLWINDSOFDANGERSKETCH_FORMATIONMANAGER_H