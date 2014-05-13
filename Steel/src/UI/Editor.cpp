#include <algorithm>
#include <json/json.h>

#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Controls/ElementTabSet.h>
#include <Rocket/Controls/ElementFormControlSelect.h>
#include <Rocket/Controls/ElementFormControlTextArea.h>
#include <Rocket/Core/Event.h>
#include <Rocket/Debugger.h>
#include <Rocket/Core/Element.h>

#include <MyGUI.h>

#include "UI/Editor.h"
#include "Debug.h"
#include "tools/StringUtils.h"
#include "tools/OgreUtils.h"
#include "Level.h"
#include "Engine.h"
#include "UI/FileSystemDataSource.h"
#include "UI/UI.h"
#include "Camera.h"
#include "models/Agent.h"
#include "models/OgreModelManager.h"
#include "models/PhysicsModelManager.h"
#include "tools/JsonUtils.h"
#include "tools/3dParties/MyGUI/TreeControlItemDecorator.h"
#include "tools/3dParties/MyGUI/TreeControlItem.h"
#include "tools/3dParties/MyGUI/MyGUIFileTreeDataSource.h"
#include "models/AgentManager.h"
#include "SelectionManager.h"
#include "models/LocationModel.h"
#include <models/LocationModelManager.h>
#include "TagManager.h"

namespace Steel
{

    const Ogre::String Editor::MENU_TAB_INDEX_SETTING = "Editor::menuTabIndex";
    const Ogre::String Editor::MENUTAB_ITEMNAME_SETTING = "Editor::menuTabItemName";

    const char *Editor::SELECTION_TAGS_INFO_BOX = "selectionTagsInfoBox";
    const char *Editor::SELECTIONS_TAG_EDIT_BOX = "selection_tags_editbox";
    const char *Editor::AGENT_TAG_ITEM_NAME = "agenttagitem";

    const char *Editor::SELECTION_PATH_INFO_BOX = "selectionPathsInfoBox";
    const char *Editor::SELECTIONS_PATH_EDIT_BOX = "selection_path_editbox";

    const Ogre::String Editor::TERRABRUSH_INTENSITY_MYGUIVAR = "terrabrushIntensity";
    const Ogre::String Editor::TERRABRUSH_RADIUS_MYGUIVAR = "terrabrushRadius";
    const Ogre::String Editor::MENUTAB_CONTROLNAME_MYGUIVAR = "editorMenuTab";

    Editor::Editor(UI &ui): UIPanel(ui, "Editor", "data/ui/current/editor/editor.rml"),
        mEngine(nullptr), mInputMan(nullptr),
        mSignals(), mFSResources(nullptr),
        mDataDir(), mBrush(), mDebugEvents(false), mIsDraggingFromMenu(false),
        mDebugValueMan(), mMyGUIWidgets()
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

        if(nullptr != mFSResources)
        {
            delete mFSResources;
            mFSResources = nullptr;
        }
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
        }
    }

    void Editor::saveConfig(ConfigFile &config) const
    {
        mBrush.saveConfig(config);
        saveMenuTabIndexSetting(config);
    }

    void Editor::saveMenuTabIndexSetting(ConfigFile &config) const
    {
        // save state
        if(nullptr != mDocument)
        {
            auto elem = (Rocket::Controls::ElementTabSet *) mDocument->GetElementById("editor_tabset");

            if(nullptr != elem)
            {
                int tabNo = elem->GetActiveTab();
                config.setSetting(Editor::MENU_TAB_INDEX_SETTING, Ogre::StringConverter::toString(tabNo));
            }
        }
    }

    void Editor::shutdown()
    {
        //MyGUI stuff
        {
            mMyGUIWidgets.resourceTreeControlItemDecorator = nullptr;
            mMyGUIWidgets.selectionTagCloud = nullptr;
            mMyGUIWidgets.tagsListComboBox = nullptr;
        }

        mBrush.shutdown();

        if(nullptr != mEngine->level())
        {
            mEngine->level()->selectionMan()->removeListener(this);
        }

        mEngine->removeEngineEventListener(this);
        mDebugValueMan.shutdown();
        UIPanel::shutdown();
    }

    void Editor::init(unsigned int width, unsigned int height)
    {
        init(width, height, nullptr);
    }

    void Editor::init(unsigned int width, unsigned int height, Engine *engine)
    {
        Debug::log("Editor::init()").endl();

        if(nullptr != engine)
        {
            mEngine = engine;
            mInputMan = engine->inputMan();
        }

        mDebugValueMan.init("debugvaluemanager_select_entry", mDocument);
        mDataDir = mUI.UIDataDir().subfile("editor").fullPath();
        auto resGroupMan = Ogre::ResourceGroupManager::getSingletonPtr();
        // true is for recursive search. Add to this resources.cfg
        resGroupMan->addResourceLocation(mDataDir.fullPath(), "FileSystem", "UI", true);

        mFSResources = new FileSystemDataSource("resources", mEngine->resourcesDir());
        UIPanel::init(width, height);

        // finalize MYGui setup
        {
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

        }
        mFSResources->localizeDatagridBody(mDocument);
        auto elem = (Rocket::Controls::ElementFormControlInput *) mDocument->GetElementById("level_name");

        if(elem != nullptr)
        {
            elem->SetValue("MyLevel");
            // does not work for some reason
            // elem->AddEventListener("submit",this);
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
        untagBtn->eventMouseButtonClick += MyGUI::newDelegate((UIPanel *)this, &UIPanel::OnMyGUIMouseButtonClick);
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

    void Editor::onLevelSet(Level *level)
    {
        level->selectionMan()->addListener(this);
        updatePathsList();
    }

    void Editor::onLevelUnset(Level *level)
    {
        level->selectionMan()->removeListener(this);
    }

    bool Editor::rocketHitTest(int x, int y, Rocket::Core::String childId)
    {
        Rocket::Core::Element *elem = mDocument;

        if(elem != nullptr)
        {
            if((elem = elem->GetElementById(childId)) != nullptr)
            {
                const Rocket::Core::Vector2f &tl = elem->GetAbsoluteOffset(Rocket::Core::Box::PADDING);
                int left = tl.x;
                int top = tl.y;
                const Rocket::Core::Vector2f &box = elem->GetBox(Rocket::Core::Box::BORDER).GetSize(
                                                        Rocket::Core::Box::BORDER);
                int right = left + box.x;
                int bottom = top + box.y;

                if(x >= left && y >= top && x <= right && y <= bottom)
                    return true;
            }
        }

        return false;
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
        Rocket::Core::Input::KeyIdentifier keyIdentifier = mUI.getRocketKeyIdentifier(key);
        mContext->ProcessKeyDown(keyIdentifier, mUI.getRocketKeyModifierState());

        if(evt.text >= 32)
        {
            mContext->ProcessTextInput((Rocket::Core::word) evt.text);
        }
        else if(keyIdentifier == Rocket::Core::Input::KI_RETURN)
        {
            mContext->ProcessTextInput((Rocket::Core::word) '\n');
        }

        return true;
    }

    bool Editor::keyReleased(Input::Code key, Input::Event const &evt)
    {
        Rocket::Core::Input::KeyIdentifier keyIdentifier = mUI.getRocketKeyIdentifier(key);
        mContext->ProcessKeyUp(keyIdentifier, mUI.getRocketKeyModifierState());

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
            {
//                 auto child = mDocument->GetElementById("editor");

//                 if(nullptr == child || !child->IsPseudoClassSet("focus"))
//                 {
                selectionMan->deleteSelection();
//                 }
            }
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
        if(rocketHitTest(evt.position.x, evt.position.y, "menu"))
            mContext->ProcessMouseButtonDown(mUI.getRocketMouseIdentifier(button), mUI.getRocketKeyModifierState());
        else if(!MyGUIHitTest(evt.position.x, evt.position.y))
            mBrush.mousePressed(button, evt);

        return true;
    }

    bool Editor::mouseReleased(Input::Code button, Input::Event const &evt)
    {
        mContext->ProcessMouseButtonUp(mUI.getRocketMouseIdentifier(button), mUI.getRocketKeyModifierState());
        mBrush.mouseReleased(button, evt);
        return true;
    }

    bool Editor::mouseWheeled(int delta, Input::Event const &evt)
    {
        auto keyModifierState = mUI.getRocketKeyModifierState();
        mContext->ProcessMouseWheel(delta / -120, keyModifierState);

        if(!rocketHitTest(evt.position.x, evt.position.y, "menu") && !MyGUIHitTest(evt.position.x, evt.position.y))
        {
            mBrush.mouseWheeled(delta, evt);
        }

        return true;
    }

    bool Editor::mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt)
    {
        bool hoveringMenu = rocketHitTest(position.x, position.y, "menu");

        if((!mIsDraggingFromMenu && !hoveringMenu) || (mBrush.isDragging() || mBrush.isInContiniousMode() || mBrush.isSelecting()))
        {
            mBrush.mouseMoved(position, evt);
            Rocket::Core::Element *elem;

            elem = mDocument->GetElementById("editor_terrabrush_intensity");

            if(elem != nullptr)
                elem->SetInnerRML(Ogre::StringConverter::toString(mBrush.intensity()).c_str());

            elem = mDocument->GetElementById("editor_terrabrush_radius");

            if(elem != nullptr)
                elem->SetInnerRML(Ogre::StringConverter::toString(mBrush.radius()).c_str());
        }
        else
        {
            mContext->ProcessMouseMove(position.x, position.y, mUI.getRocketKeyModifierState());
        }

        return true;
    }

    void Editor::onFileChangeEvent(File file)
    {
        Debug::log("Editor::onFileChangeEvent(): ")(file.fullPath()).endl();
        UIPanel::onFileChangeEvent(file);
        Rocket::Debugger::SetContext(mContext);
        Rocket::Debugger::SetVisible(true);
    }

    void Editor::onShow()
    {
        mBrush.onShow();

        // (re)load state
        // active menu tab
        auto elem = (Rocket::Controls::ElementTabSet *) mDocument->GetElementById("editor_tabset");

        if(nullptr != elem)
        {
            int tabNo;
            mEngine->config().getSetting(Editor::MENU_TAB_INDEX_SETTING, tabNo, 0);
            elem->SetActiveTab(tabNo);
        }

        // ## reconnect to document
        // data sources
        mFSResources->refresh(mDocument);
        mDebugValueMan.refresh(mDocument);

        // events
        if(nullptr != mDocument)
        {
            mDocument->AddEventListener("click", this);
            mDocument->AddEventListener("dragstart", this);
            mDocument->AddEventListener("dragdrop", this);
            mDocument->AddEventListener("change", this);
            mDocument->AddEventListener("submit", this);
        }

        // debugger
        Rocket::Debugger::SetContext(mContext);
        Rocket::Debugger::SetVisible(true);

        // set brush shape
        auto select_form = static_cast<Rocket::Controls::ElementFormControlSelect *>(mDocument->GetElementById("editor_select_terrabrush_shape"));

        if(select_form != nullptr and select_form->GetNumOptions() > 0)
        {
            int index = select_form->GetSelection();

            if(index < 0)
                select_form->SetSelection(0);
            else
                processCommand(Ogre::String("editorbrush.terrabrush.distribution.") + select_form->GetOption(index)->GetValue().CString());
        }

        refreshSelectionTagsWidget();
        refreshSelectionPathWidget();
    }

    void Editor::onHide()
    {
        if(nullptr != mDocument)
        {
            mDocument->RemoveEventListener("click", this);
            mDocument->RemoveEventListener("dragdrop", this);
            mDocument->RemoveEventListener("change", this);
            mDocument->RemoveEventListener("submit", this);
            saveMenuTabIndexSetting(mEngine->config());
        }

        mBrush.onHide();
    }

    void Editor::onSelectionChanged(Selection &selection)
    {
        refreshSelectionTagsWidget();
        refreshSelectionPathWidget();
    }

    void Editor::refreshSelectionPathWidget()
    {
        if(nullptr == mEngine->level())
            return;

        auto level = mEngine->level();
        auto selectionMan = level->selectionMan();

        // libRocket
        {
            Rocket::Core::String innerRML = "";

            if(selectionMan->selection().size() == 0)
            {
                innerRML = "empty selection";
            }
            else if(selectionMan->selection().size() > 1)
            {
                innerRML = "single selection only";
            }
            else
            {
                Agent *agent = level->agentMan()->getAgent(selectionMan->selection().front());

                if(nullptr == agent)
                    innerRML = "no path found in selection";
                else
                {
                    LocationModel *model = agent->locationModel();

                    if(nullptr == model || !model->hasPath())
                        innerRML = "no path found in selection";
                    else
                    {
                        innerRML = model->path().c_str();
                    }
                }
            }

            if(nullptr != mDocument)
            {

                Rocket::Core::Element *elem = mDocument->GetElementById(Editor::SELECTION_PATH_INFO_BOX);

                if(nullptr == elem)
                    Debug::error(STEEL_METH_INTRO, "child ").quotes(Editor::SELECTION_PATH_INFO_BOX)("not found. Aborting.").endl();
                else
                    elem->SetInnerRML(innerRML);
            }
        }

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
//         Debug::log(STEEL_METH_INTRO, "tags: ", tags).endl();

        // libRocket
        if(nullptr != mDocument)
        {
            Rocket::Core::Element *elem = mDocument->GetElementById(Editor::SELECTION_TAGS_INFO_BOX);

            if(nullptr != elem)
            {

                // Emtpy it
                while(nullptr != elem->GetFirstChild())
                {
                    Rocket::Core::Element *child = elem->GetFirstChild();
                    elem->RemoveChild(child);
                }

                // Repopulate it
                for(auto const & it : tags)
                {
                    Rocket::Core::Element *child = mDocument->CreateElement(Editor::AGENT_TAG_ITEM_NAME);
                    decorateSelectionTagWidgetItem(child, it.c_str());
                    elem->AppendChild(child);
                    child->RemoveReference();
                }
            }
            else
            {
                Debug::error(STEEL_METH_INTRO, "child with id ").quotes(Editor::SELECTION_TAGS_INFO_BOX)(" not found. Aborting.").endl();
            }
        }

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

    void Editor::decorateSelectionTagWidgetItem(Rocket::Core::Element *item, Ogre::String const &tagName)
    {
        // default tag button: "tagName [x]"
        Ogre::String rml = tagName + "<p style=\"margin-left:3px;\" onclick=\"selection.tag.unset.$tagName\">[x]</p>";

        // try getting the closing button from the data templates
        Rocket::Core::Element *elemTemplate = mDocument->GetElementById("SelectionTagWidget_AgentItem_Btn");

        if(nullptr != elemTemplate && elemTemplate->GetNumChildren() > 0)
        {
            rml = tagName + elemTemplate->GetInnerRML().CString();
        }

        item->SetInnerRML(Ogre::StringUtil::replaceAll(rml, "$tagName", tagName).c_str());
    }

    void Editor::ProcessEvent(Rocket::Core::Event &event)
    {
        if(!isVisible())
            return;

// create the command
        Rocket::Core::Element *elem = nullptr;

// in case of drag&drop, elem points to the element being dragged
        if(event == "dragdrop")
        {
            // ok in stable, not in dev
            //             elem= static_cast<Rocket::Core::Element *>(event.GetParameter< void * >("drag_element", nullptr));
            Rocket::Core::ElementReference *ref = static_cast<Rocket::Core::ElementReference *>(event.GetParameter<void *>(
                    "drag_element", nullptr));
            elem = **ref;
            mIsDraggingFromMenu = false;
        }
        else if(event == "dragstart")
            mIsDraggingFromMenu = true;
        else
        {
            elem = event.GetTargetElement();
            mIsDraggingFromMenu = false;
        }

        if(elem == nullptr)
        {
            if(mDebugEvents)
                Debug::log("Editor::ProcessEvent(): no target element for event of type ")(event.GetType()).endl();

            return;
        }

        auto etype = event.GetType();
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + etype, "NoValue").CString();

        if(event_value == "NoValue")
        {
            if(elem->GetId() != "")
            {
                if(mDebugEvents)
                {
                    Debug::warning("Editor::ProcessEvent(): no event_value for event of type ")
                    (event.GetType())(" with elem of id ")(elem->GetId()).endl();
                }

                return;
            }
        }

        if(etype == "change")
            processChangeEvent(event, elem);
        else if(etype == "click")
            processClickEvent(event, elem);
        else if(etype == "dragdrop")
            processDragDropEvent(event, elem);
        else if(etype == "submit")
            processSubmitEvent(event, elem);
        else
            Debug::log("Editor::ProcessEvent(): unknown event ot type:")(event.GetType())(" and value: ")(event_value).endl();

        return;
    }

    void Editor::processSubmitEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem)
    {
        Debug::log("Editor::processSubmitEvent(): type:")(event.GetType());
        Debug::log(" and value: ")(elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString());
        Debug::log.endl();
    }

    void Editor::processClickEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem)
    {
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString();
        Ogre::String rawCommand = event_value;

        if(rawCommand == "engine.set_level")
        {
            auto inputField = (Rocket::Controls::ElementFormControlInput *) mDocument->GetElementById("level_name");

            if(inputField == nullptr)
            {
                Debug::error("Editor::processClickEvent(): can't find level name input field with id=\"level_name\". Aborted.").endl();
                return;
            }

            Ogre::String levelName = inputField->GetValue().CString();

            if(levelName == "")
            {
                Debug::error("Editor::processClickEvent(): can't create a level with not name. Aborted.").endl();
                return;
            }

            rawCommand += "." + levelName;
        }
        else if(rawCommand == "selection.tag.set")
        {
            Ogre::String content = getFormControlInputValue(Editor::SELECTIONS_TAG_EDIT_BOX);

            if(content.size() > 0)
            {
                rawCommand += ".";
                rawCommand += content;
            }
            else
            {
                Debug::log(STEEL_METH_INTRO, "rawCommand: ").quotes(rawCommand)
                (", text input is empty. Not setting the empty tag.").endl();
                return;
            }
        }
        else if(Ogre::StringUtil::startsWith(rawCommand, "selection.tag.unset"))
        {
            // valid command, nothing to add
        }
        else if(rawCommand == "selection.path.set")
        {
            Ogre::String content = getFormControlInputValue(Editor::SELECTIONS_PATH_EDIT_BOX);

            if(content.size() > 0)
            {
                rawCommand += ".";
                rawCommand += content;
            }
            else
            {
                Debug::log(STEEL_METH_INTRO, "rawCommand: ").quotes(rawCommand)(", text input is empty. Not setting the empty tag.").endl();
                return;
            }
        }
        else if(rawCommand == "NoValue")
        {
            return;
        }

        //         Debug::log("Editor::processClickEvent() event type:")(event.GetType())(" rawCommand:")(rawCommand).endl();
        if(rawCommand.size())
            processCommand(rawCommand);
    }

    void Editor::processChangeEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem)
    {
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString();
        Ogre::String rawCommand = event_value;
        bool verbose = true;

        if(Ogre::StringUtil::startsWith(rawCommand, "debugvaluemanager."))
        {
            if(Ogre::StringUtil::endsWith(rawCommand, ".update"))
                verbose = false;

            // parse sibling, looking for the select element
            Rocket::Core::ElementList children;
            elem->GetParentNode()->GetElementsByTagName(children, "select");

            if(children.size() != 1)
            {
                Debug::warning(STEEL_METH_INTRO, "did not find 1 single sibling of ")(elem->GetTagName())(" tagged \"select\". Aborting command ").quotes(rawCommand).endl();
                return;
            }

            Ogre::String id = children[0]->GetId().CString();

            rawCommand += ".";
            rawCommand += id;

        }
        else if(rawCommand == "editorbrush.terrabrush.distribution")
        {
            Rocket::Controls::ElementFormControlSelect *form = static_cast<Rocket::Controls::ElementFormControlSelect *>(elem);
            auto optionId = form->GetSelection();

            if(optionId > -1)
            {
                rawCommand += ".";
                rawCommand += form->GetValue().CString();
            }
            else
                return;
        }
        else if(rawCommand == "selection.tag.set")
        {
            bool linebreak = event.GetParameter<bool>("linebreak", false);

            if(!linebreak)
                return;

            Ogre::String content = getFormControlInputValue(Editor::SELECTIONS_TAG_EDIT_BOX);

            if(content.size() > 0)
            {
                rawCommand += ".";
                rawCommand += content;
            }
            else
            {
                if(mDebugEvents)
                {
                    Debug::log(STEEL_METH_INTRO, "rawCommand: ").quotes(rawCommand)(", event_value: ").quotes(event_value)(" does not end with a new line. Skipping.").endl();
                }

                return;
            }
        }
        else if(rawCommand == "selection.path.set")
        {
            bool linebreak = event.GetParameter<bool>("linebreak", false);

            if(!linebreak)
                return;

            Ogre::String content = getFormControlInputValue(Editor::SELECTIONS_PATH_EDIT_BOX);

            if(content.size() > 0)
            {
                rawCommand += ".";
                rawCommand += content;
            }
            else
            {
                if(mDebugEvents)
                {
                    Debug::log(STEEL_METH_INTRO, "rawCommand: ").quotes(rawCommand)(", event_value: ").quotes(event_value)(" does not end with a new line. Skipping.").endl();
                }

                return;
            }
        }

        //         Debug::log("Editor::processChangeEvent() event type:")(event.GetType())(" rawCommand:")(rawCommand).endl();
        if(rawCommand.size())
            processCommand(rawCommand, verbose);
    }

    void Editor::processDragDropEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem)
    {
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString();
        Ogre::String rawCommand = "";

        if(elem->GetId() == mFSResources->GetDataSourceName())
        {
            rawCommand = "instanciate.";
            File file = File(event_value);

            if(!file.exists())
            {
                Debug::error("Editor::ProcessEvent(): file not found: ")(file).endl();
                return;
            }

            rawCommand += file.fullPath();
        }
        else
        {
            Debug::warning("Editor::ProcessDragDropEvent() unknown element source for event of type: ")(event.GetType());
            Debug::warning(" value: ")(event_value).endl();
            return;
        }

        //         Debug::log("Editor::processDragDropEvent() event type:")(event.GetType())(" rawCommand:")(rawCommand).endl();
        if(rawCommand.size())
            processCommand(rawCommand);
    }

    bool Editor::processCommand(Ogre::String rawCommand, bool verbose)
    {
        if("NoValue" == rawCommand)
            return true;

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
        else if(command[0] == "debugvaluemanager")
        {
            Debug::error("TODO: check implementation").endl().breakHere();
            command.erase(command.begin());
            // second call erase the id of the debug value manager. We have a single one for now, so that selection is easy.
            command.erase(command.end());
            return mDebugValueMan.processCommand(command);
        }
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
                            auto form = setFormControlInputValue(Editor::SELECTIONS_TAG_EDIT_BOX, "");

                            if(nullptr != form)
                                form->Focus();
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
                        auto form = setFormControlInputValue(Editor::SELECTIONS_TAG_EDIT_BOX, "");

                        if(nullptr != form)
                            form->Focus();

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

    Ogre::String Editor::getFormControlInputValue(Ogre::String elementId)
    {
        if(nullptr == mDocument)
            return "";

        Rocket::Core::Element *elem = mDocument->GetElementById(elementId.c_str());

        if(nullptr == elem)
            return "";

        // try to assert we're actually using a form
        Ogre::String tagName = elem->GetTagName().CString();

        if("input" != tagName && "select" != tagName)
        {
            Debug::error(STEEL_METH_INTRO, "trying to use elem with id ").quotes(elementId)
            (" and tagname ").quotes(tagName)(" as Rocket form. This would probably result in a segfault. Aborting.").endl();
            return "";
        }

        Rocket::Controls::ElementFormControlInput *form = static_cast<Rocket::Controls::ElementFormControlInput *>(elem);
        Ogre::String content = form->GetValue().CString();
        return content;
    }

    Rocket::Controls::ElementFormControlInput *Editor::setFormControlInputValue(Ogre::String elementId, Ogre::String value)
    {
        if(nullptr == mDocument)
            return nullptr;

        Rocket::Core::Element *elem = mDocument->GetElementById(elementId.c_str());

        if(nullptr == elem)
            return nullptr;

        // try to assert we're actually using a form
        Ogre::String tagName = elem->GetTagName().CString();

        if("input" != tagName && "select" != tagName)
        {
            Debug::error(STEEL_METH_INTRO, "trying to use elem with id ").quotes(elementId)
            (" and tagname ").quotes(tagName)(" as Rocket form. This would probably result in a segfault. Aborting.").endl();
            return nullptr;
        }

        Rocket::Controls::ElementFormControlInput *form = static_cast<Rocket::Controls::ElementFormControlInput *>(elem);
        Rocket::Core::String _value = value.c_str();
        form->SetValue(_value);
        return form;
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
            mDebugEvents = !mDebugEvents;
            Debug::log("flag DebugEvent ")(mDebugEvents ? "activated" : "deactivated").endl();
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

    void Editor::addDebugValue(const Ogre::String &entryName, Steel::DebugValueManager::CallbackFunction callback, float min, float max, float init)
    {
        mDebugValueMan.addDebugValue(entryName, callback, min , max, init);
    }

    void Editor::removeDebugValue(const Ogre::String &entryName)
    {
        mDebugValueMan.removeDebugValue(entryName);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


