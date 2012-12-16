#ifndef EDITOR_H
#define EDITOR_H

#include <vector>

#include <OgreString.h>

#include "UI/UIPanel.h"

namespace Steel
{
    class FileSystemDataSource;
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
            /// submethod processing ui commands concerning the engine
            void processEngineCommands(std::vector<Ogre::String> command,Rocket::Core::Event *evt=NULL);
            /// submethod processing ui commands concerning levels
            void processLevelCommands(std::vector<Ogre::String> command, Rocket::Core::Event *evt);
            
            // used to reattach the debugger on reload
            virtual void onFileChangeEvent(File *file);
            
        protected:
            //not owned
            Engine *mEngine;
            UI *mUI;
            //owned
            /// resources available (for levels, etc)
            FileSystemDataSource *mFSResources;
            File mDataDir;
    };
}
#endif // EDITOR_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
