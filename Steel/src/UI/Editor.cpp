
#include "UI/Editor.h"
#include "Debug.h"
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
        Rocket::Core::String msg=evt.GetTargetElement()->GetAttribute<Rocket::Core::String>("on"+evt.GetType(),"no iop");
        Debug::log("Editor::ProcessEvent()")(msg)(" ")(evt.GetType()).endl();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


