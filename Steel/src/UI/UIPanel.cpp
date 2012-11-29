
#include <Rocket/Core.h>

#include <Debug.h>
#include "UI/UIPanel.h"

namespace Steel
{
    UIPanel::UIPanel():
        Rocket::Core::EventListener(),
        mContext(NULL),mContextName(""),mDocumentFile(""),mDocument(NULL),
#ifdef DEBUG
        mAutoReload(true)
#else
        mAutoReload(false)
#endif
    {
    }

    UIPanel::UIPanel(Ogre::String contextName,File mDocumentFile):
        Rocket::Core::EventListener(),
        mContext(NULL),mContextName(contextName),mDocumentFile(mDocumentFile),mDocument(NULL),
#ifdef DEBUG
        mAutoReload(true)
#else
        mAutoReload(false)
#endif
    {
    }

    UIPanel::UIPanel(const UIPanel& other)
    {
        throw std::runtime_error("UIPanel::UIPanel=(const UIPanel& other) not implemented.");
        if(mContext!=NULL)
            ;

        if(mDocument!=NULL)
            mDocument->GetContext()->UnloadDocument(mDocument);
        if(mDocument!=other.mDocument)
        {
            mDocument=other.mDocument;
            mDocument->AddReference();
        }

        mContextName=other.mContextName;
        mAutoReload=other.mAutoReload;
    }

    UIPanel::~UIPanel()
    {
        shutdown();
    }

    UIPanel& UIPanel::operator=(const UIPanel& other)
    {
        throw std::runtime_error("UIPanel::operator=(const UIPanel& other) not implemented.");
        return *this;
    }

    void UIPanel::init(unsigned int width, unsigned int height)
    {
        mContext=Rocket::Core::CreateContext(mContextName.c_str(), Rocket::Core::Vector2i(width, height));
        mDocument = mContext->LoadDocument(mDocumentFile.fullPath().c_str());
        if(mAutoReload)
        {
            mDocumentFile.addFileListener(this);
        }
    }

    void UIPanel::onFileChangeEvent(File &file)
    {
        Debug::log("UIPanel::onFileChangeEvent(")(file.fullPath())(")").endl();
    }

    void UIPanel::shutdown()
    {
        if(mDocument!=NULL)
        {
            mDocument->GetContext()->UnloadDocument(mDocument);
            mDocument->RemoveReference();
            mDocument=NULL;
        }
        if(mContext!=NULL)
        {
            mContext->UnloadAllDocuments();
            mContext->UnloadAllMouseCursors();
            mContext->RemoveReference();
            mContext=NULL;
        }
    }

    void UIPanel::show()
    {
        if (mDocument== NULL)
        {
            Debug::warning("UIPanel.show(): can't show an undefined mDocument ! skipping.").endl();
            return;
        }
        onShow();
        mDocument->Show();
    }

    void UIPanel::hide()
    {
        if (mDocument== NULL)
        {
            Debug::warning("UIPanel.hide(): can't hide an undefined mDocument ! skipping.").endl();
            return;
        }
        onHide();
        mDocument->Hide();
    }

    void UIPanel::ProcessEvent(Rocket::Core::Event& event)
    {

    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
