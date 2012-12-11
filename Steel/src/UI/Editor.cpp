
#include <Rocket/Core/Factory.h>
#include <Rocket/Controls/ElementFormControlInput.h>

#include "UI/Editor.h"
#include "Debug.h"
#include "tools/StringUtils.h"
#include "Level.h"
#include "Engine.h"

namespace Steel
{
    Editor::Editor():UIPanel("Editor","data/ui/current/editor/editor.rml"),
        mEngine(NULL),mUI(NULL)
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
    }

    Editor& Editor::operator=(const Editor& other)
    {
        return *this;
    }

    void Editor::init(unsigned int width, unsigned int height, Engine *engine, UI * ui)
    {
        UIPanel::init(width,height);
        mEngine=engine;
        mUI=ui;
        auto elem=(Rocket::Controls::ElementFormControlInput *)mDocument->GetElementById("new_level_name");
        if(elem!=NULL)
        {
            elem->SetValue("MyLevel");
            // does not work for some reason
//             elem->AddEventListener("submit",this);
        }
    }

    void Editor::onShow()
    {
        mDocument->AddEventListener("click",this);
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

        if(command[0]=="new_level")
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
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


