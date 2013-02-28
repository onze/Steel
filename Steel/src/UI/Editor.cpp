
#include <json/json.h>

#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Controls/ElementTabSet.h>
#include <Rocket/Controls/ElementFormControlSelect.h>
#include <Rocket/Core/Event.h>
#include <Rocket/Debugger.h>
#include <Rocket/Core/Element.h>

#include "UI/Editor.h"
#include "Debug.h"
#include "tools/StringUtils.h"
#include "tools/OgreUtils.h"
#include "Level.h"
#include "Engine.h"
#include "UI/FileSystemDataSource.h"
#include "Camera.h"
#include "Agent.h"
#include "OgreModelManager.h"
#include "PhysicsModelManager.h"

namespace Steel
{
    Editor::Editor():UIPanel("Editor","data/ui/current/editor/editor.rml"),
        mEngine(NULL),mUI(NULL),mInputMan(NULL),mFSResources(NULL),mDataDir(),
        mMenuTabIndex(1),mBrush(),mDebugEvents(false),mIsDraggingFromMenu(false),
        mSelectionsTags(std::map<Ogre::String, Selection>())
    {
#ifdef DEBUG
        mAutoReload=true;
#endif
    }

    Editor::Editor(const Editor& other)
    {
        throw std::runtime_error("Editor::Editor(const Editor& other): Not Implemented");
    }

    Editor::~Editor()
    {
        shutdown();
    }

    Editor& Editor::operator=(const Editor& other)
    {
        throw std::runtime_error("Editor::operator=(const Editor& other): Not Implemented");
        return *this;
    }

    void Editor::shutdown()
    {
        UIPanel::shutdown();
        mBrush.shutdown();

        if(NULL!=mFSResources)
        {
            delete mFSResources;
            mFSResources=NULL;
        }

        mEngine=NULL;
        mUI=NULL;
        mInputMan=NULL;
    }

    void Editor::init(unsigned int width, unsigned int height, Engine *engine, UI *ui, InputManager *inputMan)
    {
        mDataDir=ui->dataDir().subfile("editor").fullPath();
        Debug::log("Editor::init()").endl();
        auto resGroupMan=Ogre::ResourceGroupManager::getSingletonPtr();
        // true is for recursive search. Add to this resources.cfg
        resGroupMan->addResourceLocation(mDataDir.fullPath(), "FileSystem", "UI",true);
        //resGroupMan->addResourceLocation(mDataDir.subfile("images").fullPath(), "FileSystem", "UI",true);
        //resGroupMan->declareResource("inode-directory.png","Texture","UI");

        mFSResources=new FileSystemDataSource("resources",engine->rootDir().subfile("data").subfile("resources"));
        mEngine=engine;
        mUI=ui;
        mInputMan=inputMan;
        UIPanel::init(width,height);
        mFSResources->localizeDatagridBody(mDocument);
        auto elem=(Rocket::Controls::ElementFormControlInput *)mDocument->GetElementById("level_name");
        if(elem!=NULL)
        {
            elem->SetValue("MyLevel");
            // does not work for some reason
            //             elem->AddEventListener("submit",this);
        }
        mBrush.init(engine,this,mInputMan);
    }

    AgentId Editor::agentIdUnderMouse()
    {
        auto selection=Selection();
        mEngine->pickAgents(selection, mInputMan->mousePos().x,mInputMan->mousePos().y);

        AgentId aid=INVALID_ID;
        if(selection.size()>0)
            aid=selection.front();
        return aid;
    }

    ModelId Editor::modelIdUnderMouse(ModelType mType)
    {
        Ogre::String intro="Editor::ModelIdUnderMouse(mType="+modelTypesAsString[mType]+")";

        ModelId mid=INVALID_ID;
        AgentId aid=agentIdUnderMouse();

        Agent *agent=mEngine->level()->getAgent(aid);
        if(NULL==agent)
            Debug::error(intro)("Can't find agent ")(aid).endl();
        else
            mid=agent->modelId(mType);
        return mid;
    }

    bool Editor::dynamicFillSerialization(Json::Value& root, AgentId aid)
    {
        Ogre::String intro="Editor::dynamicFillSerialization(): ";
        if(root.isConvertibleTo(Json::arrayValue))
        {
            // array: add each value to the fringe
            for(unsigned i=0; i<root.size(); ++i)
            {
                if(!dynamicFillSerialization(root[i]))
                    return false;
            }
        }
        else if(root.isConvertibleTo(Json::objectValue))
        {
            // dict:: process each value
            Ogre::String key;
            Json::Value value;
            auto names=root.getMemberNames();
            auto level=mEngine->level();
            if(NULL==level)
            {
                Debug::error(intro)("Cannot find level. Aborting.").endl();
                return false;
            }
            for(auto it=names.begin(); it!=names.end(); ++it)
            {
                key=*it;
                Ogre::String svalue,new_svalue;
                svalue=new_svalue=root[key].asString();

                if(INVALID_ID!=aid)
                {
                    auto agent=level->getAgent(aid);
                    if(svalue=="$agentOgreModel")
                        new_svalue=Ogre::StringConverter::toString(agent->ogreModelId());
                    else if(svalue=="$agentPhysicsModel")
                        new_svalue=Ogre::StringConverter::toString(agent->physicsModelId());
                }

                if(svalue.at(0)!='$')
                    new_svalue=svalue;
                else if(svalue=="$dropTargetPosition")
                    new_svalue=Ogre::StringConverter::toString(getDropTargetPosition());
                else if(svalue=="$dropTargetRotation")
                    new_svalue=Ogre::StringConverter::toString(getDropTargetRotation());
                else if(svalue=="$agentUnderMouse")
                {
                    AgentId aid_um=agentIdUnderMouse();
                    if(INVALID_ID==aid_um)
                        return false;
                    new_svalue=Ogre::StringConverter::toString((unsigned int)aid_um);
                }
                else if(svalue=="$OgreModelUnderMouse")
                {
                    //TODO use ModelIdUnderMouse
                    if(0);
                    else
                        new_svalue=Json::Value().asString();
                }
                else if(svalue=="$slotDropPosition")
                {
                    auto slotPosition=getSlotDropPosition();
                    // drops farther than 5km away are forbidden
                    if(slotPosition.length()>5000)
                    {
                        Debug::error(intro)("slot drop position is invalid (>10km away):")(slotPosition)(". Aborted.").endl();
                        return false;
                    }
                    new_svalue=Ogre::StringConverter::toString(slotPosition);
                }

                root[key]=new_svalue.c_str();
            }
        }
        else
        {
            Debug::error(intro)(" unrecognized value type. Value was:")(root).endl()("Aborted.").endl();
            return false;
        }
        return true;
    }

    Ogre::Vector3 Editor::getDropTargetPosition()
    {
        if(NULL==mEngine || NULL==mEngine->level())
            return Ogre::Vector3::ZERO;
        return mEngine->level()->camera()->dropTargetPosition();
    }

    Ogre::Quaternion Editor::getDropTargetRotation()
    {
        if(NULL==mEngine || NULL==mEngine->level())
            return Ogre::Quaternion::ZERO;
        return mEngine->level()->camera()->dropTargetRotation();
    }

    Ogre::Vector2 Editor::getSlotDropPosition()
    {
        if(NULL==mEngine || NULL==mEngine->level())
            return Ogre::Vector2::ZERO;

        auto cam=mEngine->level()->camera();

        auto mousPos=mInputMan->mousePos();
        auto camRay=cam->cam()->getCameraToViewportRay(mousPos.x/static_cast<float>(mWidth),mousPos.y/static_cast<float>(mHeight));

        // get terrain under the cam
        Ogre::Terrain *terrain=NULL;
        auto camPos=cam->camNode()->convertLocalToWorldPosition(Ogre::Vector3::ZERO);
        float height=mEngine->level()->terrainManager()->terrainGroup()->getHeightAtWorldPosition(camPos,&terrain);

        Ogre::Plane plane(Ogre::Vector3::UNIT_Y,0.f);
        if(NULL!=terrain)
        {
            // cam is above a terrain, we use its height as base for the plane
            plane.d+=height;
        }

        // find where the drop point is
        auto result=camRay.intersects(plane);
        if(!result.first)
        {
            Debug::warning("Camera::slotDropPosition(): can't drop above horizontal camera plan !").endl();
            return Ogre::Vector2(FLT_MAX,FLT_MAX);
        }
        Ogre::Vector3 slotWorldPos=camRay.getPoint(result.second);

        long int x=0,y=0;
        mEngine->level()->terrainManager()->terrainGroup()->convertWorldPositionToTerrainSlot(slotWorldPos,&x,&y);
        return Ogre::Vector2(static_cast<float>(x),static_cast<float>(y));
    }

    bool Editor::hitTest(int x,int y, Rocket::Core::String childId)
    {
        Rocket::Core::Element *elem=mDocument;
        if(elem!=NULL)
        {
            if((elem=elem->GetElementById(childId))!=NULL)
            {
                const Rocket::Core::Vector2f &tl=elem->GetAbsoluteOffset(Rocket::Core::Box::PADDING);
                int left=tl.x;
                int top=tl.y;
                const Rocket::Core::Vector2f &box=elem->GetBox(Rocket::Core::Box::BORDER).GetSize(Rocket::Core::Box::BORDER);
                int right=left+box.x;
                int bottom=top+box.y;
                if(x>=left && y>=top && x<=right && y<=bottom)
                    return true;
            }
        }
        return false;
    }

    bool Editor::instanciateResource(File &file)
    {
        Ogre::String intro="Editor::instanciateResource("+file.fullPath()+"): ";
        Ogre::String content=file.read();

        Json::Reader reader;
        Json::Value root;
        bool parsingOk = reader.parse(content, root, false);
        if (!parsingOk)
        {
            Debug::error(intro)("could not parse this:").endl();
            Debug::error(content).endl();
            Debug::error(reader.getFormattedErrorMessages()).endl();
            return false;
        }

        if(!dynamicFillSerialization(root))
        {
            Debug::error(intro)("could not find all values. Aborted.").endl();
            return false;
        }

        // will end up pointing to the agent owning all created models
        Ogre::String aid_s="";
        if(!root.isArray() && !root["aid"].isNull())
            aid_s=root["aid"].asCString();
        AgentId aid=Ogre::StringConverter::parseUnsignedLong(aid_s,INVALID_ID);

        Ogre::String instancitationType=file.extension();
        if(instancitationType=="model")
            loadModelFromSerialization(root,aid);
        else if(instancitationType=="models")
            loadModelsFromSerializations(root,aid);
        else if(instancitationType=="terrain_slot")
            loadTerrainSlotFromSerialization(root);
        else
        {
            Debug::log("instanciation type unknown: ")(instancitationType).endl();
            return false;
        }
        return true;
    }

    bool Editor::loadModelsFromSerializations(Json::Value& root, Steel::AgentId& aid)
    {
        //         Debug::log("Editor::loadModelsFromSerializations(")(root)(")").endl();
        Ogre::String intro="Editor::loadModelsFromSerializations(): ";

        Level *level=mEngine->level();
        if(level==NULL)
        {
            Debug::error(intro)("no level to instanciate stuff in.").endl();
            return false;
        }

        if(!root.isConvertibleTo(Json::arrayValue))
        {
            Debug::warning(intro)("can't use models as array. Trying as simple model instead.").endl();
            return loadModelFromSerialization(root,aid);
        }

        // instanciate all models
        for (Json::ValueIterator it = root.begin(); it != root.end(); ++it)
        {
            Json::Value node=*it;

            if(INVALID_ID==aid)
            {
                Json::Value value=node["modelType"];
                if(value.isNull() || value.asCString()!=modelTypesAsString[MT_OGRE])
                {
                    Debug::error("serialization is not starting with an OgreModel as modelType. Aborted.").endl();
                    return false;
                }
            }

            if(!loadModelFromSerialization(node,aid))
            {
                Debug::error(intro)("could not load models. Aborting.").endl();
                return false;
            }
        }
        return true;
    }

    bool Editor::loadModelFromSerialization(Json::Value& root, Steel::AgentId& aid)
    {
        Debug::log("Editor::loadModelFromSerialization(")(root)(")").endl();
        Ogre::String intro="Editor::loadModelFromSerialization(): ";

        Level *level=mEngine->level();
        if(level==NULL)
        {
            Debug::error(intro)("no level to instanciate stuff in.").endl();
            return false;
        }

        Json::Value value=root["modelType"];
        if(value.isNull())
        {
            Debug::error("serialization is missing a \"modelType\" value. Aborted.").endl();
            return false;
        }

        Ogre::String modelType= value.asString();

        // this is used to know it the agent should be deleted upon failure of the method
        bool fresh_aid=INVALID_ID==aid;
        if(fresh_aid)
        {
            aid=level->newAgent();
            if(INVALID_ID==aid)
            {
                Debug::error(intro)("could not create an agent to link the model to. Aborted.").endl();
                return false;
            }
            Debug::log(intro)("created agent ")(aid).endl();
        }
        // ask the right manager to load this model
        if(modelType=="MT_OGRE")
        {
            if(INVALID_ID!=level->getAgent(aid)->modelId(MT_OGRE))
            {
                Debug::error(intro)("cannot create a second OgreModel to agent ")(aid);
                Debug::error(". Skipping OgreModel instanciation.").endl();
                return true;// skipped, not aborted
            }

            intro.append("in MT_OGRE type: ");
            ModelId mid = level->ogreModelMan()->fromSingleJson(root);
            if(!level->linkAgentToModel(aid,MT_OGRE,mid))
            {
                level->ogreModelMan()->decRef(mid);
                Debug::error(intro)("could not link agent ")(aid)("to OgreModel ");
                Debug::error(mid)(". Model released. Aborted.").endl();
                if(fresh_aid)level->deleteAgent(aid);
                return false;
            }
            else
                Debug::log("new OgreModel with id ")(mid).endl();
            //TODO add visual notification in the UI
        }
        else if(modelType=="MT_PHYSICS")
        {
            if(INVALID_ID!=level->getAgent(aid)->modelId(MT_PHYSICS))
            {
                Debug::error(intro)("cannot create a second PhysicsModel to agent ")(aid);
                Debug::error(". Skipping OgreModel instanciation.").endl();
                return true;// skipped, not aborted
            }

            intro.append("in MT_PHYSICS type: ");
            ModelId mid=level->physicsModelMan()->fromSingleJson(root);
            if(!level->linkAgentToModel(aid,MT_PHYSICS,mid))
            {
                level->physicsModelMan()->decRef(mid);
                Debug::error(intro)("could not link agent ")(aid)("to PhysicsModel ");
                Debug::error(mid)(". Model released. Aborted.").endl();
                if(fresh_aid)level->deleteAgent(aid);
                return false;
            }
            else
                Debug::log("new PhysicsModel with id ")(mid).endl();
            //TODO add visual notification in the UI
        }
        else
            Debug::log(intro)("Unknown model type: ")(modelType).endl();
        return true;
    }

    AgentId Editor::instanciateFromMeshFile(File &meshFile,Ogre::Vector3 &pos,Ogre::Quaternion &rot)
    {
        Ogre::String intro="Editor::instanciateFromMeshFile(";
        Debug::log(intro)(meshFile)(" pos=")(pos)(" rot=")(rot).endl();
        Level *level=mEngine->level();
        if(level==NULL)
        {
            Debug::warning(intro+"): no level to instanciate stuff in.").endl();
            return INVALID_ID;
        }
        //      Ogre::Quaternion r = mEngine->camera()->camNode()->getOrientation();
        Steel::ModelId mid = level->newOgreModel(meshFile.fileName(), pos, rot);
        AgentId aid=level->newAgent();
        if(!level->linkAgentToModel(aid,MT_OGRE,mid))
        {
            Debug::error(intro+"): could not level->linkAgentToModel(")(aid)(", MT_OGRE, ")(mid)(")").endl();
            return INVALID_ID;
        }
        return mid;
    }

    void Editor::loadTerrainSlotFromSerialization(Json::Value &root)
    {
        Ogre::String intro="Editor::loadTerrainSlotFromJson()";

        auto level=mEngine->level();
        if(NULL==level)
        {
            Debug::error(intro)("no level loaded. Aborted.").endl();
            return;
        }

        if(root["slotPosition"].isNull())
        {
            Debug::error(intro)("can't find key \"slotPosition\". Aborted.").endl();
            return;
        }

        TerrainManager::TerrainSlotData slot;
        level->terrainManager()->terrainSlotFromJson(root,slot);
        if(!slot.isValid())
        {
            Debug::error(intro)("TerrainManager::TerrainSlotData is not valid. Serialized string was:");
            Debug::error(root.toStyledString()).endl()("Aborted.").endl();
            return;
        }

        level->terrainManager()->addTerrainSlot(slot);
    }

    bool Editor::keyPressed(const OIS::KeyEvent& evt)
    {
        Rocket::Core::Input::KeyIdentifier keyIdentifier = mUI->keyIdentifiers()[evt.key];
        mContext->ProcessKeyDown(keyIdentifier ,mUI->getKeyModifierState());

        if (evt.text >= 32)
        {
            mContext->ProcessTextInput((Rocket::Core::word) evt.text);
        }
        else if (keyIdentifier == Rocket::Core::Input::KI_RETURN)
        {
            mContext->ProcessTextInput((Rocket::Core::word) '\n');
        }
        return true;
    }

    bool Editor::keyReleased(const OIS::KeyEvent& evt)
    {
        Rocket::Core::Input::KeyIdentifier keyIdentifier = mUI->keyIdentifiers()[evt.key];
        mContext->ProcessKeyUp(keyIdentifier ,mUI->getKeyModifierState());
        Level *level=mEngine->level();

        switch(evt.key)
        {
            case OIS::KC_H:
                mBrush.setMode(EditorBrush::TERRAFORM);
                break;
            case OIS::KC_R:
                if(mInputMan->isKeyDown(OIS::KC_LCONTROL))
                    reloadContent();
                else
                    mBrush.setMode(EditorBrush::ROTATE);
                break;
            case OIS::KC_S:
                if(mInputMan->isKeyDown(OIS::KC_LCONTROL))
                {
                    if(level!=NULL)
                        level->save();
                }
                else
                    mBrush.setMode(EditorBrush::SCALE);
                break;
            case OIS::KC_T:
                mBrush.setMode(EditorBrush::TRANSLATE);
                break;
            case OIS::KC_DELETE:
                mEngine->selectionMan().deleteSelection();
                break;
            default:
                break;
        }

        // numeric key pressed: handles tagging (keys: 1,2,3,...,0)
        if(NULL!=mEngine && evt.key>=OIS::KC_1 && evt.key<=OIS::KC_0)
        {
            // OIS::KC_0 is the highest; the modulo makes it 0
            int tagKey=(evt.key-OIS::KC_1+1)%10;
            Ogre::String sKey=Ogre::StringConverter::toString(tagKey);
            if(mInputMan->isKeyDown(OIS::KC_LCONTROL))
                setSelectionTag(mEngine->selectionMan().selection(),sKey);
            else
                setTaggedSelection(sKey);
        }
        return true;
    }

    void Editor::setSelectionTag(const Selection &selection,const Ogre::String &tag)
    {
        Debug::log("Editor::setSelectionTag(): saving ")(selection)(" under tag ")(tag).endl();
        mSelectionsTags.erase(tag);
        if(selection.size())
            mSelectionsTags.insert(std::pair<Ogre::String,Selection>(tag,selection));
    }

    void Editor::setTaggedSelection(const Ogre::String &tag)
    {
        mEngine->selectionMan().clearSelection();
        auto it=mSelectionsTags.find(tag);
        if(it!=mSelectionsTags.end())
        {
            auto selection=(*it).second;
            Debug::log("Editor::setTaggedSelection(): selecting ")(selection)(" as tag ")(tag).endl();
            mEngine->selectionMan().setSelectedAgents(selection);
        }
    }

    bool Editor::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        if(!hitTest(evt.state.X.abs,evt.state.Y.abs,"menu"))
            mBrush.mousePressed(evt,id);
        return true;
    }

    bool Editor::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        mBrush.mouseReleased(evt,id);
        if(id==OIS::MB_Right)
            mFSResources->expandRows();
        return true;
    }

    bool Editor::mouseMoved(const OIS::MouseEvent& evt)
    {
        if(!mIsDraggingFromMenu && !hitTest(evt.state.X.abs,evt.state.Y.abs,"menu"))
        {
            mBrush.mouseMoved(evt);
            Rocket::Core::Element *elem;

            elem=mDocument->GetElementById("editor_terrabrush_intensity");
            if(elem!=NULL)
                elem->SetInnerRML(Ogre::StringConverter::toString(mBrush.intensity()).c_str());

            elem=mDocument->GetElementById("editor_terrabrush_radius");
            if(elem!=NULL)
                elem->SetInnerRML(Ogre::StringConverter::toString(mBrush.radius()).c_str());
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
        auto elem=(Rocket::Controls::ElementTabSet *)mDocument->GetElementById("editor_tabset");
        if(elem==NULL)return;
        elem->SetActiveTab(Ogre::StringConverter::parseInt(mEngine->config().getSetting("Editor::menuTabIndex"),0));

        mFSResources->refresh(mDocument);
        mDocument->AddEventListener("click",this);
        mDocument->AddEventListener("dragstart",this);
        mDocument->AddEventListener("dragdrop",this);
        mDocument->AddEventListener("change",this);
        mDocument->AddEventListener("submit",this);

        Rocket::Debugger::SetContext(mContext);
        Rocket::Debugger::SetVisible(true);

        // set brush shape
        auto select_form=static_cast<Rocket::Controls::ElementFormControlSelect *>(mDocument->GetElementById("editor_select_terrabrush_shape"));
        if(select_form!=NULL and select_form->GetNumOptions()>0)
        {
            int index=select_form->GetSelection();
            if(index<0)
                select_form->SetSelection(0);
            else
                processCommand(Ogre::String("editorbrush.terrabrush.distribution.")+select_form->GetOption(index)->GetValue().CString());
        }
    }

    void Editor::onHide()
    {
        if(mDocument)
        {
            mDocument->RemoveEventListener("click",this);
            mDocument->RemoveEventListener("dragdrop",this);
            mDocument->RemoveEventListener("change",this);
            mDocument->RemoveEventListener("submit",this);
            // save state
            auto elem=(Rocket::Controls::ElementTabSet *)mDocument->GetElementById("editor_tabset");
            if(NULL!=elem)
            {
                mMenuTabIndex=elem->GetActiveTab();
                mEngine->config().setSetting("Editor::menuTabIndex",Ogre::StringConverter::toString(mMenuTabIndex));
            }
        }
        mBrush.onHide();
    }

    void Editor::ProcessEvent(Rocket::Core::Event &evt)
    {
        if(!isVisible())
            return;
        // create the command
        Rocket::Core::Element *elem=NULL;
        // in case of drag&drop, elem points to the element being dragged
        if(evt=="dragdrop")
        {
            // ok in stable, not in dev
//             elem= static_cast<Rocket::Core::Element *>(evt.GetParameter< void * >("drag_element", NULL));
            Rocket::Core::ElementReference *ref= static_cast<Rocket::Core::ElementReference *>(evt.GetParameter< void * >("drag_element", NULL));
            elem=**ref;
            mIsDraggingFromMenu=false;
        }
        else if(evt=="dragstart")
            mIsDraggingFromMenu=true;
        else
            elem=evt.GetTargetElement();

        if(elem==NULL)
        {
            if(mDebugEvents)Debug::log("Editor::ProcessEvent(): no target element for event of type ")(evt.GetType()).endl();
            return;
        }

        auto etype=evt.GetType();
        Ogre::String event_value=elem->GetAttribute<Rocket::Core::String>("on"+etype,"NoValue").CString();

        if(event_value=="NoValue")
        {
            if(mDebugEvents && elem->GetId()!="")
                Debug::warning("Editor::ProcessEvent(): no event_value for event of type ")(evt.GetType())(" with elem of id ")(elem->GetId()).endl();
            return;
        }

        Ogre::String raw_commmand="";
        if(evt=="dragdrop")
        {
            if(elem->GetId()==mFSResources->GetDataSourceName())
            {
                raw_commmand="instanciate.";
                File file=File(event_value);
                if(!file.exists())
                {
                    Debug::error("Editor::ProcessEvent(): file not found: ")(file).endl();
                    return;
                }
                raw_commmand+=file.fullPath();
            }
            else
            {
                Debug::warning("Editor::ProcessEvent() unknown element source for event of type: ")(evt.GetType());
                Debug::warning(" value: ")(event_value).endl();
                return;
            }
        }
        else if(evt=="click")
        {
            raw_commmand=event_value;
            if(raw_commmand=="engine.set_level")
            {
                auto inputField=(Rocket::Controls::ElementFormControlInput *) mDocument->GetElementById("level_name");
                if(inputField==NULL)
                {
                    Debug::error("Editor::ProcessEvent(): can't find level name input field with id=\"level_name\". Aborted.").endl();
                    return;
                }
                Ogre::String levelName=inputField->GetValue().CString();
                if(levelName=="")
                {
                    Debug::error("Editor::ProcessEvent(): can't create a level with not name. Aborted.").endl();
                    return;
                }
                raw_commmand+="."+levelName;
            }
        }
        else if(evt=="change")
        {
            raw_commmand=event_value;
            if(raw_commmand=="editorbrush.terrabrush.distribution")
            {
                Rocket::Controls::ElementFormControlSelect *form=static_cast<Rocket::Controls::ElementFormControlSelect *>(elem);
                auto optionId=form->GetSelection();
                if(optionId>-1)
                {
                    raw_commmand+=".";
                    raw_commmand+=form->GetValue().CString();
                }
                else
                    return;
            }
        }
        else
        {
            Debug::log("Editor::ProcessEvent(): unknown event ot type:")(evt.GetType())(" and value: ")(event_value).endl();
            return;
        }
//         Debug::log("Editor::ProcessEvent() event type:")(evt.GetType())(" raw_commmand:")(raw_commmand).endl();
        if (raw_commmand.size())
            processCommand(raw_commmand);
    }

    void Editor::processCommand(Ogre::String raw_commmand)
    {
        Debug::log("Editor::processCommand(")(raw_commmand)(")").endl();
        std::vector<Ogre::String> command;
        command=StringUtils::split(std::string(raw_commmand),std::string("."));
        // dispatch the command to the right subprocessing function
        if(command[0]=="editor")
        {
            command.erase(command.begin());
            if(command[0]=="hide")
                mEngine->stopEditMode();
        }
        else if(command[0]=="editorbrush")
        {
            command.erase(command.begin());
            mBrush.processCommand(command);
        }
        else if(command[0]=="engine")
        {
            command.erase(command.begin());
            mEngine->processCommand(command);
        }
        else if(command[0]=="instanciate")
        {
            command.erase(command.begin());
            if(command.size()<1)
            {
                Debug::error("command contains no file !").endl();
                return;
            }
            File file(StringUtils::join(command,"."));
            if(file.exists())
                instanciateResource(file);
            else
                Debug::warning("file \"")(file)("\"not found. Aborted.").endl();
        }
        else if(command[0]=="options")
        {
            command.erase(command.begin());
            if(command.size()==0)
                Debug::warning("Editor::processCommand(): no option given.").endl();
            else
                processOptionCommand(command);
        }
        else if(command[0]=="resourceGroupsInfos")
            OgreUtils::resourceGroupsInfos();
        else
            Debug::warning("Editor::processCommand(): unknown command: ")(command).endl();
    }

    void Editor::processOptionCommand(std::vector<Ogre::String> command)
    {
        if(command.size()==0)
            return;
        if(command[0]=="switch_debug_events")
        {
            mDebugEvents=!mDebugEvents;
            Debug::log("flag DebugEvent ")(mDebugEvents?"activated":"deactivated").endl();
        }
        else
            Debug::warning("Editor::processOptionCommand(): unknown command: ")(command).endl();

    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 





