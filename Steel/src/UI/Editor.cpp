
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

    void Editor::init(unsigned int width, unsigned int height, Engine *engine, UI * ui)
    {
        mDataDir=ui->dataDir().subfile("editor").fullPath();
        Debug::log("Editor::init()").endl();
        auto resGroupMan=Ogre::ResourceGroupManager::getSingletonPtr();
        // true is for recursive search. Add to this resources.cfg
        resGroupMan->addResourceLocation(mDataDir.fullPath(), "FileSystem", "UI",true);
        //resGroupMan->addResourceLocation(mDataDir.subfile("images").fullPath(), "FileSystem", "UI",true);
        //resGroupMan->declareResource("inode-directory.png","Texture","UI");

        mFSResources=new FileSystemDataSource("resources",engine->rootDir().subfile("data").subfile("resources"));

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
        Rocket::Debugger::SetContext(mContext);
        Rocket::Debugger::SetVisible(true);
        mFSResources->refresh();
        //debug
        auto elem=(Rocket::Controls::ElementFormControlInput *)mDocument->GetElementById("editor_menu_tab_edit");
        if(elem!=NULL)
            elem->Click();
    }

    void Editor::onHide()
    {
        mDocument->RemoveEventListener("click",this);
    }

    void Editor::ProcessEvent(Rocket::Core::Event& evt)
    {
        if(!isVisible())
            return;
        Rocket::Core::String msg=evt.GetTargetElement()->GetAttribute<Rocket::Core::String>("on"+evt.GetType(),"");
        std::vector<Ogre::String> command=StringUtils::split(std::string(msg.CString()),std::string("."));
        if(command[0]=="engine")
        {
            command.erase(command.begin());
            processEngineCommands(command,&evt);
        }
        else
        {
            Debug::log("Editor::ProcessEvent() event value:")(msg)(", event type:")(evt.GetType()).endl();
        }
    }

    void Editor::processEngineCommands(std::vector<Ogre::String> command, Rocket::Core::Event *evt)
    {
        if(command.size()==0)
            return;
        if(command[0]=="level")
        {
            command.erase(command.begin());
            processLevelCommands(command,evt);
        }
    }

    void Editor::processLevelCommands(std::vector<Ogre::String> command, Rocket::Core::Event *evt)
    {
        if(command.size()==0)
            return;
        if(command[0]=="load")
        {
            auto intro="Editor::processEngineCommands() new level: ";
            auto inputField=(Rocket::Controls::ElementFormControlInput *) mDocument->GetElementById("new_level_name");
            Ogre::String levelName=inputField->GetValue().CString();

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
                return;
            level->save();
        }
        else if(command[0]=="delete")
        {
            Debug::error("to be implemented: level deletion").endl();
        }
        else
            Debug::log("Editor::processLevelCommands(): unknown command: ")(command).endl();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


