#ifndef EDITOR_H
#define EDITOR_H
#include <vector>
#include <OgreString.h>
#include "UI/UIPanel.h"

namespace Steel
{
    class Engine;
    class UI;
    class Editor:public UIPanel
    {
        private:
        public:
            Editor();
            Editor(const Editor& other);
            virtual ~Editor();
            virtual Editor& operator=(const Editor& other);

            virtual void init(unsigned int width, unsigned int height, Engine *engine, UI *ui);
            /// called right before the underlying document gets shown
            virtual void onShow();
            /// called right before the underlying document gets hidden
            virtual void onHide();

            /// Rocket events
            void ProcessEvent(Rocket::Core::Event& event);
            /// turns commands from the ui into call to the engine
            void processEngineCommands(std::vector<Ogre::String> command);
    protected:
        //not owned
        Engine *mEngine;
    };
}
#endif // EDITOR_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
