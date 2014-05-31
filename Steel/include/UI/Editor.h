#ifndef STEEL_EDITOR_H
#define STEEL_EDITOR_H


#include <OIS.h>
#include <OgreString.h>

#include "steeltypes.h"
#include "UI/UIPanel.h"
#include "SelectionManager.h"
#include "EngineEventListener.h"
#include "InputSystem/Input.h"
#include "EditorBrush.h"

namespace std
{
    template<class _Signature >
    class function;
}

namespace MyGUI
{
    class DDContainer;
    class DDItemInfo;
    class ItemBox;
    class ListBox;
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

    class ScrollView;
    class TreeControlItem;
    class TreeControlItemDecorator;
    class TextBox;
    class Window;
}

namespace Steel
{

    class Level;
    class ConfigFile;
    class UI;
    class InputManager;
    class Engine;
    class Model;
    class PropertyGridManager;

    class Editor: public UIPanel, public SelectionManager::Listener, public EngineEventListener
    {
    private:
        /// MyGUI Editor menu tab label
        static const Ogre::String MENUTAB_ITEMNAME_SETTING;
        /// MyGUI main window position label
        static const Ogre::String MENU_WINDOW_POSITION_SETTING;

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
        /// Name of the UI Editor element used to show/edit the currently selected agent model.
        static const char *MODEL_SERIALIZATION_EDITBOX;
        /// Name of the agent browser tree control
        static const char *AGENTBROWSER_TREECONTROLL_CHILD;

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

        /// Display debug tags info in the console
        void printTagsInfos();
        /// Display current level's LocationModel's paths in the console
        void printPathsInfos();

        /// used to reattach the debugger on reload
        virtual void onFileChangeEvent(File file);

        /// create an OgreModel from a mesh file
        Steel::AgentId instanciateFromMeshFile(Steel::File &meshFile, Ogre::Vector3 &pos, Ogre::Quaternion &rot);

        /// SelectionManager::Listener interface
        void onSelectionChanged(Selection const&selection);

        /// called when a new level becomes the current level.
        virtual void onLevelSet(Level *level);
        /// called right before a level is unset (becomes not current anymore).
        virtual void onLevelUnset(Level *level);

        virtual void onSignal(Signal signal, SignalEmitter *const src);

    private:
        /// Moves the main window at the position saved in the config file.
        void loadMainWindowPosition();
        /// Called whenever an item of the resource treeControl is dropped
        void MyGUIResourceTreeItemDropped(MyGUI::TreeControlItemDecorator *sender, MyGUI::TreeControlNode *node, MyGUI::IntPoint const &pos);

        /// Creates a MyGUI widget to represent a tag of the currently selected agent.
        void MyGUIRequestCreateSelectionTagItem(MyGUI::ItemBox *_sender, MyGUI::Widget *_item);
        void MyGUIRequestDrawSelectionTagItem(MyGUI::ItemBox *_sender, MyGUI::Widget *_item, const MyGUI::IBDrawItemInfo &_info);
        void MyGUIRequestCoordWidgetItem(MyGUI::ItemBox *_sender, MyGUI::IntCoord &_coord, bool _drag);
        void MyGUISelectionTagItemMouseWheel(MyGUI::Widget *_sender, int _rel);
        /// Synchronizes the tags combobox to the TagManager's available tags
        void updateTagsList();

        /// Synchronizes the paths combobox to the current level's locationModelManager available paths
        void updatePathsList();

        /// triggered when the main panel is moved/resized
        void MyGUIEditorWindowChangeCoord(MyGUI::Window *window);

        void refreshSelectionTagsWidget();
        void populateSelectionTagsWidget(std::list< Ogre::String > tags);
        
        void refreshLevelList();

        void refreshSelectionPathWidget();
        
        /// Called whenever a model is selected in the AgentBrowser.
        void MyGUIAgentBrowserTreeNodeSelected(MyGUI::TreeControl *control, MyGUI::TreeControlNode *modelNode);
        /// Dual of updateCurrentModelFromEditBox. Resets the edit box with the model serailization.
        void setAgentModelPropertyGridTarget();

        //not owned
        Engine *mEngine;
        InputManager *mInputMan;

        //owned
        struct Signals
        {   
            Signal brushIntensityUpdate = INVALID_SIGNAL;
            Signal brushRadiusUpdate = INVALID_SIGNAL;
            Signal menuTabChanged = INVALID_SIGNAL;
            Signal newTagCreated = INVALID_SIGNAL;
            Signal newPathCreated = INVALID_SIGNAL;
            Signal pathDeleted = INVALID_SIGNAL;
        };
        Signals mSignals;

        File mDataDir;
        /// handles mouse props wrt mEditMode
        EditorBrush mBrush;

        /// handles drag and dropping of the MyGUI::TreeControl dedicated to resources
        struct MyGUIWidgets
        {            
            MyGUI::TreeControlItemDecorator *resourceTreeControlItemDecorator = nullptr;
            
            MyGUI::TreeControl *agentItemBrowserTree = nullptr;
            MyGUI::TreeControlItemDecorator *agentBrowserTreeControlItemDecorator = nullptr;
            MyGUI::ScrollView *propertyGrid = nullptr;

            MyGUI::ItemBox *selectionTagCloud = nullptr;
            MyGUI::ComboBox *tagsListComboBox = nullptr;

            MyGUI::TextBox *selectionPathTextBox = nullptr;
            MyGUI::ComboBox *pathsListComboBox = nullptr;

            MyGUI::Window *mainWindow = nullptr;
            MyGUI::ComboBox *levelSelectionCbbox = nullptr;
        };
        MyGUIWidgets mMyGUIWidgets;
        PropertyGridManager *mAgenModelPropertyGridMan = nullptr; // not a MyGUI widget, yet MyGUI related.

    };
}
#endif // STEEL_EDITOR_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
