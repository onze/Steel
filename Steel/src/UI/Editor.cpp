
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

namespace Steel
{
    Editor::Editor():UIPanel("Editor","data/ui/current/editor/editor.rml"),
        mEngine(NULL),mUI(NULL),mInputMan(NULL),mFSResources(NULL),mDataDir(),
        mMenuTabIndex(0),mBrush()
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

        mFSResources=new FileSystemDataSource("resources",engine->rootDir().subfile("data").subfile("models"));
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
        mBrush.init(engine,this,mInputMan);
        // brush shape
        auto form=static_cast<Rocket::Controls::ElementFormControlSelect *>(mDocument->GetElementById("editor_select_terrabrush_shape"));
        if(form!=NULL and form->GetNumOptions()>0)
            form->SetSelection(0);
    }

    void Editor::onFileChangeEvent(File *file)
    {
        Debug::log("Editor::onFileChangeEvent(): ")(file->fullPath()).endl();
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
        mDocument->RemoveEventListener("click",this);
        mDocument->RemoveEventListener("dragdrop",this);
        mDocument->RemoveEventListener("change",this);
        mDocument->RemoveEventListener("submit",this);

        // save state
        auto elem=(Rocket::Controls::ElementTabSet *)mDocument->GetElementById("editor_tabset");
        if(elem==NULL)return;
        mMenuTabIndex=elem->GetActiveTab();
        mBrush.onHide();
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

    void Editor::ProcessEvent(Rocket::Core::Event &evt)
    {
        // create the command
        Rocket::Core::Element *elem=NULL;
        // in case of drag&drop, elem points to the element being dragged
        if(evt=="dragdrop")
        {
            // ok in stable, not in dev
            elem= static_cast<Rocket::Core::Element *>(evt.GetParameter< void * >("drag_element", NULL));
            // ok in dev, but dev has a corrupt stack on exit
//             Rocket::Core::ElementReference *ref= static_cast<Rocket::Core::ElementReference *>(evt.GetParameter< void * >("drag_element", NULL));
//             elem=**ref;
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
            if(elem->GetId()!="")
                Debug::log("Editor::ProcessEvent(): no event_value for event of type ")(evt.GetType())(" with elem of id ")(elem->GetId()).endl();
            return;
        }

        Ogre::String raw_commmand="";
        if(evt=="dragdrop")
        {
            if(elem->GetId()==mFSResources->GetDataSourceName())
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
        if(command[0]=="engine")
        {
            command.erase(command.begin());
            processEngineCommand(command);
        }
        else if(command[0]=="resourceGroupsInfos")
        {
            OgreUtils::resourceGroupsInfos();
        }
        else if(command[0]=="editorbrush")
        {
            command.erase(command.begin());
            mBrush.processCommand(command);
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
            Level *mLevel=mEngine->level();
            bool backup;
            if((backup=(mLevel!=NULL)))
            {
                // unload all resources that require a level
                mBrush.shutdown();
            }
            mLevel = mEngine->createLevel(levelName);
            mLevel->load();
            if(backup)
            {
                mBrush.init(mEngine,this,mInputMan);
            }
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
        Steel::ModelId mid = level->newOgreModel(meshFile.fileName(), pos, rot);
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



