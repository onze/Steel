#include "UI/HUD.h"

namespace Steel
{
    HUD::HUD():UIPanel("hud","ui/current/hud/hud.rml")
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
