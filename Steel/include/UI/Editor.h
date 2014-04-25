#ifndef STEEL_EDITOR_H
#define STEEL_EDITOR_H


#include <OIS.h>
#include <OgreString.h>
// #include <Rocket/Controls.h>

#include "steeltypes.h"
#include "UI/UIPanel.h"
#include "SelectionManager.h"
#include "EngineEventListener.h"
#include "InputSystem/Input.h"
#include "EditorBrush.h"
#include "UI/DebugValueManager.h"

namespace std
{
    template<class _Signature >
    class function;
}

namespace Rocket
{
    namespace Core
    {
        class Element;
    }
    namespace Controls
    {
        class ElementFormControlInput;
    }
}

namespace Steel
{

    class Level;
    class ConfigFile;
    class FileSystemDataSource;
    class UI;
    class InputManager;
    class Engine;

    class Editor: public UIPanel, public SelectionManager::Listener, public EngineEventListener
    {
    private:
        /// Editor menu tab index setting name
        static const Ogre::String MENU_TAB_INDEX_SETTING;

        /// Name of the UI Editor element that contains tags elements.
        static const char *SELECTION_TAGS_INFO_BOX;
        /// Name of the UI Editor element that get user input for tag elements.
        static const char *SELECTIONS_TAG_EDIT_BOX;
        /// Name of the UI Editor element that displays a tag.
        static const char *AGENT_TAG_ITEM_NAME;

        /// Name of the UI Editor element that contains tags elements.
        static const char *SELECTION_PATH_INFO_BOX;
        /// Name of the UI Editor element that get user input for tag elements.
        static const char *SELECTIONS_PATH_EDIT_BOX;


    public:
        Editor(UI &ui);
        Editor(Steel::Editor const& o);
        virtual ~Editor();
        virtual Editor &operator=(const Editor &o);

        void init(unsigned int width, unsigned int height, Steel::Engine *engine = nullptr);
    protected:
        // this init should not be called from outside, since it does not have all necessary parameters. It is called upon UIPanel::reload though.
        virtual void init(unsigned int width, unsigned int height);
    public:
        virtual void shutdown();

        void loadConfig(ConfigFile const &config);
        void saveConfig(ConfigFile &config) const;

        /// called right before the underlying document gets shown
        virtual void onShow();
        /// called right before the underlying document gets hidden
        virtual void onHide();

        /** returns whether the given screen coordinates collide with the child element with given Id.
         * If no Id is given, the hit test is made with the main document.**/
        bool hitTest(int x, int y, Rocket::Core::String childId = "body");

        /// take a rocket event and dispatch it to process*Event
        void ProcessEvent(Rocket::Core::Event &event);
        ///general command processing method. dispatches the work to other process
        bool processCommand(Ogre::String rawCommand, bool verbose = true);
        ///general command processing method. dispatches the work to other process
        bool processCommand(std::vector< Ogre::String > command);
        /// submethod processing ui commands concerning options
        bool processOptionsCommand(std::vector< Ogre::String > command);

        //implements the OIS keyListener and mouseListener, as this allows the UI to pass them on as they arrive.
        bool keyPressed(Input::Code key, Input::Event const &evt);
        bool keyReleased(Input::Code key, Input::Event const &evt);
        bool mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt);
        bool mousePressed(Input::Code button, Input::Event const &evt);
        bool mouseReleased(Input::Code button, Input::Event const &evt);
        bool mouseWheeled(int delta, Input::Event const &evt);

        /// used to reattach the debugger on reload
        virtual void onFileChangeEvent(File file);

        /// create an OgreModel from a mesh file
        Steel::AgentId instanciateFromMeshFile(Steel::File &meshFile, Ogre::Vector3 &pos, Ogre::Quaternion &rot);

        /// SelectionManager::Listener interface
        void onSelectionChanged(Selection &selection);

        /// called when a new level becomes the current level.
        virtual void onLevelSet(Level *level);
        /// called right before a level is unset (becomes not current anymore).
        virtual void onLevelUnset(Level *level);


        /// Shortcut to DebugValueManager::addDebugValue.
        void addDebugValue(const Ogre::String &entryName,
                           DebugValueManager::CallbackFunction callback,
                           float min = .0f, float max = 1.f, float init = -1.f);
        /// Shortcut to DebugValueManager::removeDebugValue.
        void removeDebugValue(Ogre::String const &entryName);

    private:
        /// make a command out of a Rocket event.
        void processSubmitEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem);
        /// make a command out of a Rocket event.
        void processClickEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem);
        /// make a command out of a Rocket event.
        void processChangeEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem);
        /// make a command out of a Rocket event.
        void processDragDropEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem);

        void refreshSelectionTagsWidget();
        void populateSelectionTagsWidget(std::list< Ogre::String > tags);
        void decorateSelectionTagWidgetItem(Rocket::Core::Element *item, const Ogre::String &tagName);

        void refreshSelectionPathWidget();

        void saveMenuTabIndexSetting(ConfigFile &config) const;

        /// Retrieve a libRocket form element element's "value" attribute value.
        Ogre::String getFormControlInputValue(Ogre::String elementId);

        /// Set a libRocket form element element's "value" attribute value.
        Rocket::Controls::ElementFormControlInput *setFormControlInputValue(Ogre::String elementId, Ogre::String value);

        //not owned
        Engine *mEngine;
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
        DebugValueManager mDebugValueMan;

    };
}
#endif // STEEL_EDITOR_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
