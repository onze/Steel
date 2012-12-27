
#include <json/json.h>

#include <Rocket/Core/Factory.h>
#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Controls/ElementTabSet.h>
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

namespace Steel
{
    Editor::Editor():UIPanel("Editor","data/ui/current/editor/editor.rml"),
        mEngine(NULL),mUI(NULL),mInputMan(NULL),
        mFSModels(NULL),mDataDir(),mSelectionPosBeforeTransformation(Ogre::Vector3::ZERO),
        mMenuTabIndex(0),mEditMode(TRANSLATE),mIsDraggingSelectionCancelled(false),mIsDraggingSelection(false)
    {
#ifdef DEBUG
        mAutoReload=true;
        mMenuTabIndex=1;
#endif
    }

    Editor::Editor(const Editor& other)
    {
        throw std::runtime_error("Editor::Editor(const Editor& other): Not Implemented");
    }

    Editor::~Editor()
    {
        mEngine=NULL;
        mUI=NULL;
        mFSModels=NULL;
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

        mFSModels=new FileSystemDataSource("models",engine->rootDir().subfile("data").subfile("models"));
        UIPanel::init(width,height);
        mEngine=engine;
        mUI=ui;
        mInputMan=inputMan;
        auto elem=(Rocket::Controls::ElementFormControlInput *)mDocument->GetElementById("level_name");
        if(elem!=NULL)
        {
            elem->SetValue("MyLevel");
            // does not work for some reason
//             elem->AddEventListener("submit",this);
        }
        mIsDraggingSelectionCancelled=mIsDraggingSelection=false;
    }

    void Editor::onFileChangeEvent(File *file)
    {
        UIPanel::onFileChangeEvent(file);
        Rocket::Debugger::SetContext(mContext);
        Rocket::Debugger::SetVisible(true);
    }

    void Editor::onShow()
    {
        mDocument->AddEventListener("click",this);
        mDocument->AddEventListener("dragdrop",this);
        Rocket::Debugger::SetContext(mContext);
        Rocket::Debugger::SetVisible(true);
        mFSModels->refresh();

        // (re)load state
        auto elem=(Rocket::Controls::ElementTabSet *)mDocument->GetElementById("editor_tabset");
        if(elem==NULL)return;
        elem->SetActiveTab(mMenuTabIndex);
    }

    void Editor::onHide()
    {
        mDocument->RemoveEventListener("click",this);
        mDocument->RemoveEventListener("dragdrop",this);

        // save state
        auto elem=(Rocket::Controls::ElementTabSet *)mDocument->GetElementById("editor_tabset");
        if(elem==NULL)return;
        mMenuTabIndex=elem->GetActiveTab();
    }

    bool Editor::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {

        std::list<ModelId> selection;
        switch (id)
        {
            case OIS::MB_Left:
                if (mEngine->hasSelection())
                {
                    mEngine->clearSelection();
                }
                mEngine->pickAgents(selection, evt.state.X.abs, evt.state.Y.abs);
                mEngine->setSelectedAgents(selection, true);
                if (mEngine->hasSelection())
                {
                    // saved so that we know what to reset properties to
                    //TODO: save complete serialisations
                    mSelectionPosBeforeTransformation = mEngine->selectionPosition();
                }
                break;
            case OIS::MB_Right:
                // cancel current selection dragging (translate, etc)
                if(mIsDraggingSelection)
                {
                    mIsDraggingSelectionCancelled=true;
                    mEngine->setSelectionPosition(mSelectionPosBeforeTransformation);
                }
                break;
            default:
                break;
        }
        return true;
    }

    bool Editor::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        if(id==OIS::MB_Left)
        {
            mIsDraggingSelectionCancelled=false;
            mIsDraggingSelection=false;
            mSelectionPosBeforeTransformation = mEngine->selectionPosition();
        }
        return true;
    }

    bool Editor::mouseMoved(const OIS::MouseEvent& evt)
    {
        if(!evt.state.buttonDown(OIS::MB_Left))
            return true;
        if (mEngine->hasSelection())
        {
            if(mIsDraggingSelectionCancelled)
                return true;
            mIsDraggingSelection=true;
            Ogre::Real x = float(evt.state.X.abs);
            Ogre::Real y = float(evt.state.Y.abs);
            Ogre::Real _x = x-float(evt.state.X.rel);
            Ogre::Real _y = y-float(evt.state.Y.rel);
            Ogre::Real w = float(mWidth);
            Ogre::Real h = float(mHeight);
            switch (mEditMode)
            {
                case TRANSLATE:
                    // translating with shift held: along Y axis
                    if (mInputMan->isKeyDown(OIS::KC_LSHIFT))
                    {
                        Ogre::Vector3 selectionPos = mEngine->selectionPosition();
                        // I have not found a faster way to do this:
                        // first I build a plan with the camera orientation as normal (but vertical).
                        Ogre::Vector3 normal = mEngine->camera()->camNode()->getOrientation()* Ogre::Vector3::UNIT_Z;
                        normal.y = .0f;
                        Ogre::Plane plane = Ogre::Plane(normal, Ogre::Vector3::ZERO);
                        // and since I don't know how to compute the distance to feed it, I let it at (0,0,0).
                        // ask IT its distance to where I wanted to put it (the selection),
                        Ogre::Real dist = plane.getDistance(selectionPos);
                        // and build a new one there. suboptimal =/
                        plane = Ogre::Plane(normal, dist);
                        plane.normalise();
                        // the idea here is to move the selection on a plane perpendicular to the camera view.
                        // we want a vector of the translation from src to dst, where src is where a ray cast from the camera
                        // to the last mouse coordinates hits the mentionned plane, and dst is the same with a ray
                        // passing through the current mouse coordinates.
                        Ogre::Ray ray = mEngine->camera()->cam()->getCameraToViewportRay(_x / w, _y / h);
                        std::pair<bool, Ogre::Real> result = ray.intersects(plane);
                        if (result.first)
                        {
                            Ogre::Vector3 src = ray.getPoint(result.second);
                            // then we do the same with the new coordinates on the viewport
                            ray = mEngine->camera()->cam()->getCameraToViewportRay(x / w, y / h);
                            result = ray.intersects(plane);
                            if (result.first)
                            {
                                Ogre::Vector3 dst = ray.getPoint(result.second);
                                // finally, we translate the selection according to the vector given by substracting two points.
                                Ogre::Vector3 t = dst - src;
                                t.y = t.y > 10.f ? .0f : t.y < -10.f ? 0.f : t.y;
                                mEngine->translateSelection(Ogre::Vector3::UNIT_Y * t);
                            }
                        }
                    }
                    else
                    {
                        Ogre::Vector3 selectionPos = mEngine->selectionPosition();
                        // normal translation: on the x/z plane
                        Ogre::Plane plane = Ogre::Plane(Ogre::Vector3::UNIT_Y, selectionPos.y);
                        plane.normalise();
                        // what we want is a vector of translation from the selection's position to a new one.
                        // first we see where falls a ray that we cast from the cam to the last coordinates on the viewport
                        // (the idea is to cast a ray from the camera to a horizontal plane at the base of the selection)
                        Ogre::Ray ray = mEngine->camera()->cam()->getCameraToViewportRay(_x / w, _y / h);
                        std::pair<bool, Ogre::Real> result = ray.intersects(plane);
                        if (result.first)
                        {
                            Ogre::Vector3 src = ray.getPoint(result.second);
                            // then we do the same with the new coordinates on the viewport
                            ray = mEngine->camera()->cam()->getCameraToViewportRay(x / w, y / h);
                            result = ray.intersects(plane);
                            if (result.first)
                            {
                                Ogre::Vector3 dst = ray.getPoint(result.second);
                                // finally, we translate the selection according to the vector given by substracting two points.
                                Ogre::Vector3 t = dst - src;
                                // just making sure we have an horizontal translation (should be useless since plane is horizontal)
                                t.y = 0.f;
                                mEngine->translateSelection(t);
                            }
                        }
                    }
                    break;
                case ROTATE:
                    break;
                case SCALE:
                    break;
                default:
                    break;
            } //end of switch (mEditMode)
        } //end of if (mEngine->hasSelection())
        return true;
    }

    void Editor::ProcessEvent(Rocket::Core::Event& evt)
    {
        // create the command
        Rocket::Core::Element *elem;
        // in case of drag&drop, elem points to the element being dragged
        if(evt=="dragdrop")
            elem = static_cast< Rocket::Core::Element* >(evt.GetParameter< void* >("drag_element", NULL));
        else
            elem=evt.GetTargetElement();

        if(elem==NULL)
            return;

        Ogre::String raw_commmand="";
        Ogre::String event_value=elem->GetAttribute<Rocket::Core::String>("on"+evt.GetType(),"").CString();

        if(evt=="dragdrop")
        {
            if(elem->GetId()==mFSModels->GetDataSourceName())
            {
                raw_commmand="engine.level.instanciate.model.";
                raw_commmand+=event_value;
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
            if(raw_commmand=="engine.level.load")
            {
                auto inputField=(Rocket::Controls::ElementFormControlInput *) mDocument->GetElementById("level_name");
                if(inputField==NULL)
                {
                    Debug::error("Editor::ProcessEvent(): can't find level name input field with id=\"level_name\". Aborting.").endl();
                    return;
                }
                Ogre::String levelName=inputField->GetValue().CString();
                raw_commmand+="."+levelName;
            }
        }
        else
        {
            Debug::log("Editor::ProcessEvent(): unknown event ot type:")(evt.GetType())(" and value: ")(event_value).endl();
            return;
        }
//         Debug::log("Editor::ProcessEvent() event type:")(evt.GetType())(" msg:")(msg)(" command:")(command).endl();
        if (raw_commmand.size())
            processCommand(raw_commmand);
    }

    void Editor::processCommand(Ogre::String raw_commmand)
    {
        Debug::log("Editor::processCommand(")(raw_commmand)(")").endl();
        std::vector<Ogre::String> command;
        command=StringUtils::split(std::string(raw_commmand),std::string("."));
        // dispatch the command to the right subprocessing function
        if(command[0]=="engine")
        {
            command.erase(command.begin());
            processEngineCommand(command);
        }
        else if(command[0]=="resourceGroupsInfos")
        {
            OgreUtils::resourceGroupsInfos();
        }
        else if(command[0]=="editmode")
        {
            command.erase(command.begin());
            if(command[0]=="translate")
                mEditMode=TRANSLATE;
            else if(command[0]=="rotate")
                mEditMode=ROTATE;
            else if(command[0]=="scale")
                mEditMode=SCALE;
        }
        else
        {
            Debug::warning("Editor::processCommand(): unknown command: ")(command).endl();
        }
    }

    void Editor::processEngineCommand(std::vector<Ogre::String> command)
    {
        if(command.size()==0)
            return;
        if(command[0]=="level")
        {
            command.erase(command.begin());
            processLevelCommand(command);
        }
        else
        {
            Debug::log("Editor::processEngineCommand(): unknown command: ")(command).endl();
        }
    }

    void Editor::processLevelCommand(std::vector<Ogre::String> command)
    {
        if(command.size()==0)
            return;
        if(command[0]=="load")
        {
            auto intro="Editor::processLevelCommand(): command="+StringUtils::join(command,".")+", ";
            Ogre::String levelName=StringUtils::join(command,".",1);

            if(levelName.length()==0)
            {
                Debug::warning(intro)("level has no name ! aborted.").endl();
                return;
            }
            Debug::log(intro)(levelName).endl();
            Level *mLevel = mEngine->createLevel(levelName);
            mLevel->load();
        }
        else if(command[0]=="save")
        {
            Level *level=mEngine->level();
            if(level==NULL)
            {
                Debug::warning("Editor::processLevelCommand(): command="+StringUtils::join(command,".")+": no level to save.").endl();
                return;
            }
            level->save();
        }
        else if(command[0]=="delete")
        {
            Debug::error("to be implemented: level deletion").endl();
        }
        else if(command[0]=="instanciate")
        {
            command.erase(command.begin());
            Ogre::String intro="Editor::processLevelCommand() instanciate: ";
            if(command[0]=="model")
            {
                command.erase(command.begin());
                if(command.size()<1)
                {
                    Debug::error(intro+"command contains no file !").endl();
                    return;
                }
                File file=File(StringUtils::join(command,"."));
                if(!file.exists())
                {
                    Debug::error(intro+"no such file ! ")(file).endl();
                    return;
                }
                loadModelFromFile(file);
            }
            else
            {
                Debug::log(intro+"instanciation type unknown: ")(command).endl();
                return;
            }
        }
        else
        {
            Debug::log("Editor::processLevelCommand(): unknown command: ")(command).endl();
        }
    }

    void Editor::loadModelFromFile(File &file)
    {
        Debug::log("Editor::loadModelFromFile(")(file)(")").endl();
        Ogre::String content=file.read();
        Debug::log(content).endl();

        Json::Reader reader;
        Json::Value root;
        bool parsingOk = reader.parse(content, root, false);
        if (!parsingOk)
        {
            Debug::error("could not parse this:").endl();
            Debug::error(content).endl();
            Debug::error(reader.getFormattedErrorMessages()).endl();
            return;
        }

        if(root.size()==0)
        {
            Debug::error("item has size 0, is it an array ?").endl()(root.toStyledString()).endl();
            return;
        }

        Json::Value jsonModel=root[0u];
        if(jsonModel.isNull())
        {
            Debug::error("root[0] is null, is it defined ?").endl()(root.toStyledString()).endl();
            return;
        }
//         Debug::log(root).endl();

        Json::Value jsonAttr=jsonModel["modelType"];
        if(jsonAttr.isNull())
        {
            Debug::error("template is missing a \"modelType\" value.").endl();
            return;
        }
        Ogre::String modelType= jsonAttr.asString();


        // fill dynamic values
        jsonModel.removeMember("modelType");
        auto names=jsonModel.getMemberNames();
        Json::Value updatedModel;
        Ogre::String key,value,new_value;
        for(auto it=names.begin(); it!=names.end(); ++it)
        {
            key=*it;
            value=jsonModel[key].asString();
            Debug::log(key)(":")(value);
            new_value="";
            if(value.at(0)!='$')
            {
                new_value=value;
            }
            else if(value=="$dropTargetPosition")
            {
                Ogre::Vector3 pos=mEngine->camera()->dropTargetPosition();
                new_value=Ogre::StringConverter::toString(pos);
            }
            else if(value=="$dropTargetRotation")
            {
                Ogre::Quaternion rot=mEngine->camera()->dropTargetRotation();
                new_value=Ogre::StringConverter::toString(rot);
            }
            updatedModel[key]=new_value.c_str();
            Debug::log(" -> ")(updatedModel[key].asString()).endl();
        }
        root[0u]=updatedModel;

        content=root.toStyledString();
        Debug::log("->").endl()(content).endl();
        // ask the right manager to load this model
        if(modelType=="MT_OGRE")
        {
            Level *level=mEngine->level();
            if(level==NULL)
            {
                Debug::warning("Editor::loadModelFromFile(): no level to instanciate stuff in.").endl();
                return;
            }
            auto mids=level->ogreModelMan()->fromJson(root);
            ModelId mid = mids[0];
            if(mid==INVALID_ID)
                return;
            AgentId aid=level->newAgent();
            if(aid==INVALID_ID)
                return;
            if(!level->linkAgentToModel(aid,MT_OGRE,mid))
                return;
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
        Steel::ModelId mid = level->newOgreModel(meshFile.fileName(), pos, rot, true);
        AgentId aid=level->newAgent();
        if(!level->linkAgentToModel(aid,MT_OGRE,mid))
        {
            Debug::error(intro+"): could not level->linkAgentToModel(")(aid)(", MT_OGRE, ")(mid)(")").endl();
            return INVALID_ID;
        }
        return mid;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


