#include "UI/HUD.h"

#include <Rocket/Core/Element.h>

#include <Engine.h>
#include <UI/UI.h>
#include <Debug.h>

namespace Steel
{
    HUD::HUD():UIPanel("HUD","data/ui/current/HUD/HUD.rml"),Ogre::FrameListener(),
        mEngine(NULL),mUI(NULL),mShowFPS(false)
    {
#ifdef DEBUG
        mAutoReload=true;
        showFPS();
#endif
    }

    HUD::HUD(const HUD& other)
    {

    }

    HUD::~HUD()
    {
        mEngine=NULL;
        mUI=NULL;
    }

    HUD& HUD::operator=(const HUD& other)
    {
        return *this;
    }

    void HUD::init(int width,int height,Engine *engine, UI *ui)
    {
        Debug::log("HUD::init()").endl();
        UIPanel::init(width,height);
        mEngine=engine;
        mUI=ui;
    }

    bool HUD::frameRenderingQueued(const Ogre::FrameEvent &evt)
    {
        Rocket::Core::Element *elem;
        if(mShowFPS)
        {
            if(mDocument!=NULL)
            {
                elem=mDocument->GetElementById("fps");
                if(elem!=NULL)
                {
                    Ogre::String open="<span style=\"display:inline-block;width:10%;\">";
                    Ogre::String close="</span>";
                    const Engine::Stats steelStats=mEngine->stats();
                    Ogre::RenderTarget::FrameStats ogreStats=mEngine->renderWindow()->getStatistics();
                    Ogre::String rml=open+Ogre::StringConverter::toString(static_cast<int>(ogreStats.avgFPS))+" fps"+close;
                    rml.append("|"+open+" E: "+Ogre::StringConverter::toString(static_cast<int>(steelStats.lastEngineDuration))+close);
                    rml.append("|"+open+" G: "+Ogre::StringConverter::toString(static_cast<int>(steelStats.lastGraphicFrameDuration))+close);
                    rml.append("|"+open+" T: "+Ogre::StringConverter::toString(static_cast<int>(1./steelStats.lastFullFrameDuration*1000.))+close);
                    rml.append(" ms");
                    elem->SetInnerRML(rml.c_str());
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
        mShowFPS=true;

    }

    void HUD::hideFPS()
    {
        if(!mShowFPS)return;
        mShowFPS=false;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
