#ifndef STEEL_EDITOR_H
#define STEEL_EDITOR_H

#include <vector>
#include <map>

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
            
            Ogre::Vector3 getDropTargetPosition();
            Ogre::Quaternion getDropTargetRotation();
            Ogre::Vector2 getSlotDropPosition();
            
            /// called right before the underlying document gets shown
            virtual void onShow();
            /// called right before the underlying document gets hidden
            virtual void onHide();
            
            /// Fills dynamic fields with values of dynamic queries. Optional aid helps filling $contect* fields.
            bool dynamicFillSerialization(Json::Value& root, Steel::AgentId aid=INVALID_ID);

            /**
             * Instanciate a model from its serialization. 
             * If aid is valid (!=INVALID_ID), the model is attached to that agent. Otherwise, a new agent is created,
             * aid is set to its id, and then the model is attached to it.
             * Returns false if a stopping problem occured.
             */
            bool loadModelFromSerialization(Json::Value &root, AgentId &aid);
            
            /// Instanciate one or many models from a serialization, and returns the AgentId of the agent that controls it.
            bool loadModelsFromSerializations(Json::Value& root,AgentId &aid);
            
            /**
             * reads an incomplete terrain slot file from the data folder, fills the incomplete parts (i.e.: terrain position),
             * and instanciate it.
             */
            void loadTerrainSlotFromSerialization(Json::Value &root);
            
            /// Finds the agent owning the first OgreModel under the mouse, and returns its id.
            AgentId agentIdUnderMouse();
            /// Finds the agent owning the first OgreModel under the mouse, and returns its model of the given type.
            ModelId modelIdUnderMouse(ModelType mType);

            /** returns whether the given screen coordinates collide with the child element with given Id.
             * If no Id is given, the hit test is made with the main document.**/
            bool hitTest(int x,int y, Rocket::Core::String childId="body");
            
            /// Preprocess a resource file (fills dynamic values), and instanciate its description if all requirements are satisfied.
            bool instanciateResource(Steel::File& file);

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
            bool keyPressed(const OIS::KeyEvent& evt);
            bool keyReleased(const OIS::KeyEvent& evt);
            bool mouseMoved(const OIS::MouseEvent& evt);
            bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);

            /// used to reattach the debugger on reload
            virtual void onFileChangeEvent(File file);

            /// create an OgreModel from a mesh file
            Steel::AgentId instanciateFromMeshFile(Steel::File& meshFile, Ogre::Vector3& pos, Ogre::Quaternion& rot);
            
            /// Saves a Selection under the given tag.
            void setSelectionTag(const Selection &selection,const Ogre::String &tag);
            /// Set tagged agents as selected
            void setTaggedSelection(const Ogre::String &tag);

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
            /// true during the dragging of a item from the edior's menu.
            bool mIsDraggingFromMenu;
            /// maps tags to set of agents
            std::map<Ogre::String, Selection> mSelectionsTags;
        private:
    };
}
#endif // STEEL_EDITOR_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
