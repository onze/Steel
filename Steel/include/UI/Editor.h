#ifndef STEEL_EDITOR_H
#define STEEL_EDITOR_H

#include <vector>
#include <map>
#include <set>

#include <OIS.h>
#include <OgreString.h>

#include "steeltypes.h"
#include "EditorBrush.h"
#include "tools/ConfigFile.h"
#include "UI/UIPanel.h"
#include "SelectionManager.h"
#include "EngineEventListener.h"

namespace Steel
{
    class FileSystemDataSource;
    class UI;
    class InputManager;
    class Engine;
    class Editor: public UIPanel, SelectionManager::Listener, EngineEventListener
    {
        private:
            /// Reference lookup table setting name 
            static const Ogre::String REFERENCE_PATH_LOOKUP_TABLE_SETTING;
            /// Editor menu tab index setting name
            static const Ogre::String MENU_TAB_INDEX_SETTING;
            
            /// Name of the UI Editor element that contains tags elements.
            static const char *SELECTION_TAG_INFO_BOX;
            /// Name of the UI Editor element that displays a tag.
            static const char *AGENT_TAG_ITEM_NAME;
        public:
            Editor();
            Editor(const Editor& other);
            virtual ~Editor();
            virtual Editor& operator=(const Editor& other);

            virtual void init(unsigned int width, unsigned int height, Engine *engine, UI *ui, InputManager *inputMan);
            virtual void shutdown();

            void loadConfig(ConfigFile const &config);
            void saveConfig(ConfigFile &config) const;

            Ogre::Vector3 getDropTargetPosition();
            Ogre::Quaternion getDropTargetRotation();
            Ogre::Vector2 getSlotDropPosition();

            /// called right before the underlying document gets shown
            virtual void onShow();
            /// called right before the underlying document gets hidden
            virtual void onHide();

            /**
             * Fills dynamic fields with values of dynamic queries. Optional aid helps filling dynamic
             * fields involving the owner agent. This method is recursive.
             */
            bool dynamicFillSerialization(Json::Value& node, Steel::AgentId aid = INVALID_ID);

            /**
             * Reads the conf to create the lookup table used to resolve dynamic resource locations in
             * models reference descriptor files.
             */
            void setupReferencePathsLookupTable(Ogre::String const &source);
            /**
             * Replaces references' special $keys by specific values (paths set in the conf).
             * The call is recursive (keys can reference other keys).
             */
            void resolveReferencePaths(Ogre::String const &src, Ogre::String &dst);

            /**
             * Instanciate a model from its serialization.
             * If aid is valid (!=INVALID_ID), the model is attached to that agent. Otherwise, a new agent is created,
             * aid is set to its id, and then the model is attached to it.
             * Returns false if a stopping problem occured.
             */
            bool loadModelFromSerialization(Json::Value &root, AgentId &aid);

            /// Instanciate one or many models from a serialization, and returns the AgentId of the agent that controls it.
            bool loadModelsFromSerializations(Json::Value& root, AgentId &aid);

            /// Instanciate models from a .model_refs file content.
            bool loadModelsReferencesFromSerializations(Json::Value& root, Steel::AgentId& aid);

            /**
             * reads an incomplete terrain slot file from the data folder, fills the incomplete parts (i.e.: terrain position),
             * and instanciate it.
             */
            bool loadTerrainSlotFromSerialization(Json::Value &root);

            /// Finds the agent owning the first OgreModel under the mouse, and returns its id.
            AgentId agentIdUnderMouse();
            /// Finds the agent owning the first OgreModel under the mouse, and returns its model of the given type.
            ModelId modelIdUnderMouse(ModelType mType);

            /** returns whether the given screen coordinates collide with the child element with given Id.
             * If no Id is given, the hit test is made with the main document.**/
            bool hitTest(int x, int y, Rocket::Core::String childId = "body");

            /**
             * Preprocess a resource file (fills dynamic values), and instanciate its description if all requirements are satisfied.
             * - if the file content contains an aid, the resource will be attached to the pointed agent (created if needed).
             * - if the aid argument is not INVALID_ID, the resource will be attached to the pointed agent (created if needed).
             * The aid given as argument has precedence over the one in the serialization.
             * This agentId to which the resource is attached is eventually returned.
             */
            bool instanciateResource(Steel::File& file, AgentId &aid);
            /// Forward call to instanciateResource(Steel::File& file, AgentId &aid), and discards the aid.
            bool instanciateResource(Steel::File& file);

            /// take a rocket event and dispatch it to process*Event
            void ProcessEvent(Rocket::Core::Event& event);
            ///general command processing method. dispatches the work to other process
            void processCommand(Ogre::String rawCommand);
            ///general command processing method. dispatches the work to other process
            void processCommand(std::vector<Ogre::String> command);
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
            
            /// SelectionManager::Listener interface
            void onSelectionChanged(Selection &selection);
            
            /// called when a new level becomes the current level.
            virtual void onLevelSet(Level *level);
            /// called right before a level is unset (becomes not current anymore).
            virtual void onLevelUnset(Level *level);
        private:
            /// If found and set to true in an object of a model serialization, values of the object skip their way through dynamicFillSerialization.
            static const char *DF_CANCEL_DYNAMIC_FILLING_ATTRIBUTE;
            
            /// make a command out of a Rocket event.
            void processSubmitEvent(Rocket::Core::Event& event, Rocket::Core::Element *elem);
            /// make a command out of a Rocket event.
            void processClickEvent(Rocket::Core::Event& event, Rocket::Core::Element *elem);
            /// make a command out of a Rocket event.
            void processChangeEvent(Rocket::Core::Event& event, Rocket::Core::Element *elem);
            /// make a command out of a Rocket event.
            void processDragDropEvent(Rocket::Core::Event& event, Rocket::Core::Element *elem);

            void populateSelectionTagWidget(std::set<Tag> tags);
            void saveMenuTabIndexSetting(ConfigFile &config) const;

            //not owned
            Engine *mEngine;
            UI *mUI;
            InputManager *mInputMan;

            //owned
            /// resources available (for levels, models, BTs, etc)
            FileSystemDataSource *mFSResources;
            File mDataDir;
            /// handles mouse props wrt mEditMode
            EditorBrush mBrush;
            /// If set to true, will print all events with empty "value" attribute for elements whose "id" attribute is set.
            bool mDebugEvents;
            /// true during the dragging of a item from the edior's menu.
            bool mIsDraggingFromMenu;
            /// internal representation of REFERENCE_PATH_LOOKUP_TABLE atribute of the application conf file.
            std::map<Ogre::String, Ogre::String> mReferencePathsLookupTable;
    };
}
#endif // STEEL_EDITOR_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
