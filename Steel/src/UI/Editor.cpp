
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
#include <tools/OgreUtils.h>
#include "Level.h"
#include "Engine.h"
#include "UI/FileSystemDataSource.h"
#include <Camera.h>
#include <OgreModelManager.h>
#include <Agent.h>

namespace Steel
{
    Editor::Editor():UIPanel("Editor","data/ui/current/editor/editor.rml"),
        mEngine(NULL),mUI(NULL),mInputMan(NULL),mFSResources(NULL),mDataDir(),
        mMenuTabIndex(1),mBrush(),mDebugEvents(false)
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
        mBrush.shutdown();

        delete mFSResources;
        mFSResources=NULL;

        mEngine=NULL;
        mUI=NULL;
        mInputMan=NULL;
    }

    Editor& Editor::operator=(const Editor& other)
    {
        throw std::runtime_error("Editor::operator=(const Editor& other): Not Implemented");
        return *this;
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
        auto elem=(Rocket::Controls::ElementFormControlInput *)mDocument->GetElementById("level_name");
        if(elem!=NULL)
        {
            elem->SetValue("MyLevel");
            // does not work for some reason
//             elem->AddEventListener("submit",this);
        }
        mBrush.init(engine,this,mInputMan);
        // brush shape
        auto form=static_cast<Rocket::Controls::ElementFormControlSelect *>(mDocument->GetElementById("editor_select_terrabrush_shape"));
        if(form!=NULL and form->GetNumOptions()>0)
            form->SetSelection(0);
    }

    bool Editor::dynamicFillSerialization(Json::Value& root)
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
            for(auto it=names.begin(); it!=names.end(); ++it)
            {
                key=*it;
                Ogre::String svalue,new_svalue;
                svalue=new_svalue=root[key].asString();

                if(svalue.at(0)!='$')
                    new_svalue=svalue;
                else if(svalue=="$dropTargetPosition")
                    new_svalue=Ogre::StringConverter::toString(getDropTargetPosition());
                else if(svalue=="$dropTargetRotation")
                    new_svalue=Ogre::StringConverter::toString(getDropTargetRotation());
                else if(svalue=="$agentUnderMouse")
                {
                    auto selection=std::list<ModelId>();
                    mEngine->pickAgents(selection, mInputMan->mousePos().x,mInputMan->mousePos().y);

                    if(selection.size()>0)
                        new_svalue=Ogre::StringConverter::toString(selection.front());
                    else
                        new_svalue=Json::Value().asString();
                }
                else if(svalue=="$slotDropPosition")
                {
                    auto slotPosition=getSlotDropPosition();
                    // drops farther than 10km away are forbidden
                    if(slotPosition.length()>10000)
                    {
                        Debug::error(intro)("slot drop position is invalid (>10km away):")(slotPosition)(". Aborting.").endl();
                        return false;
                    }
                    new_svalue=Ogre::StringConverter::toString(slotPosition);
                }

                root[key]=new_svalue.c_str();
            }
        }
        else
        {
            Debug::error(intro)(" unrecognized value type. Value was:")(root).endl()("Aborting.").endl();
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
            Debug::error(intro)("could not find all values. Aborting.").endl();
            return false;
        }

        Ogre::String instancitationType=file.extension();
        if(instancitationType=="model")
            loadModelFromSerialization(root);
        else if(instancitationType=="terrain_slot")
            loadTerrainSlotFromSerialization(root);
        else
        {
            Debug::log("instanciation type unknown: ")(instancitationType).endl();
            return false;
        }
        return true;
    }

    void Editor::loadModelFromSerialization(Json::Value &root)
    {
        Debug::log("Editor::loadModelFromSerialization(")(root)(")").endl();

        Level *level=mEngine->level();
        Ogre::String intro="Editor::loadModelFromSerialization(): ";
        if(level==NULL)
        {
            Debug::error(intro)("no level to instanciate stuff in.").endl();
            return;
        }

        Json::Value value=root["modelType"];
        if(value.isNull())
        {
            Debug::error("serialization is missing a \"modelType\" value. Aborting.").endl();
            return;
        }

        Ogre::String modelType= value.asString();
        // ask the right manager to load this model
        if(modelType=="MT_OGRE")
        {
            ModelId mid = level->ogreModelMan()->fromSingleJson(root);
            if(mid==INVALID_ID)
            {
                Debug::error(intro)("got invalid model id for content:")(value).endl()("Aborting.").endl();
                return;
            }

            AgentId aid=level->newAgent();
            if(aid==INVALID_ID)
            {
                level->ogreModelMan()->releaseModel(mid);
                Debug::error(intro)("could not create an agent to link the model to ! Aborting (model was released).").endl();
                return;
            }

            if(!level->linkAgentToModel(aid,MT_OGRE,mid))
            {
                level->ogreModelMan()->releaseModel(mid);
                level->deleteAgent(aid);
                Debug::error(intro)("could not link agent ")(aid)("to model ")(mid)(" ! Aborting (model was released, agent deleted).").endl();
                return;
            }
            //TODO add visual notification in the UI
        }
        else if(modelType=="MT_PHYSICS")
        {
//             TODO: invoke physics modelMan, create bullet instance, attach it to ogre model
        }
        else
            Debug::log("Unknown model type: ")(modelType).endl();
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
            Debug::error(intro)("no level loaded. Aborting.").endl();
            return;
        }

        if(root["slotPosition"].isNull())
        {
            Debug::error(intro)("can't find key \"slotPosition\". Aborting.").endl();
            return;
        }

        TerrainManager::TerrainSlotData slot;
        level->terrainManager()->terrainSlotFromJson(root,slot);
        if(!slot.isValid())
        {
            Debug::error(intro)("TerrainManager::TerrainSlotData is not valid. Serialized string was:");
            Debug::error(root.toStyledString()).endl()("Aborting.").endl();
            return;
        }

        level->terrainManager()->addTerrainSlot(slot);
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
        return true;
    }

    bool Editor::mouseMoved(const OIS::MouseEvent& evt)
    {
        if(!hitTest(evt.state.X.abs,evt.state.Y.abs,"menu"))
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
        elem->SetActiveTab(mMenuTabIndex);

        mFSResources->refresh();
        mDocument->AddEventListener("click",this);
        mDocument->AddEventListener("dragdrop",this);
        mDocument->AddEventListener("change",this);
        mDocument->AddEventListener("submit",this);

        Rocket::Debugger::SetContext(mContext);
        Rocket::Debugger::SetVisible(true);
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
                mMenuTabIndex=elem->GetActiveTab();
        }
        mBrush.onHide();
    }

    void Editor::ProcessEvent(Rocket::Core::Event &evt)
    {
        // create the command
        Rocket::Core::Element *elem=NULL;
        // in case of drag&drop, elem points to the element being dragged
        if(evt=="dragdrop")
        {
            // ok in stable, not in dev
//             elem= static_cast<Rocket::Core::Element *>(evt.GetParameter< void * >("drag_element", NULL));
            // ok in dev, but dev has a corrupt stack on exit
            Rocket::Core::ElementReference *ref= static_cast<Rocket::Core::ElementReference *>(evt.GetParameter< void * >("drag_element", NULL));
            elem=**ref;
        }
        else
            elem=evt.GetTargetElement();

        if(elem==NULL)
        {
            Debug::log("Editor::ProcessEvent(): no target element for event of type ")(evt.GetType()).endl();
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
                    Debug::error("Editor::ProcessEvent(): can't find level name input field with id=\"level_name\". Aborting.").endl();
                    return;
                }
                Ogre::String levelName=inputField->GetValue().CString();
                if(levelName=="")
                {
                    Debug::error("Editor::ProcessEvent(): can't create a level with not name. Aborting.").endl();
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
                    raw_commmand+=form->GetOption(optionId)->GetValue().CString();
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
                Debug::warning("file \"")(file)("\"not found. Aborting.").endl();
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



