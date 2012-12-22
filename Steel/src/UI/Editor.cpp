
#include <Rocket/Core/Factory.h>
#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Debugger.h>
#include <Rocket/Core/Element.h>

#include "UI/Editor.h"
#include "Debug.h"
#include "tools/StringUtils.h"
#include "Level.h"
#include "Engine.h"
#include "UI/FileSystemDataSource.h"
#include <Camera.h>

namespace Steel
{
    Editor::Editor():UIPanel("Editor","data/ui/current/editor/editor.rml"),
        mEngine(NULL),mUI(NULL),mFSResources(NULL),mDataDir()
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
        mEngine=NULL;
        mUI=NULL;
        mFSResources=NULL;
    }

    Editor& Editor::operator=(const Editor& other)
    {
        throw std::runtime_error("Editor::operator=(const Editor& other): Not Implemented");
        return *this;
    }

    void Editor::init(unsigned int width, unsigned int height, Engine *engine, UI *ui)
    {
        mDataDir=ui->dataDir().subfile("editor").fullPath();
        Debug::log("Editor::init()").endl();
        auto resGroupMan=Ogre::ResourceGroupManager::getSingletonPtr();
        // true is for recursive search. Add to this resources.cfg
        resGroupMan->addResourceLocation(mDataDir.fullPath(), "FileSystem", "UI",true);
        //resGroupMan->addResourceLocation(mDataDir.subfile("images").fullPath(), "FileSystem", "UI",true);
        //resGroupMan->declareResource("inode-directory.png","Texture","UI");

        mFSResources=new FileSystemDataSource("models",engine->rootDir().subfile("data").subfile("models"));

        UIPanel::init(width,height);
        mEngine=engine;
        mUI=ui;
        auto elem=(Rocket::Controls::ElementFormControlInput *)mDocument->GetElementById("level_name");
        if(elem!=NULL)
        {
            elem->SetValue("MyLevel");
            // does not work for some reason
//             elem->AddEventListener("submit",this);
        }
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
        mFSResources->refresh();
        //debug
        auto elem=(Rocket::Controls::ElementFormControlInput *)mDocument->GetElementById("editor_menu_tab_edit");
        if(elem!=NULL)
            elem->Click();
//         processCommand("engine.level.instanciate.resource./media/a0/cpp/1210/usmb/install_dir/data/resources/meshes/seaweed_00.mesh");
    }

    void Editor::onHide()
    {
        mDocument->RemoveEventListener("click",this);
        mDocument->RemoveEventListener("dragdrop",this);
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
            if(elem->GetId()==mFSResources->GetDataSourceName())
            {
                raw_commmand="engine.level.instanciate.resource.";
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
        std::vector<Ogre::String> command;
        command=StringUtils::split(std::string(raw_commmand),std::string("."));
        // dispatch the command to the right subprocessing function
        if(command[0]=="engine")
        {
            command.erase(command.begin());
            processEngineCommand(command);
        }
        else
        {
            Debug::log("Editor::processCommand(): unknown command: ")(command).endl();
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
            Ogre::String intro="Editor::processLevelCommand(): ";
            if(command[1]=="resource")
            {
                if(command.size()<3)
                {
                    Debug::error(intro+" command conains no file !").endl();
                    return;
                }
                File file=File(StringUtils::join(command,".",2));
                if(!file.exists())
                {
                    Debug::error(intro+" no such file ! ")(file).endl();
                    return;
                }
                if(file.extension()=="mesh")
                {
                    Debug::log("instanciating ")(file)("...").endl();


                    Ogre::Vector3 pos=mEngine->camera()->dropTargetPosition();
                    Ogre::Quaternion rot=mEngine->camera()->dropTargetRotation();
                    ModelId mid=instanciateFromMeshFile(file,pos,rot);
                    if(mid==Steel::INVALID_ID)
                        return;
                }
                else
                {
                    Debug::log(intro+"instanciation file ext unknown for file ")(file).endl();
                    return;
                }
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


