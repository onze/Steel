#ifndef STEEL_HUD_H
#define STEEL_HUD_H

#include <OgreFrameListener.h>

#include "UI/UIPanel.h"

namespace Steel
{

    class ConfigFile;
    class UI;
    class Engine;

    class HUD: public UIPanel, Ogre::FrameListener
    {
    public:
        HUD(UI &ui);
    private:
        HUD(UI &ui, Ogre::String const &);
    public:
        HUD(const HUD &other);
        virtual ~HUD();
        virtual HUD &operator=(const HUD &other);

        void init(int width, int height, Engine *engine);

        /// called right before the underlying document gets shown
        virtual void onShow();
        /// called right before the underlying document gets hidden
        virtual void onHide();

        /// called by Ogre once per frame
        bool frameRenderingQueued(const Ogre::FrameEvent &evt);

        void showFPS();
        void hideFPS();

        void saveConfig(ConfigFile &config) const;
        void loadConfig(ConfigFile const &config);
    protected:
        // now owned
        Engine *mEngine;
        //owned
        bool mShowFPS;
    };
}
#endif // STEEL_HUD_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
