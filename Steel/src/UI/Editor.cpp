#include "UI/Editor.h"

namespace Steel
{
    Editor::Editor():UIPanel("Editor","ui/editor/default/editor.rml")
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
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
