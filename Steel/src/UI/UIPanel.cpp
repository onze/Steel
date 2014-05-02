
#include <string>
#include <OgreString.h>

#include <Rocket/Core.h>
#include <../Source/Core/StyleSheetFactory.h>
#include <../Source/Core/XMLNodeHandlerHead.h>
#include <../Source/Core/DocumentHeader.h>

#include <MyGUI_OgrePlatform.h>
#include <MyGUI.h>
#include <MyGUI_IResource.h>
#include <MyGUI_ComboBox.h>
#include <MyGUI_EditBox.h>
#include <MyGUI_ScrollBar.h>
#include <MyGUI_TabControl.h>
#include <MyGUI_Widget.h>

#include "Debug.h"
#include "UI/UIPanel.h"
#include "UI/UI.h"
#include "tools/OgreUtils.h"
#include <SignalManager.h>

namespace Steel
{

    const std::string UIPanel::SteelOnClick = "SteelOnClick";
    const std::string UIPanel::SteelOnChange = "SteelOnChange";

    const Ogre::String UIPanel::commandSeparator = ";";
    const Ogre::String UIPanel::SteelSetVariable = "SteelSetVariable";
    const Ogre::String UIPanel::SteelCommand = "SteelCommand";

    UIPanel::UIPanel(UI &ui): Rocket::Core::EventListener(), SignalListener(), SignalEmitter(),
        mUI(ui),
        mWidth(0), mHeight(0),
        mContext(nullptr), mContextName(StringUtils::BLANK), mDocumentFile(StringUtils::BLANK), mDocument(nullptr),
        mAutoReload(false), mDependencies(std::set<File *>())
    {
    }

    UIPanel::UIPanel(UI &ui, Ogre::String contextName, File mDocumentFile): Rocket::Core::EventListener(), SignalListener(), SignalEmitter(),
        mUI(ui),
        mWidth(0), mHeight(0),
        mContext(nullptr), mContextName(contextName), mDocumentFile(mDocumentFile), mDocument(nullptr),
        mAutoReload(false), mDependencies(std::set<File *>())
    {
        if(!mDocumentFile.exists())
            Debug::error("UIPanel::UIPanel(): panel resource ").quotes(mDocumentFile)(" not found.").endl().breakHere();
    }

    UIPanel::UIPanel(const UIPanel &o) : Rocket::Core::EventListener(o), SignalListener(), SignalEmitter(),
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

    bool UIPanel::MyGUIHitTest(int const x, int const y) const
    {
        for(MyGUI::Widget const * const widget : mMyGUIData.layout)
        {
            MyGUI::IntCoord const coords = widget->getAbsoluteCoord();

            if(x >= coords.left && x <= coords.right() && y > coords.top && y < coords.bottom())
                return true;
        }

        return false;
    }

    void UIPanel::setupMyGUIWidgetsLogic(std::vector<MyGUI::Widget *> &fringe)
    {
        // parse UI tree depth-first, subscribe to:
        // - all button clicks
        // - all combobox udpates
        // - all scrollbar updates
        while(fringe.size() > 0)
        {
            MyGUI::Widget *widget = fringe.back();
            fringe.pop_back();

            // register to relevant node events
            if(MyGUI::Button::getClassTypeName() == widget->getTypeName() && hasEvent(widget, UIPanel::SteelOnClick))
                widget->eventMouseButtonClick += MyGUI::newDelegate(this, &UIPanel::OnMyGUIMouseButtonClick);
            else if(MyGUI::ComboBox::getClassTypeName() == widget->getTypeName() && hasEvent(widget, UIPanel::SteelOnChange))
                static_cast<MyGUI::ComboBox *>(widget)->eventComboAccept += MyGUI::newDelegate(this, &UIPanel::OnMyGUIComboAccept);
            else if(MyGUI::ScrollBar::getClassTypeName() == widget->getTypeName() && hasEvent(widget, UIPanel::SteelOnChange))
                static_cast<MyGUI::ScrollBar *>(widget)->eventScrollChangePosition += MyGUI::newDelegate(this, &UIPanel::OnMyGUIScrollChangePosition);
            else if(MyGUI::EditBox::getClassTypeName() == widget->getTypeName() && hasEvent(widget, UIPanel::SteelOnChange))
                static_cast<MyGUI::EditBox *>(widget)->eventEditSelectAccept += MyGUI::newDelegate(this, &UIPanel::OnMyGUIEditSelectAccept);
            else if(MyGUI::TabControl::getClassTypeName() == widget->getTypeName() && hasEvent(widget, UIPanel::SteelOnChange))
                static_cast<MyGUI::TabControl *>(widget)->eventTabChangeSelect += MyGUI::newDelegate(this, &UIPanel::OnMyGUITabControlChangeSelect);

            // add children
            MyGUI::EnumeratorWidgetPtr it = widget->getEnumerator();

            while(it.next())
                fringe.push_back(it.current());
        }
    }

    void UIPanel::OnMyGUIMouseButtonClick(MyGUI::Widget *button)
    {
        Ogre::String commands = button->getUserString(SteelOnClick);
        executeWidgetCommands(button, commands);
    }

    void UIPanel::OnMyGUIComboAccept(MyGUI::ComboBox *comboBox, size_t index)
    {
        Ogre::String commands = comboBox->getUserString(UIPanel::SteelOnChange);
        executeWidgetCommands(comboBox, commands);
    }

    void UIPanel::OnMyGUIScrollChangePosition(MyGUI::ScrollBar *scrollBar, size_t index)
    {
        Ogre::String commands = scrollBar->getUserString(UIPanel::SteelOnChange);
        executeWidgetCommands(scrollBar, commands);
    }

    void UIPanel::OnMyGUIEditSelectAccept(MyGUI::EditBox *editBox)
    {
        Ogre::String commands = editBox->getUserString(UIPanel::SteelOnChange);
        executeWidgetCommands(editBox, commands);
    }

    void UIPanel::OnMyGUITabControlChangeSelect(MyGUI::TabControl *tabControl, size_t index)
    {
        Ogre::String commands = tabControl->getUserString(UIPanel::SteelOnChange);
        executeWidgetCommands(tabControl, commands);
    }

    void UIPanel::executeWidgetCommands(MyGUI::Widget *widget, Ogre::String const &commandsLine)
    {
        if(0 == commandsLine.size())
            return;

        std::vector<Ogre::String> const commands = StringUtils::split(commandsLine, UIPanel::commandSeparator);

        for(auto const & command : commands)
        {
            if(UIPanel::SteelSetVariable == command)
                executeSetVariableCommand(widget);
            else if(UIPanel::SteelCommand == command)
                executeEngineCommand(widget);
        }
    }

    void UIPanel::executeSetVariableCommand(MyGUI::Widget *widget)
    {
        static const std::string SteelVariableName = "SteelVariableName";

        if(!hasWidgetKey(widget, SteelVariableName))
            return;

        Ogre::String variableName = widget->getUserString(SteelVariableName).c_str();

        std::string value;

        if(MyGUI::ComboBox::getClassTypeName() == widget->getTypeName())
        {
            MyGUI::ComboBox *downcastedWidget = static_cast<MyGUI::ComboBox *>(widget);
            size_t index = downcastedWidget->getIndexSelected();

            if(MyGUI::ITEM_NONE != index)
                value = downcastedWidget->getItemNameAt(index).asUTF8_c_str();
        }
        else if(MyGUI::ScrollBar::getClassTypeName() == widget->getTypeName())
        {
            MyGUI::ScrollBar *downcastedWidget = static_cast<MyGUI::ScrollBar *>(widget);
            value = Ogre::StringConverter::toString(downcastedWidget->getScrollPosition() * 100 / downcastedWidget->getScrollRange());
        }
        else if(MyGUI::EditBox::getClassTypeName() == widget->getTypeName())
        {
            MyGUI::EditBox *downcastedWidget = static_cast<MyGUI::EditBox *>(widget);
            value = downcastedWidget->getCaption();
        }
        else if(MyGUI::TabControl::getClassTypeName() == widget->getTypeName())
        {
            MyGUI::TabControl *downcastedWidget = static_cast<MyGUI::TabControl *>(widget);
            size_t index = downcastedWidget->getIndexSelected();

            if(MyGUI::ITEM_NONE != index)
                value = downcastedWidget->getItemNameAt(index).asUTF8_c_str();
        }
        else
        {
            Debug::error("UIPanel::bindSetVariableMyGUIDelegate() on ", widget, ": unhandled widget type. Skipping.").endl();
            return;
        }

        setMyGUIVariable(variableName, value);
    }

    void UIPanel::executeEngineCommand(MyGUI::Widget *widget)
    {
        static const std::string SteelCommand = "SteelCommand";

        if(!hasWidgetKey(widget, SteelCommand))
            return;

        std::string raw_command = widget->getUserString(SteelCommand);
        std::string command = replaceDynamicValues(raw_command);
        // all commands are postponed to next frame
        mUI.editor().processCommand("engine.register." + command);
    }

    std::string UIPanel::replaceDynamicValues(std::string const &in) const
    {
        std::string const delimBegin = "$(";
        std::string const delimEnd = ")";
        std::string out = "";
        std::string::size_type j = 0 , i = 0;

        while(j < in.size() && std::string::npos != (i = in.find(delimBegin, j)))
        {
            out += in.substr(j, i);

            j = in.find(delimEnd, i + 2);

            if(std::string::npos == j)
            {
                Debug::warning("UIPanel::replaceDynamicValues(): unclosed dynamic value ").quotes(in.substr(i))(" in ").quotes(in)(". Skipping.").endl();
                return in;
            }

            std::string key = in.substr(i + 2, (j++) - (i + 2)), value = "";
            auto it = mMyGUIData.UIVariables.find(key);

            if(mMyGUIData.UIVariables.cend() != it)
                value = it->second;

            out += value;
        }

        out += in.substr(j);

        return out;
    }

    Ogre::String UIPanel::getMyGUIVariable(Ogre::String key) const
    {
        auto it = mMyGUIData.UIVariables.find(key);
        return mMyGUIData.UIVariables.cend() == it ? StringUtils::BLANK : it->second;
    }

    void UIPanel::setMyGUIVariable(Ogre::String key, Ogre::String value)
    {
        Debug::log("UIPanel::setMyGUIVariable(", key, " = ", value, ")").endl();
        bool newValue = false;
        auto it = mMyGUIData.UIVariables.find(key);

        if(mMyGUIData.UIVariables.end() == it)
            newValue = true;
        else
            newValue = mMyGUIData.UIVariables[key] != value;

        mMyGUIData.UIVariables[key] = value;

        if(newValue)
            emit(getMyGUIVariableUpdateSignal(key));
    }

    Signal UIPanel::getMyGUIVariableUpdateSignal(Ogre::String const &variableName)
    {
        if(StringUtils::BLANK == variableName)
            return INVALID_SIGNAL;

        return SignalManager::instance().toSignal("__UIPanel__" + mContextName + "__MyGUIVariable__" + variableName + "__UpdateSignal");
    }

    bool UIPanel::hasEvent(MyGUI::Widget *widget, Ogre::String const &eventName)
    {
        return hasWidgetKey(widget, eventName);
    }

    bool UIPanel::hasWidgetKey(MyGUI::Widget *widget, Ogre::String const &eventName)
    {
        return !widget->getUserString(eventName).empty();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

