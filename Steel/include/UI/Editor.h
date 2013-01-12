#ifndef EDITOR_H
#define EDITOR_H

#include <vector>

#include <OIS.h>
#include <OgreString.h>

#include "steeltypes.h"
#include "UI/UIPanel.h"
#include "EditorBrush.h"

namespace Steel
{
    class FileSystemDataSource;
    class Engine;
    class UI;
    class InputManager;
    class Editor:public UIPanel
    {
        private:
        public:
            Editor();
            Editor(const Editor& other);
            virtual ~Editor();
            virtual Editor& operator=(const Editor& other);

            virtual void init(unsigned int width, unsigned int height, Engine *engine, UI *ui, InputManager *inputMan);
            /// called right before the underlying document gets shown
            virtual void onShow();
            /// called right before the underlying document gets hidden
            virtual void onHide();

            /**
             * reads an incomplete model file from the data folder, fills the incomplete parts (i.e.: OgreModel: position),
             * and loads it.
             */
            void loadModelFromFile(File &file);

            /** returns whether the given screen coordinates collide with the child element with given Id.
             * If no Id is given, the hit test is made with the main document.**/
            bool hitTest(int x,int y, Rocket::Core::String childId="body");

            /// make a command out of a Rocket event
            void ProcessEvent(Rocket::Core::Event& event);
            ///general command processing method. dispatches the work to other process*Commands
            void processCommand(Ogre::String command);
            /// submethod processing ui commands concerning the engine
            void processEngineCommand(std::vector<Ogre::String> command);
            /// submethod processing ui commands concerning levels
            void processLevelCommand(std::vector<Ogre::String> command);
            /// submethod processing ui commands concerning options
            void processOptionCommand(std::vector<Ogre::String> command);

            //implements the OIS keyListener and mouseListener, as this allows the UI to pass them on as they arrive.
            bool mouseMoved(const OIS::MouseEvent& evt);
            bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);

            /// used to reattach the debugger on reload
            virtual void onFileChangeEvent(File file);

            /// create an OgreModel from a mesh file
            Steel::AgentId instanciateFromMeshFile(Steel::File& meshFile, Ogre::Vector3& pos, Ogre::Quaternion& rot);

        protected:
            //not owned
            Engine *mEngine;
            UI *mUI;
            InputManager *mInputMan;

            //owned
            /// resources available (for levels, models, BTs, etc)
            FileSystemDataSource *mFSResources;
            File mDataDir;
            /// last active tab in the main editor menu (default to 0)
            int mMenuTabIndex;
            /// handles mouse props wrt mEditMode
            EditorBrush mBrush;
            /// If set to true, will print all events with empty "value" attribute for elements whose "id" attribute is set.
            bool mDebugEvents;
        private:
    };
}
#endif // EDITOR_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
