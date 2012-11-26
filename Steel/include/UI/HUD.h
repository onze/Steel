#ifndef HUD_H
#define HUD_H

#include "UI/UIPanel.h"

namespace Steel
{
    class HUD:public UIPanel
    {
        public:
            HUD();
        private:
            HUD(Ogre::String){};
        public:
            HUD(const HUD& other);
            virtual ~HUD();
            virtual HUD& operator=(const HUD& other);
    };
}
#endif // HUD_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
