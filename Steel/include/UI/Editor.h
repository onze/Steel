#ifndef EDITOR_H
#define EDITOR_H

#include "UI/UIPanel.h"

namespace Steel
{
    class Editor:public UIPanel
    {
        public:
            Editor();
        private:
            Editor(Ogre::String) {};
        public:
            Editor(const Editor& other);
            virtual ~Editor();
            virtual Editor& operator=(const Editor& other);
    };
}
#endif // EDITOR_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
