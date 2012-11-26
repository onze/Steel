#include "UI/HUD.h"

namespace Steel
{
    HUD::HUD():UIPanel("hud","ui/hud/default/hud.rml")
    {

    }

    HUD::HUD(const HUD& other)
    {

    }

    HUD::~HUD()
    {

    }

    HUD& HUD::operator=(const HUD& other)
    {
        return *this;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
