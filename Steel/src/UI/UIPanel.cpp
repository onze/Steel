
#include <string>
#include <OgreString.h>

#include <Rocket/Core.h>
#include <../Source/Core/StyleSheetFactory.h>
#include <../Source/Core/XMLNodeHandlerHead.h>
#include <../Source/Core/DocumentHeader.h>

#include <MyGUI_OgrePlatform.h>
#include <MyGUI.h>
#include <MyGUI_IResource.h>
#include <MyGUI_Widget.h>
#include <MyGUI_ComboBox.h>

#include "Debug.h"
#include "UI/UIPanel.h"
#include "UI/UI.h"
#include "tools/OgreUtils.h"

namespace Steel
{

    UIPanel::UIPanel(UI &ui): Rocket::Core::EventListener(),
        mUI(ui),
        mWidth(0), mHeight(0),
        mContext(nullptr), mContextName(StringUtils::BLANK), mDocumentFile(StringUtils::BLANK), mDocument(nullptr),
        mAutoReload(false), mDependencies(std::set<File *>())
    {
    }

    UIPanel::UIPanel(UI &ui, Ogre::String contextName, File mDocumentFile): Rocket::Core::EventListener(),
        mUI(ui),
        mWidth(0), mHeight(0),
        mContext(nullptr), mContextName(contextName), mDocumentFile(mDocumentFile), mDocument(nullptr),
        mAutoReload(false), mDependencies(std::set<File *>())
    {
        if(!mDocumentFile.exists())
            Debug::error("UIPanel::UIPanel(): panel resource ").quotes(mDocumentFile)(" not found.").endl().breakHere();
    }

    UIPanel::UIPanel(const UIPanel &o) : Rocket::Core::EventListener(o),
        mUI(o.mUI),
        mWidth(o.mWidth), mHeight(o.mHeight),
        mContext(o.mContext), mContextName(o.mContextName), mDocumentFile(o.mDocumentFile), mDocument(o.mDocument),
        mAutoReload(o.mAutoReload), mDependencies(o.mDependencies)
    {
        Debug::error("UIPanel::UIPanel=(const UIPanel& o) not implemented").endl().breakHere();

//         if(nullptr != mDocument)
//             mDocument->GetContext()->UnloadDocument(mDocument);
//
//         if(mDocument != o.mDocument)
//         {
//             mDocument = o.mDocument;
//             mDocument->AddReference();
//         }
//
//         mContextName = o.mContextName;
//         mAutoReload = o.mAutoReload;
    }

    UIPanel::~UIPanel()
    {
        shutdown();
    }

    UIPanel &UIPanel::operator=(const UIPanel &other)
    {
        Debug::error("UIPanel::operator=(const UIPanel& other) not implemented").endl().breakHere();
        return *this;
    }

    void UIPanel::init(unsigned int width, unsigned int height)
    {
        mWidth = width;
        mHeight = height;

        //libRocket
        {
            mContext = Rocket::Core::CreateContext(mContextName.c_str(), Rocket::Core::Vector2i(width, height));
            mDocument = mContext->LoadDocument(mDocumentFile.fullPath().c_str());
        }

        if(mAutoReload)
        {
            buildDependences();

            for(auto it = mDependencies.begin(); it != mDependencies.end(); ++it)
            {
                File *f = *it;
                f->addFileListener(this);
            }
        }

        // MyGUI
        {
            mMyGUIData.resource = nullptr;

            auto rgm = Ogre::ResourceGroupManager::getSingletonPtr();
            rgm->addResourceLocation((mDocumentFile.parentDir() / "images").absPath(), "FileSystem", mUI.resourceGroup());

            File skin = skinFile();

            if(skin.exists())
            {
                if(!MyGUI::ResourceManager::getInstance().load(skin.fullPath()))
                {
                    Debug::error("could not load skin", skin).endl().breakHere();
                    return;
                }
            }

            File resource = resourceFile();

            if(resource.exists())
            {
                if(!MyGUI::ResourceManager::getInstance().load(resource.fullPath()))
                {
                    Debug::error("could not load resource ", resource).endl();
                    return;
                }
            }

            MyGUI::ResourceLayout *RLayout = MyGUI::LayoutManager::getInstance().getByName(mContextName + "Layout", false);

            if(nullptr == RLayout)
            {
                Debug::error("ResourceLayout ", mContextName + "Layout ", "not found in resource ", resource).endl();
                return;
            }

            mMyGUIData.layout = RLayout->createLayout();

            if(0 == mMyGUIData.layout.size())
            {
                Debug::error("layout ", mContextName + "Layout ", "loaded from resource ", resource, " has no content.").endl();
                return;
            }

            std::vector<MyGUI::Widget *> widgets(mMyGUIData.layout.begin(), mMyGUIData.layout.end());
            setupMyGUIWidgetsLogic(widgets);
        }
    }

    void UIPanel::buildDependences()
    {
        // libRocket
        {
            //mDocumentFile.addFileListener(this);
            mDependencies.insert(new File(mDocumentFile));
            // listen on dependencies changes:
            // all <link> elements *in the body* with the attribute 'reloadonchange' set to true;

            Rocket::Core::ElementList links;
            mDocument->GetElementsByTagName(links, "reloadonchange");

            for(auto it = links.begin(); it != links.end(); ++it)
            {
                Rocket::Core::Element *elem = *it;
                Rocket::Core::String value;
                elem->GetAttribute("path")->GetInto<Rocket::Core::String>(value);
                File file = mDocumentFile.parentDir().subfile(value.CString());

                if(!file.exists())
                    continue;

                mDependencies.insert(new File(file));
            }
        }

        // MyGUI
        {
            mDependencies.insert(new File(layoutFile()));
        }
    }

    File UIPanel::layoutFile()
    {
        File layoutFile = mDocumentFile.absPath();
        layoutFile.extension() = "layout";
        return layoutFile;
    }

    File UIPanel::resourceFile()
    {
        File resourceFile = mDocumentFile.absPath();
        resourceFile.extension() = "myguiproject.resource.xml";
        return resourceFile;
    }

    File UIPanel::skinFile()
    {
        File skinFile = mDocumentFile.absPath();
        skinFile.extension() = "mygui_skin.xml";
        return skinFile;
    }

    void UIPanel::onFileChangeEvent(File file)
    {
        Debug::log("reloading content from file ")(file);
        reloadContent();
    }

    void UIPanel::reloadContent()
    {
        if(nullptr == mContext)
        {
            Debug::log("UIPanel::reloadContent(): no main document to reload. Skipping operation.");
            return;
        }

        // save state
        bool shown = mDocument->IsVisible();
        Rocket::Core::Vector2i dims = mContext->GetDimensions();

        if(shown)
            this->hide();

        this->shutdown();
        MyGUI::ResourceManager::getInstance().removeByName(layoutFile().fullPath());
        Rocket::Core::StyleSheetFactory::ClearStyleSheetCache();

        // load state
        this->init(dims.x, dims.y);

        if(shown)
            this->show();
    }

    void UIPanel::shutdown()
    {
        while(mDependencies.size())
        {
            auto begin = *mDependencies.begin();
            begin->removeFileListener(this);
            delete begin;
            mDependencies.erase(mDependencies.begin());
        }

        //libRocket
        {
            if(nullptr != mDocument)
            {
                mDocumentFile.removeFileListener(this);
                mDocument->GetContext()->UnloadDocument(mDocument);
                mDocument->RemoveReference();
                mDocument = nullptr;
            }

            if(nullptr != mContext)
            {
                mContext->UnloadAllDocuments();
                mContext->UnloadAllMouseCursors();
                mContext->RemoveReference();
                mContext = nullptr;
            }
        }

        // MyGUI
        {

            if(mMyGUIData.layout.size())
            {
                MyGUI::LayoutManager::getInstance().unloadLayout(mMyGUIData.layout);
                mMyGUIData.layout.clear();
            }

            if(nullptr != mMyGUIData.resource)
            {
                MyGUI::ResourceManager::getInstance().removeResource(mMyGUIData.resource);
                mMyGUIData.resource = nullptr;
            }
        }
    }

    void UIPanel::show()
    {
        // Rocket
        {
            if(nullptr != mDocument)
                mDocument->Show();
        }

        // MyGUI
        {
            for(MyGUI::Widget * widget : mMyGUIData.layout)
                widget->setVisible(true);
        }

        onShow();
    }

    void UIPanel::hide()
    {
        onHide();

        // Rocket
        {
            if(nullptr != mDocument)
                mDocument->Hide();
        }

        // MyGUI
        {
            for(MyGUI::Widget * widget : mMyGUIData.layout)
                widget->setVisible(false);
        }
    }

    bool UIPanel::isVisible()
    {
        return mDocument->IsVisible();
    }

    void UIPanel::ProcessEvent(Rocket::Core::Event &event)
    {
    }

    void UIPanel::setupMyGUIWidgetsLogic(std::vector<MyGUI::Widget *> &fringe)
    {
        // events
        static const std::string SteelOnChange = "SteelOnChange";
        static const std::string SteelOnClick = "SteelOnClick";

        // parse UI tree depth-first, subscribe to:
        // - all button clicks
        // - all combobox udpates
        // - all scrollbar updates
        while(fringe.size() > 0)
        {
            MyGUI::Widget *widget = fringe.back();
            fringe.pop_back();
            
            // register to relevant node events
            if(MyGUI::Button::getClassTypeName() == widget->getTypeName() && hasEvent(widget, SteelOnClick))
                widget->eventMouseButtonClick += MyGUI::newDelegate(this, &UIPanel::OnMyGUIMouseButtonClick);

            if(MyGUI::ComboBox::getClassTypeName() == widget->getTypeName() && hasEvent(widget, SteelOnChange))
                static_cast<MyGUI::ComboBox *>(widget)->eventComboAccept += MyGUI::newDelegate(this, &UIPanel::OnMyGUIComboAccept);

            if(MyGUI::ScrollBar::getClassTypeName() == widget->getTypeName() && hasEvent(widget, SteelOnChange))
                static_cast<MyGUI::ScrollBar *>(widget)->eventScrollChangePosition += MyGUI::newDelegate(this, &UIPanel::OnMyGUIScrollChangePosition);

            // add children
            MyGUI::EnumeratorWidgetPtr it = widget->getEnumerator();

            while(it.next())
                fringe.push_back(it.current());
        }
    }

    void UIPanel::OnMyGUIMouseButtonClick(MyGUI::Widget *button)
    {
        static const std::string SteelOnClick = "SteelOnClick";
        Ogre::String commands = button->getUserString(SteelOnClick);
        executeWidgetCommands(button, commands);
    }

    void UIPanel::OnMyGUIComboAccept(MyGUI::ComboBox *comboBox, size_t index)
    {
        static const std::string SteelOnChange = "SteelOnChange";
        Ogre::String commands = comboBox->getUserString(SteelOnChange);
        executeWidgetCommands(comboBox, commands);
    }

    void UIPanel::OnMyGUIScrollChangePosition(MyGUI::ScrollBar *scrollBar, size_t index)
    {
        static const std::string SteelOnChange = "SteelOnChange";
        Ogre::String commands = scrollBar->getUserString(SteelOnChange);
        executeWidgetCommands(scrollBar, commands);
    }

    void UIPanel::executeWidgetCommands(MyGUI::Widget *widget, Ogre::String const &commandsLine)
    {
        if(0 == commandsLine.size())
            return;

        static const Ogre::String commandSeparator = ";";
        //commands
        static const Ogre::String SteelSetVariable = "SteelSetVariable";
        static const Ogre::String SteelCommand = "SteelCommand";

        std::vector<Ogre::String> commands = StringUtils::split(commandsLine, commandSeparator);

        while(commands.size())
        {
            Ogre::String const command = commands.back();
            commands.pop_back();

            if(SteelSetVariable == command)
                executeSetVariableCommand(widget);
            else if(SteelCommand == command)
                executeEngineCommand(widget);
        }
    }

    void UIPanel::executeSetVariableCommand(MyGUI::Widget *widget)
    {
        static const std::string SteelVariableName = "SteelVariableName";

        if(!hasWidgetKey(widget, SteelVariableName))
            return;

        if(MyGUI::ComboBox::getClassTypeName() != widget->getTypeName())
        {
            Debug::error("UIPanel::bindSetVariableMyGUIDelegate() on ", widget, ": wrong widget type. Skipping.").endl();
            return;
        }

        Ogre::String variableName = widget->getUserString(SteelVariableName).c_str();
        MyGUI::ComboBox *cbBox = static_cast<MyGUI::ComboBox *>(widget);

        size_t index = cbBox->getIndexSelected();

        if(MyGUI::ITEM_NONE != index)
            setVariable(variableName, cbBox->getItemNameAt(index).asUTF8_c_str());
    }

    void UIPanel::executeEngineCommand(MyGUI::Widget *widget)
    {
        static const std::string SteelCommand = "SteelCommand";

        if(!hasWidgetKey(widget, SteelCommand))
            return;

        std::string command = widget->getUserString(SteelCommand);
        // all commands are postponed to next frame
        mUI.editor().processCommand("engine.register."+command);
    }

    bool UIPanel::hasEvent(MyGUI::Widget *widget, Ogre::String const &eventName)
    {
        return hasWidgetKey(widget, eventName);
    }

    bool UIPanel::hasWidgetKey(MyGUI::Widget *widget, Ogre::String const &eventName)
    {
        return !widget->getUserString(eventName).empty();
    }

    void UIPanel::setVariable(Ogre::String key, Ogre::String value)
    {
        mMyGUIData.UIVariables[key] = value;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

