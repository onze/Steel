#include <algorithm>
#include <json/json.h>

#include "steeltypes.h"
#include <MyGUI.h>

#include "UI/Editor.h"
#include "Camera.h"
#include "Debug.h"
#include "Engine.h"
#include "Level.h"
#include "SelectionManager.h"
#include "SignalManager.h"
#include "TagManager.h"
#include "UI/UI.h"
#include "UI/PropertyGrid/PropertyGridManager.h"
#include "UI/PropertyGrid/PropertyGridAgentAdapter.h"
#include "models/Agent.h"
#include "models/OgreModelManager.h"
#include "models/PhysicsModelManager.h"
#include "models/LocationModelManager.h"
#include "models/AgentManager.h"
#include "tools/3dParties/MyGUI/TreeControlItemDecorator.h"
#include "tools/3dParties/MyGUI/TreeControlItem.h"
#include "tools/3dParties/MyGUI/MyGUIFileTreeDataSource.h"
#include "tools/3dParties/MyGUI/MyGUIAgentBrowserDataSource.h"
#include "tools/JsonUtils.h"
#include "tools/OgreUtils.h"
#include "tools/StringUtils.h"

namespace Steel
{
    const Ogre::String Editor::MENU_WINDOW_POSITION_SETTING = "Editor::windowPosition";
    const Ogre::String Editor::MENUTAB_ITEMNAME_SETTING = "Editor::menuTabItemName";

    const char *Editor::SELECTION_TAGS_INFO_BOX = "selectionTagsInfoBox";
    const char *Editor::SELECTIONS_TAG_EDIT_BOX = "selection_tags_editbox";
    const char *Editor::AGENT_TAG_ITEM_NAME = "agenttagitem";

    const char *Editor::SELECTION_PATH_INFO_BOX = "selectionPathsInfoBox";
    const char *Editor::SELECTIONS_PATH_EDIT_BOX = "selection_path_editbox";
    const char *Editor::AGENTBROWSER_TREECONTROLL_CHILD = "AgentBrowserTreeControl_Child";

    const char *Editor::MODEL_SERIALIZATION_EDITBOX = "ModelSerializationEditBox";

    const Ogre::String Editor::TERRABRUSH_INTENSITY_MYGUIVAR = "terrabrushIntensity";
    const Ogre::String Editor::TERRABRUSH_RADIUS_MYGUIVAR = "terrabrushRadius";
    const Ogre::String Editor::MENUTAB_CONTROLNAME_MYGUIVAR = "editorMenuTab";

    Editor::Editor(UI &ui): UIPanel(ui, "editor", "data/ui/current/editor/"),
        mEngine(nullptr), mInputMan(nullptr),
        mSignals(),
        mDataDir(), mBrush(),
        mMyGUIWidgets(), mAgenModelPropertyGridMan(nullptr)
    {
#ifdef DEBUG
        mAutoReload = true;
#endif
    }

    Editor::Editor(Editor const &o): UIPanel(o)
    {
        Debug::error("Editor::Editor(const Editor& other) not implemented").endl().breakHere();
    }

    Editor::~Editor()
    {
        shutdown();
    }

    Editor &Editor::operator=(Editor const &o)
    {
        Debug::error("Editor::operator=(const Editor& other) not implemented").endl().breakHere();
        return *this;
    }

    void Editor::loadConfig(ConfigFile const &config)
    {
        mBrush.loadConfig(config);

        {
            Ogre::String tabName;

            if(config.getSetting(Editor::MENUTAB_ITEMNAME_SETTING, tabName))
                setMyGUIVariable(Editor::MENUTAB_CONTROLNAME_MYGUIVAR, tabName);

            if(nullptr != mMyGUIWidgets.mainWindow)
            {
                auto coords = mMyGUIWidgets.mainWindow->getPosition();
                Ogre::Vector2 position;

                if(config.getSetting(Editor::MENU_WINDOW_POSITION_SETTING, position, Ogre::Vector2(coords.left, coords.top)))
                    mMyGUIWidgets.mainWindow->setPosition(position.x, position.y);
            }
        }
    }

    void Editor::saveConfig(ConfigFile &config) const
    {
        mBrush.saveConfig(config);
    }

    void Editor::shutdown()
    {
        //MyGUI stuff
        {
            STEEL_DELETE(mMyGUIWidgets.agentBrowserTreeControlItemDecorator);
            STEEL_DELETE(mMyGUIWidgets.resourceTreeControlItemDecorator);

            mMyGUIWidgets.agentItemBrowserTree = nullptr;
            mMyGUIWidgets.selectionTagCloud = nullptr;
            mMyGUIWidgets.tagsListComboBox = nullptr;
            mMyGUIWidgets.selectionPathTextBox = nullptr;
            mMyGUIWidgets.pathsListComboBox = nullptr;
            mMyGUIWidgets.mainWindow = nullptr;
        }

        if(nullptr != mAgenModelPropertyGridMan)
        {
            auto adapter = mAgenModelPropertyGridMan->adapter();
            STEEL_DELETE(mAgenModelPropertyGridMan);
            STEEL_DELETE(adapter);
        }

        mBrush.shutdown();

        if(nullptr != mEngine->level())
        {
            mEngine->level()->selectionMan()->removeListener(this);
        }

        mEngine->removeEngineEventListener(this);
        UIPanel::shutdown();
    }

    void Editor::init(unsigned int width, unsigned int height)
    {
        init(width, height, nullptr);
    }

    void Editor::init(unsigned int width, unsigned int height, Engine *engine)
    {
        Debug::log(STEEL_METH_INTRO).endl();

        if(nullptr != engine)
        {
            mEngine = engine;
            mInputMan = engine->inputMan();
        }

        mDataDir = mUI.UIDataDir().subfile("editor").fullPath();
        auto resGroupMan = Ogre::ResourceGroupManager::getSingletonPtr();
        // true is for recursive search. Add to this resources.cfg
        resGroupMan->addResourceLocation(mDataDir.fullPath(), "FileSystem", "UI", true);

        UIPanel::init(width, height);

        // finalize MYGui setup
        {
            // resource tree
            {
                mMyGUIWidgets.agentItemBrowserTree = (MyGUI::TreeControl *) findMyGUIChildWidget(Editor::AGENTBROWSER_TREECONTROLL_CHILD);

                if(nullptr != mMyGUIWidgets.agentItemBrowserTree)
                {
                    // decoration here means addind drag and drop properties to the tree's items
                    mMyGUIWidgets.agentBrowserTreeControlItemDecorator = new MyGUI::TreeControlItemDecorator(mMyGUIWidgets.agentItemBrowserTree);
                    mMyGUIWidgets.agentBrowserTreeControlItemDecorator->setEnableDragAndDrop(false, false);
                    mMyGUIWidgets.agentBrowserTreeControlItemDecorator->setTextColour(MyGUI::Colour::White);
                    mMyGUIWidgets.agentItemBrowserTree->eventTreeNodeSelected += MyGUI::newDelegate(this, &Editor::MyGUIAgentBrowserTreeNodeSelected);
                }
            }

            // resource tree
            {
                MyGUI::TreeControl *resourceTree = (MyGUI::TreeControl *)findMyGUIChildWidget("ResourceTreeControl_Child");

                if(nullptr != resourceTree)
                {
                    // decoration here means addind drag and drop properties to the tree's items
                    mMyGUIWidgets.resourceTreeControlItemDecorator = new MyGUI::TreeControlItemDecorator(resourceTree);
                    mMyGUIWidgets.resourceTreeControlItemDecorator->setEnableDragAndDrop(false, true);
                    mMyGUIWidgets.resourceTreeControlItemDecorator->eventItemDropped += MyGUI::newDelegate(this, &Editor::MyGUIResourceTreeItemDropped);
                    mMyGUIWidgets.resourceTreeControlItemDecorator->setTextColour(MyGUI::Colour::White);
                }
            }

            // all tags related
            {
                mMyGUIWidgets.selectionTagCloud = (decltype(mMyGUIWidgets.selectionTagCloud))findMyGUIChildWidget("EditorSelectionTagCloud");
                mMyGUIWidgets.selectionTagCloud->requestCreateWidgetItem = MyGUI::newDelegate(this, &Editor::MyGUIRequestCreateSelectionTagItem);
                mMyGUIWidgets.selectionTagCloud->requestDrawItem = MyGUI::newDelegate(this, &Editor::MyGUIRequestDrawSelectionTagItem);
                mMyGUIWidgets.selectionTagCloud->requestCoordItem = MyGUI::newDelegate(this, &Editor::MyGUIRequestCoordWidgetItem);
                mMyGUIWidgets.tagsListComboBox = (decltype(mMyGUIWidgets.tagsListComboBox))findMyGUIChildWidget("TagsListComboBox");
                mMyGUIWidgets.tagsListComboBox->removeAllItems();
            }

            // all path related
            {
                mMyGUIWidgets.selectionPathTextBox = (decltype(mMyGUIWidgets.selectionPathTextBox))findMyGUIChildWidget("EditorSelectionPathTextBox");
                mMyGUIWidgets.pathsListComboBox = (decltype(mMyGUIWidgets.pathsListComboBox))findMyGUIChildWidget("PathsListComboBox");
                mMyGUIWidgets.pathsListComboBox->removeAllItems();
            }

            // other widgets that are useful to keep a handle on for frequent references
            mMyGUIWidgets.mainWindow = (decltype(mMyGUIWidgets.mainWindow))findMyGUIChildWidget("Root");
            mMyGUIWidgets.mainWindow->eventWindowChangeCoord += MyGUI::newDelegate(this, &Editor::MyGUIEditorWindowChangeCoord);

            // agnet property grid
            {
                MyGUI::ScrollView *const propertyGrid = (MyGUI::ScrollView *)findMyGUIChildWidget("EditorAgentPropertyGrid");

                if(nullptr == mAgenModelPropertyGridMan && nullptr != propertyGrid)
                    mAgenModelPropertyGridMan = new PropertyGridManager(propertyGrid);
                else
                    Debug::error("could not initialize mAgenModelPropertyGridMan. Skipping.").endl();
            }

            if(nullptr != mMyGUIWidgets.mainWindow)
            {
                auto coords = mMyGUIWidgets.mainWindow->getPosition();
                Ogre::Vector2 position;

                if(mEngine->config().getSetting(Editor::MENU_WINDOW_POSITION_SETTING, position, Ogre::Vector2(coords.left, coords.top)))
                    mMyGUIWidgets.mainWindow->setPosition(position.x, position.y);
            }
        }

        mBrush.init(mEngine, this, mInputMan);
        registerSignal(mSignals.brushIntensityUpdate = getMyGUIVariableUpdateSignal(Editor::TERRABRUSH_INTENSITY_MYGUIVAR));
        registerSignal(mSignals.brushRadiusUpdate = getMyGUIVariableUpdateSignal(Editor::TERRABRUSH_RADIUS_MYGUIVAR));

        registerSignal(mSignals.menuTabChanged = getMyGUIVariableUpdateSignal(Editor::MENUTAB_CONTROLNAME_MYGUIVAR));

        registerSignal(mSignals.newTagCreated = TagManager::instance().newTagCreatedSignal());
        updateTagsList();

        if(nullptr != mEngine->level())
        {
            registerSignal(mSignals.newPathCreated = mEngine->level()->locationModelMan()->newLocationPathCreatedSignal());
            registerSignal(mSignals.pathDeleted = mEngine->level()->locationModelMan()->locationPathDeletedSignal());
        }

        updatePathsList();

        mEngine->addEngineEventListener(this);

        if(nullptr != mEngine->level())
            mEngine->level()->selectionMan()->addListener(this);
    }

    void Editor::MyGUIEditorWindowChangeCoord(MyGUI::Window *window)
    {
        MyGUI::IntPoint const position = window->getPosition();
        mEngine->config().setSetting(Editor::MENU_WINDOW_POSITION_SETTING, Ogre::StringConverter::toString(Ogre::Vector2(position.left, position.top)));
    }

    void Editor::updatePathsList()
    {
        if(nullptr != mMyGUIWidgets.pathsListComboBox)
        {
            mMyGUIWidgets.pathsListComboBox->removeAllItems();

            if(nullptr == mEngine->level())
                return;

            LocationModelManager *locationMan = mEngine->level()->locationModelMan();

            if(nullptr == locationMan)
                return;

            std::vector<LocationPathName> pathsVec = locationMan->locationPathNames();
            std::sort(pathsVec.begin(), pathsVec.end(), [](LocationPathName const & left, LocationPathName const & right)->bool {return left < right;});

            for(LocationPathName const & path : pathsVec)
                mMyGUIWidgets.pathsListComboBox->addItem(path);
        }
    }

    void Editor::updateTagsList()
    {
        if(nullptr != mMyGUIWidgets.tagsListComboBox)
        {
            auto const tagsVec = TagManager::instance().tags();
            std::vector<Ogre::String> tagsStringVec = TagManager::instance().fromTags(tagsVec);
            std::sort(tagsStringVec.begin(), tagsStringVec.end(), [](Ogre::String const & left, Ogre::String const & right)->bool {return left < right;});
            mMyGUIWidgets.tagsListComboBox->removeAllItems();

            for(auto const & tagString : tagsStringVec)
                mMyGUIWidgets.tagsListComboBox->addItem(tagString);
        }
    }

    void Editor::MyGUIRequestDrawSelectionTagItem(MyGUI::ItemBox *_sender, MyGUI::Widget *_item, const MyGUI::IBDrawItemInfo &_info)
    {
        Ogre::String dataValue = *_sender->getItemDataAt<Ogre::String>(_info.index);
//         Debug::log(STEEL_METH_INTRO, "data: ", dataValue, " from ", _sender).endl();

        // set tag value
        MyGUI::TextBox *textBox = (MyGUI::TextBox *)_item->findWidget("tagTextbox");
        textBox->setCaption(dataValue.c_str());
        textBox->setVisible(true);

        // set button callback
        MyGUI::Button *untagBtn = (MyGUI::Button *)_item->findWidget("untagBtn");
        untagBtn->setUserString(UIPanel::SteelCommand, "engine.editor.selection.tag.unset." + dataValue);
    }

    void Editor::MyGUIRequestCoordWidgetItem(MyGUI::ItemBox *_sender, MyGUI::IntCoord &_coord, bool _drop)
    {
//         Debug::log(STEEL_METH_INTRO, "from ", _sender, " while drop: ", _drop).endl();
        // with an itembox there is no way to have items of different sizes (http://www.ogre3d.org/addonforums/viewtopic.php?f=17&t=29742)
        // so lets place one item on a new row.
        int border = 0;
        _coord = MyGUI::IntCoord(border, 0, _sender->getClientCoord().right() - border, 20);
    }

    void Editor::MyGUIRequestCreateSelectionTagItem(MyGUI::ItemBox *_sender, MyGUI::Widget *_item)
    {
//         Debug::log(STEEL_METH_INTRO, _item, " from ", _sender).endl();

        auto clientCoords = _sender->getClientCoord();
        int const btnSize = 20;
        int const border = 2;

        MyGUI::TextBox *textBox = _item->createWidget<MyGUI::TextBox>("TextBox", MyGUI::IntCoord(border, 0, clientCoords.width - btnSize - 3 * border, _item->getHeight()), MyGUI::Align::Left | MyGUI::Align::VCenter, "tagTextbox");
        textBox->setVisible(true);
        textBox->eventMouseWheel += MyGUI::newDelegate(this, &Editor::MyGUISelectionTagItemMouseWheel);


        MyGUI::Button *untagBtn = _item->createWidget<MyGUI::Button>("Button", MyGUI::IntCoord(clientCoords.right() - btnSize - border, 0, btnSize, _item->getHeight()), MyGUI::Align::Right | MyGUI::Align::VCenter, "untagBtn");
        untagBtn->setCaption("X");
        untagBtn->eventMouseButtonClick += MyGUI::newDelegate((UIPanel *)this, &UIPanel::onMyGUIMouseButtonClick);
        untagBtn->setUserString(UIPanel::SteelOnClick, UIPanel::SteelCommand);
        untagBtn->eventMouseWheel += MyGUI::newDelegate(this, &Editor::MyGUISelectionTagItemMouseWheel);
    }

    void Editor::MyGUISelectionTagItemMouseWheel(MyGUI::Widget *_sender, int _rel)
    {
        mMyGUIWidgets.selectionTagCloud->_riseMouseWheel(_rel);
    }

    void Editor::MyGUIResourceTreeItemDropped(MyGUI::TreeControlItemDecorator *sender, MyGUI::TreeControlNode *node, MyGUI::IntPoint const &pos)
    {
        if(MyGUIHitTest(pos.left, pos.top))
            return;

        File file = *node->getData<MyGUIFileSystemDataSource::ControlNodeDataType>();

        if(!file.exists())
        {
            Debug::error(STEEL_METH_INTRO, "file not found: ", file).endl();
            return;
        }

        Ogre::String rawCommand = "instanciate." + file.fullPath();
        processCommand(rawCommand);
    }

    void Editor::onSignal(Signal signal, SignalEmitter *const src)
    {
        if(mSignals.brushIntensityUpdate ==  signal)
        {
            mBrush.setIntensity(Ogre::StringConverter::parseReal(getMyGUIVariable(Editor::TERRABRUSH_INTENSITY_MYGUIVAR), 2.));
        }
        else if(mSignals.brushRadiusUpdate ==  signal)
        {
            mBrush.setRadius(Ogre::StringConverter::parseReal(getMyGUIVariable(Editor::TERRABRUSH_RADIUS_MYGUIVAR), 10.));
        }
        else if(mSignals.menuTabChanged ==  signal)
        {
            mEngine->config().setSetting(Editor::MENUTAB_ITEMNAME_SETTING, getMyGUIVariable(Editor::MENUTAB_CONTROLNAME_MYGUIVAR));
        }
        else if(mSignals.newTagCreated ==  signal)
        {
            updateTagsList();
        }
        else if(mSignals.newPathCreated ==  signal || mSignals.pathDeleted == signal)
        {
            updatePathsList();
        }
    }

    void Editor::onSelectionChanged(Selection const &selection)
    {
        refreshSelectionTagsWidget();
        refreshSelectionPathWidget();
    }

    void Editor::MyGUIAgentBrowserTreeNodeSelected(MyGUI::TreeControl *control, MyGUI::TreeControlNode *modelNode)
    {
        if(nullptr == mAgenModelPropertyGridMan)
            return;

        // find which agent and select it.
        if(mMyGUIWidgets.agentItemBrowserTree->getSelection() != modelNode)
        {
            Debug::error(STEEL_METH_INTRO, "invalid selected node in agentItemBrowserTree. Aborting.").endl().breakHere();
            return;
        }

        // no selection
        if(nullptr == modelNode)
            return;

        // retrieve agent and model ID of the node
        MyGUIAgentBrowserDataSource::ControlNodeDataType *modelData = modelNode->getData<MyGUIAgentBrowserDataSource::ControlNodeDataType>();

        // node has no data
        if(nullptr == modelData)
            return;

        // set selection iff it differs from the current one
        auto selection = mEngine->level()->selectionMan()->selection();

        if(selection.size() != 1 || modelData->agentId != selection.back())
            mEngine->level()->selectionMan()->setSelectedAgent(modelData->agentId);

        // the selected model may have changed though, so let's update it
        setAgentModelPropertyGridTarget();
    }

    void Editor::setAgentModelPropertyGridTarget()
    {
        MyGUI::TreeControlNode *modelNode = mMyGUIWidgets.agentItemBrowserTree->getSelection();

        PropertyGridAdapter *newAdapter = nullptr;

        if(nullptr != modelNode)
        {
            // retrieve agent and model ID of the node
            MyGUIAgentBrowserDataSource::ControlNodeDataType *modelData = modelNode->getData<MyGUIAgentBrowserDataSource::ControlNodeDataType>();

            if(nullptr != modelData)
            {
                switch(modelData->nodeType)
                {
                    case MyGUIAgentBrowserDataSource::ControlNodeDataType::NodeType::Model:// node is a model node.
                        switch(modelData->modelType)
                        {
                            case ModelType::OGRE:
                            case ModelType::PHYSICS:
                            case ModelType::LOCATION:
                            case ModelType::BT:
                            case ModelType::BLACKBOARD:
                            default:
                                newAdapter = new PropertyGridAdapter(); // TODO: use actual model adapter instead of dummy one
                                break;
                        }

                        break;

                    case MyGUIAgentBrowserDataSource::ControlNodeDataType::NodeType::Agent:// node is an agent
                        newAdapter = new PropertyGridAgentAdapter(mEngine->level()->agentMan(), modelData->agentId);
                        break;
                }
            }
            else
            {
                // node has no data
            }
        }
        else
        {
            // no selection
        }

        auto previousAdapter = mAgenModelPropertyGridMan->adapter();
        mAgenModelPropertyGridMan->setAdapter(newAdapter);
        STEEL_DELETE(previousAdapter);
    }

    void Editor::onLevelSet(Level *level)
    {
        level->selectionMan()->addListener(this);
        updatePathsList();
    }

    void Editor::onLevelUnset(Level *level)
    {
        level->selectionMan()->removeListener(this);
    }

    AgentId Editor::instanciateFromMeshFile(File &meshFile, Ogre::Vector3 &pos, Ogre::Quaternion &rot)
    {
        Level *level = mEngine->level();

        if(level == nullptr)
        {
            Debug::warning(STEEL_METH_INTRO, meshFile, " pos: ", pos, " rot: ", rot, ". no level to instanciate stuff in.").endl();
            return INVALID_ID;
        }

        //      Ogre::Quaternion r = mEngine->camera()->camNode()->getOrientation();
        Steel::ModelId mid = level->ogreModelMan()->newModel(meshFile.fileName(), pos, rot);
        AgentId aid = mEngine->level()->agentMan()->newAgent();

        if(!level->linkAgentToModel(aid, ModelType::OGRE, mid))
        {
            Debug::error(STEEL_METH_INTRO, meshFile, " pos: ", pos, " rot: ", rot, ". could not level->linkAgentToModel(", aid, ", ModelType::OGRE, mid: ", mid, ")").endl();
            return INVALID_ID;
        }

        return mid;
    }

    bool Editor::keyPressed(Input::Code key, Input::Event const &evt)
    {
        return true;
    }

    bool Editor::keyReleased(Input::Code key, Input::Event const &evt)
    {
        SelectionManager *selectionMan = mEngine->level()->selectionMan();
        Level *level = mEngine->level();

        switch(key)
        {
            case Input::Code::KC_H:
                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    mBrush.setMode(EditorBrush::BrushMode::TERRAFORM);

                break;

            case Input::Code::KC_L:
                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    mBrush.setMode(EditorBrush::BrushMode::LINK);

                break;

            case Input::Code::KC_R:
                if(mInputMan->isKeyDown(Input::Code::KC_LCONTROL))
                    reloadContent();

                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    mBrush.setMode(EditorBrush::BrushMode::ROTATE);

                break;

            case Input::Code::KC_S:
                if(mInputMan->isKeyDown(Input::Code::KC_LCONTROL))
                {
                    if(level != nullptr)
                        level->save();

                    mEngine->saveConfig(mEngine->config());
                }

                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    mBrush.setMode(EditorBrush::BrushMode::SCALE);

                break;

            case Input::Code::KC_T:
                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    mBrush.setMode(EditorBrush::BrushMode::TRANSLATE);

                break;

            case Input::Code::KC_DELETE:
                selectionMan->deleteSelection();
                break;

            case Input::Code::KC_F5:
                processCommand("engine.register.ui.reload");
                break;

            default:
                break;
        }

        // numeric key pressed: handles memoing (keys: 1,2,3,...,0)
        if(nullptr != mEngine && key >= Input::Code::KC_1 && key <= Input::Code::KC_0)
        {
            // OIS::KC_0 is the highest; the modulo makes it 0
            int memoKey = ((int)key - (int)Input::Code::KC_1 + 1) % 10;
            Ogre::String memo = Ogre::StringConverter::toString(memoKey);

            if(mInputMan->isKeyDown(Input::Code::KC_LCONTROL))
                selectionMan->saveSelectionToMemo(memo);
            else
                selectionMan->selectMemo(memo);
        }

        return true;
    }

    bool Editor::mousePressed(Input::Code button, Input::Event const &evt)
    {
        if(!MyGUIHitTest(evt.position.x, evt.position.y))
            mBrush.mousePressed(button, evt);

        return true;
    }

    bool Editor::mouseReleased(Input::Code button, Input::Event const &evt)
    {
        mBrush.mouseReleased(button, evt);
        return true;
    }

    bool Editor::mouseWheeled(int delta, Input::Event const &evt)
    {
        if(!MyGUIHitTest(evt.position.x, evt.position.y))
        {
            mBrush.mouseWheeled(delta, evt);
        }

        return true;
    }

    bool Editor::mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt)
    {
        if(!MyGUIHitTest(position.x, position.y) || (mBrush.isDragging() || mBrush.isInContiniousMode() || mBrush.isSelecting()))
            mBrush.mouseMoved(position, evt);

        return true;
    }

    void Editor::onFileChangeEvent(File file)
    {
        Debug::log(STEEL_METH_INTRO, file.fullPath()).endl();
        UIPanel::onFileChangeEvent(file);
    }

    void Editor::onShow()
    {
        mBrush.onShow();

        // (re)load state
        // TODO: set brush shape
        // processCommand(Ogre::String("editorbrush.terrabrush.distribution.") + select_form->GetOption(index)->GetValue().CString());

        refreshSelectionTagsWidget();
        refreshSelectionPathWidget();
    }

    void Editor::onHide()
    {
        mBrush.onHide();
    }

    void Editor::refreshSelectionPathWidget()
    {
        if(nullptr == mEngine->level())
            return;

        auto level = mEngine->level();
        auto selectionMan = level->selectionMan();

        // MyGUI
        {
            Ogre::String value = "";

            if(selectionMan->selection().size() == 0)
            {
                value = "empty selection";
            }
            else if(selectionMan->selection().size() > 1)
            {
                value = "single selection only";
            }
            else
            {
                Agent *agent = level->agentMan()->getAgent(selectionMan->selection().front());
                LocationModel *model = agent->locationModel();

                if(nullptr == model || !model->hasPath())
                    value = "no path found in selection";
                else
                    value = model->path();
            }

            mMyGUIWidgets.selectionPathTextBox->setCaption(value);
        }
    }

    void Editor::refreshSelectionTagsWidget()
    {
        if(nullptr == mEngine->level())
            return;

        auto allTags = mEngine->level()->selectionMan()->tagsUnion();
        auto allTagsStrings = TagManager::instance().fromTags(allTags);
        populateSelectionTagsWidget(allTagsStrings);
    }

    void Editor::populateSelectionTagsWidget(std::list<Ogre::String> tags)
    {
        // MyGUI
        {
            if(nullptr != mMyGUIWidgets.selectionTagCloud)
            {
                mMyGUIWidgets.selectionTagCloud->removeAllItems();

                // add tags buttons
                for(auto const & tag : tags)
                    mMyGUIWidgets.selectionTagCloud->addItem(tag);
            }
        }
    }

    bool Editor::processCommand(Ogre::String rawCommand, bool verbose)
    {
        if(verbose)
            Debug::log("Editor::processCommand(raw=")(rawCommand)(")").endl();

        return processCommand(StringUtils::split(std::string(rawCommand), std::string(".")));
    }

    bool Editor::processCommand(std::vector<Ogre::String> command)
    {
        while(command.size() > 0 && command[0] == "editor")
            command.erase(command.begin());

        if(0 == command.size())
            return false;

        auto level = mEngine->level();
        auto selectionMan = level->selectionMan();

        // dispatch the command to the right subprocessing function

        if(command[0] == "hide")
            mEngine->stopEditMode();
        else if(command[0] == "options")
            return processOptionsCommand(command);
        else if(command[0] == "editorbrush")
        {
            command.erase(command.begin());
            return mBrush.processCommand(command);
        }
        else if(command[0] == "engine")
        {
            command.erase(command.begin());
            return mEngine->processCommand(command);
        }
        else if(command[0] == "instanciate")
        {
            command.erase(command.begin());

            if(command.size() == 0)
            {
                Debug::error(STEEL_METH_INTRO, "instanciate: command ").quotes(command)(" contains no file !").endl();
                return false;
            }

            File file(StringUtils::join(command, "."));

            if(file.exists())
            {
                AgentId aid = INVALID_ID;
                bool success = mEngine->level()->instanciateResource(file, aid);

                if(success)
                    mEngine->level()->agentMan()->getAgent(aid)->setPersistent(true);
            }
            else
            {
                Debug::warning(STEEL_METH_INTRO, "instanciate: file ").quotes(file)(" not found for command ").quotes(command)(". Aborted.").endl();
                return false;
            }
        }
        else if(command[0] == "selection")
        {
            command.erase(command.begin());// selection

            if(command[0] == "tag")
            {
                if(command.size() < 2)
                {
                    Debug::error(STEEL_METH_INTRO, "command ").quotes(command)(" is incomplete").endl();
                    return false;
                }

                command.erase(command.begin());// tag

                if(command[0] == "set")
                {
                    command.erase(command.begin());// set

                    if(command.size() == 0)
                    {
                        Debug::error(STEEL_METH_INTRO, "no tag to set").endl();
                        return false;
                    }

                    if(nullptr != level && selectionMan->hasSelection())
                    {
                        Ogre::String const tagString(StringUtils::join(command, "."));
                        Tag tag = TagManager::instance().toTag(tagString);

                        if(INVALID_TAG != tag)
                        {
                            selectionMan->tagSelection(tag);
                            refreshSelectionTagsWidget();
                        }
                    }
                    else
                        return false;
                }
                else if(command[0] == "unset")
                {

                    command.erase(command.begin());// unset

                    if(command.size() == 0)
                    {
                        Debug::error(STEEL_METH_INTRO, "no tag to unset").endl();
                        return false;
                    }

                    if(nullptr != mEngine->level() && mEngine->level()->selectionMan()->hasSelection())
                    {
                        Tag tag = TagManager::instance().toTag(StringUtils::join(command, "."));
                        mEngine->level()->selectionMan()->untagSelection(tag);
                        refreshSelectionTagsWidget();
                        return true;
                    }
                    else
                        return false;
                }
                else
                {
                    Debug::warning(STEEL_METH_INTRO, "unknown command:").quotes(command)(".").endl();
                    return false;
                }
            }
            else if(command[0] == "path")
            {
                if(command.size() < 2)
                {
                    Debug::error(STEEL_METH_INTRO, "command ").quotes(command)(" is incomplete").endl();
                    return false;
                }

                command.erase(command.begin());// path

                if(command[0] == "set")
                {
                    command.erase(command.begin());// set

                    if(command.size() == 0)
                    {
                        Debug::error(STEEL_METH_INTRO, "no path to set").endl();
                        return false;
                    }

                    if(!selectionMan->hasSelection() || selectionMan->selection().size() > 1)
                    {
                        Debug::error(STEEL_METH_INTRO, "need to select 1 model linked to a LocationModel.").endl();
                        return false;
                    }

                    Agent *agent = level->agentMan()->getAgent(selectionMan->selection().front());

                    if(nullptr == agent)
                        return false;

                    agent->setLocationPath(StringUtils::join(command, "."));
                    refreshSelectionPathWidget();
                }
                else if(command[0] == "unset")
                {
                    command.erase(command.begin());// unset
                    Agent *agent = level->agentMan()->getAgent(selectionMan->selection().front());

                    if(nullptr == agent)
                        return false;

                    agent->unsetLocationPath();
                    refreshSelectionPathWidget();
                }
                else
                {
                    Debug::warning(STEEL_METH_INTRO, "unknown path command: \"")(command)("\".").endl();
                    return false;
                }
            }
            else
            {
                Debug::warning(STEEL_METH_INTRO, "unknown selection command: \"")(command)("\".").endl();
                return false;
            }
        }
        else
        {
            Debug::warning(STEEL_METH_INTRO, "unknown editor command: \"")(command)("\".").endl();
            return false;
        }

        return true;
    }

    bool Editor::processOptionsCommand(std::vector<Ogre::String> command)
    {
        while(command.size() > 0 && command[0] == "options")
            command.erase(command.begin());

        if(0 == command.size())
            return false;

        if(command[0] == "resourceGroupsInfos")
            OgreUtils::resourceGroupsInfos();
        else if(command[0] == "tagsInfos")
            printTagsInfos();
        else if(command[0] == "pathsInfos")
            printPathsInfos();
        else if(command[0] == "switch_debug_events")
        {
            UIPanel::setDebugEvents(!UIPanel::getDebugEvents());
        }
        else
        {
            Debug::warning("Editor::processOptionsCommand(): unknown command: ")(command).endl();
            return false;
        }

        return true;
    }

    void Editor::printTagsInfos()
    {
        // get tags
        auto const tagsVec = TagManager::instance().tags();
        Debug::log(STEEL_METH_INTRO, "total tags: ", tagsVec.size()).endl();

        // make pairs
        std::vector<std::pair<Tag, Ogre::String>> pairs;

        for(auto const & tag : tagsVec)
            pairs.push_back(std::make_pair(tag, TagManager::instance().fromTag(tag)));

        //sort em by string value
        std::sort(pairs.begin(), pairs.end(), [](std::pair<Tag, Ogre::String> const & left, std::pair<Tag, Ogre::String> const & right)->bool {return left.first < right.first;});

        for(auto const & pair : pairs)
            Debug::log("tag: ", pair.first, " value: ", pair.second).endl();

        Debug::log("done.").endl();
    }

    void Editor::printPathsInfos()
    {
        Level const *const level = mEngine->level();

        if(nullptr == level)
        {
            Debug::log(STEEL_METH_INTRO, "no current level !").endl();
        }
        else
        {
            LocationModelManager const *const locationMan = level->locationModelMan();

            if(nullptr == locationMan)
            {
                Debug::log(STEEL_METH_INTRO, "current level has no locationModelManager !").endl();
            }
            else
            {
                // get paths
                auto pathsVec = locationMan->locationPathNames();
                Debug::log(STEEL_METH_INTRO, "total paths: ", pathsVec.size()).endl();

                //sort em by string value
                std::sort(pathsVec.begin(), pathsVec.end(), [](LocationPathName const & left, LocationPathName const & right)->bool {return left < right;});

                for(auto const & path : pathsVec)
                    Debug::log("path: ", path).endl();
            }
        }

        Debug::log("done.").endl();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


