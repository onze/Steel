#ifndef EDITOR_H
#define EDITOR_H

#include <vector>

#include <OgreString.h>

#include "steeltypes.h"
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

            /// make a command out of a Rocket event
            void ProcessEvent(Rocket::Core::Event& event);
            ///general command processing method. dispatches the work to other process*Commands
            void processCommand(Ogre::String command);
            /// submethod processing ui commands concerning the engine
            void processEngineCommand(std::vector<Ogre::String> command);
            /// submethod processing ui commands concerning levels
            void processLevelCommand(std::vector<Ogre::String> command);

            /// used to reattach the debugger on reload
            virtual void onFileChangeEvent(File *file);
            
            /// create an OgreModel from a mesh file
            Steel::AgentId instanciateFromMeshFile(Steel::File& meshFile, Ogre::Vector3& pos, Ogre::Quaternion& rot);

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
