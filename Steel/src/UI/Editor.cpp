
#include "UI/Editor.h"
#include "Debug.h"
#include <tools/StringUtils.h>
#include <Rocket/Core/Factory.h>

namespace Steel
{
    Editor::Editor():UIPanel("Editor","data/ui/current/editor/editor.rml")
    {
#ifdef DEBUG
        mAutoReload=true;
#endif
    }

    Editor::Editor(const Editor& other)
    {

    }

    Editor::~Editor()
    {

    }

    Editor& Editor::operator=(const Editor& other)
    {
        return *this;
    }

    void Editor::init(unsigned int width, unsigned int height, Engine *engine, UI * ui)
    {
        UIPanel::init(width,height);
        
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
//         Debug::log("Editor::ProcessEvent()")(msg)(":")(evt.GetType()).endl();
        if(command[0]=="engine")
        {
            command.erase(command.begin());
            processEngineCommands(command);
        }
    }
    
    void Editor::processEngineCommands(std::vector<Ogre::String> command)
    {
        
        if(command[0]=="new_level")
        {
//             mDocument->GetElementById("")
        }
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


