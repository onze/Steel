#ifndef HUD_H
#define HUD_H

#include "UI/UIPanel.h"
#include <OgreFrameListener.h>

namespace Steel
{
    class UI;
    class Engine;
    
    class HUD:public UIPanel,Ogre::FrameListener
    {
        public:
            HUD();
        private:
            HUD(Ogre::String){};
        public:
            HUD(const HUD& other);
            virtual ~HUD();
            virtual HUD& operator=(const HUD& other);
            
            void init(int width, int height, Engine *engine, UI *ui);
            
            
            /// called right before the underlying document gets shown
            virtual void onShow();
            /// called right before the underlying document gets hidden
            virtual void onHide();
            
            /// called by Ogre once per frame 
            bool frameRenderingQueued(const Ogre::FrameEvent &evt);
            
            void showFPS();
            void hideFPS();
    protected:
        // now owned
        Engine *mEngine;
        UI *mUI;
        //owned
        bool mShowFPS;
    };
}
#endif // HUD_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
