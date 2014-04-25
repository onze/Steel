#include "UI/HUD.h"

#include <Rocket/Core/Element.h>

#include <Engine.h>
#include <UI/UI.h>
#include <Debug.h>
#include <Level.h>
#include <Camera.h>

namespace Steel
{
    HUD::HUD(UI &ui, Ogre::String const &): UIPanel(ui, "HUD", "data/ui/current/hud/hud.rml"), Ogre::FrameListener(),
        mEngine(nullptr), mShowFPS(false)
    {}

    HUD::HUD(UI &ui): UIPanel(ui, "HUD", "data/ui/current/hud/hud.rml"), Ogre::FrameListener(),
        mEngine(nullptr), mShowFPS(false)
    {
#ifdef DEBUG
        mAutoReload = true;
        showFPS();
#endif
    }

    HUD::HUD(const HUD &o): UIPanel(o)
    {
    }

    HUD::~HUD()
    {
        mEngine = nullptr;
    }

    void HUD::saveConfig(ConfigFile &config) const
    {
    }

    void HUD::loadConfig(ConfigFile const &config)
    {
    }

    HUD &HUD::operator=(const HUD &other)
    {
        return *this;
    }

    void HUD::init(int width, int height, Engine *engine)
    {
        Debug::log("HUD::init()").endl();
        UIPanel::init(width, height);
        mEngine = engine;
    }

    bool HUD::frameRenderingQueued(const Ogre::FrameEvent &evt)
    {
        Rocket::Core::Element *elem;

        if(mShowFPS)
        {
            if(mDocument != nullptr)
            {
                elem = mDocument->GetElementById("fps");

                if(elem != nullptr)
                {
                    Ogre::RenderTarget::FrameStats ogreStats = mEngine->renderWindow()->getStatistics();
                    elem->SetInnerRML(Ogre::StringConverter::toString(static_cast<int>(ogreStats.avgFPS)).c_str());
                }

                elem = mDocument->GetElementById("camPos");

                if(elem != nullptr)
                {
                    auto camPos = mEngine->level()->camera()->camNode()->getPosition();
                    elem->SetInnerRML(Ogre::StringConverter::toString(camPos).c_str());
                }
            }
        }

        return true;
    }

    void HUD::onShow()
    {
        Ogre::Root::getSingletonPtr()->addFrameListener(this);
    }

    void HUD::onHide()
    {
        Ogre::Root::getSingletonPtr()->removeFrameListener(this);
    }

    void HUD::showFPS()
    {
        if(mShowFPS)return;

        mShowFPS = true;

    }

    void HUD::hideFPS()
    {
        if(!mShowFPS)return;

        mShowFPS = false;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
