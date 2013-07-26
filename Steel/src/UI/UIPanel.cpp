
#include <Rocket/Core.h>
#include <../Source/Core/StyleSheetFactory.h>
#include <../Source/Core/XMLNodeHandlerHead.h>
#include <../Source/Core/DocumentHeader.h>

#include <Debug.h>
#include "UI/UIPanel.h"

namespace Steel
{

    UIPanel::UIPanel():
        Rocket::Core::EventListener(),mWidth(0),mHeight(0),
        mContext(NULL),mContextName(""),mDocumentFile(""),mDocument(NULL),
        mAutoReload(false),mDependencies(std::set<File *>())
    {
    }

    UIPanel::UIPanel(Ogre::String contextName,File mDocumentFile):
        Rocket::Core::EventListener(),mWidth(0),mHeight(0),
        mContext(NULL),mContextName(contextName),mDocumentFile(mDocumentFile),mDocument(NULL),
        mAutoReload(false),mDependencies(std::set<File *>())
    {
    }

    UIPanel::UIPanel(const UIPanel& other)
    {
        throw std::runtime_error("UIPanel::UIPanel=(const UIPanel& other) not implemented.");

        if(NULL!=mDocument)
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
        mWidth=width;
        mHeight=height;
        mContext=Rocket::Core::CreateContext(mContextName.c_str(), Rocket::Core::Vector2i(width, height));
        mDocument = mContext->LoadDocument(mDocumentFile.fullPath().c_str());
        if(mAutoReload)
        {
            mDocumentFile.addFileListener(this);

            // listen on dependencies changes:
            // all <link> elements *in the body* with the attribute 'reloadonchange' set to true;

            Rocket::Core::ElementList links;
            mDocument->GetElementsByTagName(links,"reloadonchange");
            for(auto it=links.begin(); it!=links.end(); ++it)
            {
                Rocket::Core::Element *elem=*it;
                Rocket::Core::String value;
                elem->GetAttribute("path")->GetInto<Rocket::Core::String>(value);
                File file=mDocumentFile.parentDir().subfile(value.CString());
                if(!file.exists())
                    continue;
                mDependencies.insert(new File(file));
            }
            for(auto it=mDependencies.begin(); it!=mDependencies.end(); ++it)
            {
                File *f=*it;
                f->addFileListener(this);
            }
        }
    }

    void UIPanel::onFileChangeEvent(File file)
    {
        reloadContent();
    }

    void UIPanel::reloadContent()
    {
        if(NULL==mContext)
        {
            Debug::log("UIPanel::reloadContent(): no main document to reload. Skipping operation.");
            return;
        }
        // save state
        bool shown=mDocument->IsVisible();
        Rocket::Core::Vector2i dims=mContext->GetDimensions();
        shutdown();
        Rocket::Core::StyleSheetFactory::ClearStyleSheetCache();
        // load state
        init(dims.x,dims.y);
        if(shown)
            show();
    }

    void UIPanel::shutdown()
    {
        for(auto it=mDependencies.begin(); it!=mDependencies.end(); ++it)
        {
            (*it)->removeFileListener(this);
            delete (*it);
        }
        mDependencies.clear();
        if(NULL!=mDocument)
        {
            mDocument->GetContext()->UnloadDocument(mDocument);
            mDocument->RemoveReference();
            mDocument=NULL;
        }
        if(NULL!=mContext)
        {
            mContext->UnloadAllDocuments();
            mContext->UnloadAllMouseCursors();
            mContext->RemoveReference();
            mContext=NULL;
        }
    }

    void UIPanel::show()
    {
        if (NULL==mDocument)
            return;
        mDocument->Show();
        onShow();
    }

    void UIPanel::hide()
    {
        if (NULL==mDocument)
            return;
        onHide();
        mDocument->Hide();
    }

    bool UIPanel::isVisible()
    {
        return mDocument->IsVisible();
    }

    void UIPanel::ProcessEvent(Rocket::Core::Event& event)
    {

    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

