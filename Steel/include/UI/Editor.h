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

namespace MyGUI
{
    class DDContainer;
    class DDItemInfo;
    class ItemBox;
    class IBNotifyItemData;
    class Widget;
    class IBDrawItemInfo;
    
    namespace types
    {
        template<typename T>
        struct TCoord;
        
        template<typename T>
        struct TPoint;
    }
    typedef types::TCoord<int> IntCoord;
    typedef types::TPoint<int> IntPoint;
    
    class TreeControlItem;
    class TreeControlItemDecorator;
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
        /// libRocket Editor menu tab index setting name
        static const Ogre::String MENU_TAB_INDEX_SETTING;
        /// MyGUI Editor menu tab label
        static const Ogre::String MENUTAB_ITEMNAME_SETTING;

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

        /// Name of the MyGUIVariable upon the change of which the editor brush intensity gets updated
        static const Ogre::String TERRABRUSH_INTENSITY_MYGUIVAR;
        static const Ogre::String TERRABRUSH_RADIUS_MYGUIVAR;

        /// name of the menuTab control variable updated upon tab changed
        static const Ogre::String MENUTAB_CONTROLNAME_MYGUIVAR;


    public:
        Editor(UI &ui);
        Editor(Steel::Editor const &o);
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
        bool rocketHitTest(int x, int y, Rocket::Core::String childId = "body");

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

        virtual void onSignal(Signal signal, SignalEmitter *const src);

        /// Creates a widget for the item (thge item was added to the ItemBox)
        void MyGUIRequestCreateWidgetItem(MyGUI::ItemBox* _sender, MyGUI::Widget* _item);
        void MyGUIRequestDrawItem(MyGUI::ItemBox* _sender, MyGUI::Widget* _item, const MyGUI::IBDrawItemInfo& _info) ;
        void MyGUIRequestCoordWidgetItem(MyGUI::ItemBox* _sender, MyGUI::IntCoord& _coord, bool _drop);
        
        void MyGUIMouseItemActivate(MyGUI::ItemBox *_sender, size_t _index);
        void MyGUIStartDrag(MyGUI::DDContainer *_sender, const MyGUI::DDItemInfo &_info, bool &_result);
        void MyGUIRequestDrop(MyGUI::DDContainer *_sender, const MyGUI::DDItemInfo &_info, bool &_result);
        void MyGUIDropResult(MyGUI::DDContainer *_sender, const MyGUI::DDItemInfo &_info, bool _result);
        void MyGUINotifyItem(MyGUI::ItemBox *_sender, const MyGUI::IBNotifyItemData &_info);
    private:
        /// make a command out of a Rocket event.
        void processSubmitEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem);
        /// make a command out of a Rocket event.
        void processClickEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem);
        /// make a command out of a Rocket event.
        void processChangeEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem);
        /// make a command out of a Rocket event.
        void processDragDropEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem);
        
        /// Called whenever an item of the resource treeControl is dropped
        void MyGUIResourceTreeItemDropped(MyGUI::TreeControlItemDecorator *sender, MyGUI::TreeControlNode *node, MyGUI::IntPoint const& pos);

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
        struct Signals
        {
            Signal brushIntensityUpdate;
            Signal brushRadiusUpdate;
            Signal menuTabChanged;
        };
        Signals mSignals;
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
        
        /// handles drag and dropping of the MyGUI::TreeControl dedicated to resources
        MyGUI::TreeControlItemDecorator *mResourceTreeControlItemDecorator;

    };
}
#endif // STEEL_EDITOR_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
