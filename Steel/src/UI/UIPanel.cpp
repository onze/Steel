
#include <Rocket/Core.h>

#include <Debug.h>
#include "UI/UIPanel.h"

namespace Steel
{
    UIPanel::UIPanel(Ogre::String contextName,Ogre::String documentPath):
        mContext(NULL),mContextName(contextName),mDocumentPath(documentPath),mDocument(NULL)
    {
    }

    UIPanel::UIPanel(const UIPanel& other)
    {
        if(mDocument!=NULL && mDocument!=other.mDocument)
        {
            mDocument->GetContext()->UnloadDocument(mDocument);
            mDocument=other.mDocument;
        }
        mContextName=other.mContextName;
        mDocumentPath=other.mDocumentPath;
    }

    UIPanel::~UIPanel()
    {
        if(mDocument!=NULL)
            mDocument->GetContext()->UnloadDocument(mDocument);
    }

    UIPanel& UIPanel::operator=(const UIPanel& other)
    {
        throw std::runtime_error("UIPanel::operator=(const UIPanel& other) not implemented.");
        return *this;
    }

    void UIPanel::init(unsigned int width, unsigned int height)
    {
        mContext=Rocket::Core::CreateContext(mContextName.c_str(), Rocket::Core::Vector2i(width, height));
        mDocument = mContext->LoadDocument(mDocumentPath.c_str());
    }

    void UIPanel::show()
    {
        if (mDocument== NULL)
        {
            Debug::warning("UIPanel.show(): can't show an undefined mDocument ! skipping.").endl();
            return;
        }
        mDocument->Show();

    }

    void UIPanel::hide()
    {
        if (mDocument== NULL)
        {
            Debug::warning("UIPanel.hide(): can't hide an undefined mDocument ! skipping.").endl();
            return;
        }
        mDocument->Hide();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
