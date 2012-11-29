
#include "UI/Editor.h"
#include "Debug.h"
#include <Rocket/Core/Factory.h>

namespace Steel
{
    Editor::Editor():UIPanel("Editor","data/ui/current/editor/editor.rml")
    {

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
        Rocket::Core::String msg=evt.GetTargetElement()->GetAttribute<Rocket::Core::String>("on"+evt.GetType(),"no iop");
        Debug::log("Editor::ProcessEvent()")(msg)(" ")(evt.GetType()).endl();
    }
    
    void Editor::reloadContent()
    {
        if(mContext==NULL)
        {
            Debug::log("Editor::reloadContent(): nothing to reload.");
            return;
        }
        bool shown=mDocument->IsVisible();
        Rocket::Core::Vector2i dims=mContext->GetDimensions();
        shutdown();
//         Rocket::Core::Factory::ClearStyleSheetCache();
        init(dims.x,dims.y);
        if(shown)
            show();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
