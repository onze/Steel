#ifndef EDITOR_H
#define EDITOR_H

#include "UI/UIPanel.h"

namespace Steel
{
    class Editor:public UIPanel
    {
        private:
            Editor(Ogre::String) {};
        public:
            Editor();
            Editor(const Editor& other);
            virtual ~Editor();
            virtual Editor& operator=(const Editor& other);
            
            /// called right before the underlying document gets shown
            virtual void onShow();
            /// called right before the underlying document gets hidden
            virtual void onHide();
            
            void ProcessEvent(Rocket::Core::Event& event);
            void reloadContent();
    };
}
#endif // EDITOR_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
