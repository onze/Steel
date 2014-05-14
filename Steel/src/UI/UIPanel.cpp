
#include <MyGUI_OgrePlatform.h>
#include <MyGUI.h>
#include <MyGUI_Delegate.h>
#include <MyGUI_IResource.h>
#include <MyGUI_ComboBox.h>
#include <MyGUI_EditBox.h>
#include <MyGUI_ScrollBar.h>
#include <MyGUI_TabControl.h>
#include <MyGUI_Widget.h>

#include "Debug.h"
#include "Engine.h"
#include "UI/UIPanel.h"
#include "UI/UI.h"
#include "SignalManager.h"
#include "tools/OgreUtils.h"
#include "tools/3dParties/MyGUI/MyGUIFileTreeDataSource.h"
#include "tools/3dParties/MyGUI/TreeControl.h"

namespace Steel
{

    const std::string UIPanel::SteelOnClick = "SteelOnClick";
    const std::string UIPanel::SteelOnChange = "SteelOnChange";
    const std::string UIPanel::SteelOnSubmit = "SteelOnSubmit";
    const std::string UIPanel::SteelOnToggle = "SteelOnToggle";
    const std::string UIPanel::SteelBind = "SteelBind";
    const std::string UIPanel::SteelInsertWidget = "SteelInsertWidget";
    const std::string UIPanel::TreeControl = "TreeControl";
    const std::string UIPanel::SteelTreeControlDataSourceType = "SteelTreeControlDataSourceType";
    const std::string UIPanel::SteelTreeControlDataSourceType_FileTree = "FileTree";
    const std::string UIPanel::SteelTreeControlDataSourceRoot = "SteelTreeControlDataSourceRoot";
    const std::string UIPanel::SteelTreeControlDataSourceConfigFileBaseName = "SteelTreeControlDataSourceConfigFileBaseName";

    const Ogre::String UIPanel::commandSeparator = ";";
    const Ogre::String UIPanel::SteelSetVariable = "SteelSetVariable";
    const Ogre::String UIPanel::SteelCommand = "SteelCommand";

    UIPanel::UIPanel(UI &ui): FileEventListener(), SignalListener(), SignalEmitter(),
        mUI(ui),
        mWidth(0), mHeight(0), mIsVisible(false),
        mName(Ogre::StringUtil::BLANK), mResourceDir(),
        mAutoReload(false), mDependencies(), mSignalToWidgetsBindings()
    {
    }

    UIPanel::UIPanel(UI &ui, Ogre::String const &name, File resourceDir): FileEventListener(), SignalListener(), SignalEmitter(),
        mUI(ui),
        mWidth(0), mHeight(0), mIsVisible(false),
        mName(name), mResourceDir(resourceDir),
        mAutoReload(false), mDependencies(), mSignalToWidgetsBindings()
    {
        if(!mResourceDir.exists())
        {
            if(!mResourceDir.isPathAbsolute())
                mResourceDir = mUI.UIDataDir() / mResourceDir;
        }

        if(!mResourceDir.exists())
            Debug::error("UIPanel::UIPanel(): panel resource location ").quotes(mResourceDir)(" not found.").endl().breakHere();
    }

    UIPanel::UIPanel(const UIPanel &o) : FileEventListener(o), SignalListener(o), SignalEmitter(o),
        mUI(o.mUI),
        mWidth(o.mWidth), mHeight(o.mHeight), mIsVisible(o.mIsVisible),
        mName(o.mName), mResourceDir(o.mResourceDir),
        mAutoReload(o.mAutoReload), mDependencies(o.mDependencies), mSignalToWidgetsBindings(o.mSignalToWidgetsBindings)
    {
        Debug::error("UIPanel::UIPanel=(const UIPanel& o) not implemented").endl().breakHere();
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
            rgm->addResourceLocation((mResourceDir / "images").absPath(), "FileSystem", mUI.resourceGroup());

            File skin = skinFile();

            if(skin.exists())
            {
                if(!MyGUI::ResourceManager::getInstance().load(skin.fullPath()))
                {
                    Debug::error(STEEL_METH_INTRO, "could not load skin ", skin).endl().breakHere();
                    return;
                }
            }
            else
                Debug::warning(STEEL_METH_INTRO, "panel ", mName, " has no skin at ", skin).endl();

            File resource = resourceFile();

            if(resource.exists())
            {
                if(!MyGUI::ResourceManager::getInstance().load(resource.fullPath()))
                {
                    Debug::error(STEEL_METH_INTRO, "could not load resource ", resource).endl();
                    return;
                }
            }
            else
                Debug::warning(STEEL_METH_INTRO, "panel ", mName, " has no resource at ", resource).endl();

            MyGUI::ResourceLayout *RLayout = MyGUI::LayoutManager::getInstance().getByName(layoutName(), false);

            if(nullptr == RLayout)
            {
                Debug::error(STEEL_METH_INTRO, "ResourceLayout ", layoutName(), " not found in resource ", resource).endl();
                return;
            }

            mMyGUIData.layout = RLayout->createLayout();

            if(0 == mMyGUIData.layout.size())
            {
                Debug::error(STEEL_METH_INTRO, "layout ", layoutName(), " loaded from resource ", resource, " has no content.").endl();
                return;
            }

            mSignalToWidgetsBindings.clear();
            std::vector<MyGUI::Widget *> widgets(mMyGUIData.layout.begin(), mMyGUIData.layout.end());
            setupMyGUIWidgetsLogic(widgets);
        }
    }

    Ogre::String UIPanel::layoutName()
    {
        return mName + "Layout";
    }

    void UIPanel::buildDependences()
    {
        // MyGUI
        {
            mDependencies.insert(new File(resourceFile()));
        }
    }

    File UIPanel::resourceFile() // where the layout is
    {
        return (mResourceDir / mName).absPath() + ".myguiproject.resource.xml";
    }

    File UIPanel::skinFile()
    {
        return (mResourceDir / mName).absPath() + ".mygui_skin.xml";
    }

    void UIPanel::onFileChangeEvent(File file)
    {
        Debug::log(STEEL_METH_INTRO, "reloading content from file ", file).endl();
        reloadContent();
    }

    void UIPanel::reloadContent()
    {
        bool visible = isVisible();

        if(visible)
            this->hide();

        this->shutdown();
        MyGUI::ResourceManager::getInstance().removeByName(resourceFile().fullPath());

        // load state
        this->init(mWidth, mHeight);

        if(visible)
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

        // MyGUI
        {
            while(mMyGUIData.treeControlDataSources.size())
            {
                MyGUITreeControlDataSource *dataSource = mMyGUIData.treeControlDataSources.back();
                mMyGUIData.treeControlDataSources.pop_back();
                dataSource->shutdown();
                delete dataSource;
            }

            mSignalToWidgetsBindings.clear();

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
        // MyGUI
        {
            for(MyGUI::Widget * widget : mMyGUIData.layout)
                widget->setVisible(true);
        }
        mIsVisible = true;
        onShow();
    }

    void UIPanel::hide()
    {
        mIsVisible = false;
        onHide();

        // MyGUI
        {
            for(MyGUI::Widget * widget : mMyGUIData.layout)
                widget->setVisible(false);
        }
    }

    bool UIPanel::isVisible()
    {
        return mIsVisible;
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
        Debug::log(STEEL_METH_INTRO, "name: ", mName).endl();

        // parse UI tree depth-first, subscribe to:
        // - all button clicks
        // - all combobox udpates
        // - all scrollbar updates
        while(fringe.size() > 0)
        {
            MyGUI::Widget *widget = fringe.back();
            fringe.pop_back();

            // replace custom widgets
            insertMyGUICustomWidgets(widget);

            // register to relevant node events
            std::string const widgetTypeName = widget->getTypeName();
            bool hasOnClick = hasEvent(widget, UIPanel::SteelOnClick);
            bool hasOnChange = hasEvent(widget, UIPanel::SteelOnChange);
            bool hasOnSubmit = hasEvent(widget, UIPanel::SteelOnSubmit);
            bool hasOnToggle = hasEvent(widget, UIPanel::SteelOnToggle);

            // button
            if(MyGUI::Button::getClassTypeName() == widgetTypeName)
            {
                if(hasOnClick)
                    widget->eventMouseButtonClick += MyGUI::newDelegate(this, &UIPanel::OnMyGUIMouseButtonClick);
                else if(hasOnToggle)
                    widget->eventMouseButtonClick += MyGUI::newDelegate(this, &UIPanel::OnMyGUIMouseButtonClickForCheckboxToggle);
            }
            // combobox
            else if(MyGUI::ComboBox::getClassTypeName() == widgetTypeName)
            {
                if(hasOnChange)
                    static_cast<MyGUI::ComboBox *>(widget)->eventComboChangePosition += MyGUI::newDelegate(this, &UIPanel::OnMyGUIComboChangePosition);

                if(hasOnSubmit)
                    static_cast<MyGUI::ComboBox *>(widget)->eventComboAccept += MyGUI::newDelegate(this, &UIPanel::OnMyGUIComboAccept);
            }
            // scrollbar
            else if(MyGUI::ScrollBar::getClassTypeName() == widgetTypeName && hasOnChange)
                static_cast<MyGUI::ScrollBar *>(widget)->eventScrollChangePosition += MyGUI::newDelegate(this, &UIPanel::OnMyGUIScrollChangePosition);
            // editbox
            else if(MyGUI::EditBox::getClassTypeName() == widgetTypeName && hasOnSubmit)
                static_cast<MyGUI::EditBox *>(widget)->eventEditSelectAccept += MyGUI::newDelegate(this, &UIPanel::OnMyGUIEditSelectAccept);
            // tabcontrol
            else if(MyGUI::TabControl::getClassTypeName() == widgetTypeName && hasOnChange)
                static_cast<MyGUI::TabControl *>(widget)->eventTabChangeSelect += MyGUI::newDelegate(this, &UIPanel::OnMyGUITabControlChangeSelect);

            // register binding
            if(hasWidgetKey(widget, UIPanel::SteelBind))
            {
                std::string variableName = widget->getUserString(UIPanel::SteelBind);

                if(!variableName.empty())
                    bindMyGUIWidgetToVariable(widget, variableName);
            }

            // register children for processing
            MyGUI::EnumeratorWidgetPtr it = widget->getEnumerator();

            while(it.next())
                fringe.push_back(it.current());
        }

        Debug::log("UIPanel::setupMyGUIWidgetsLogic() done !").endl();
    }

    MyGUI::Widget *const UIPanel::findMyGUIChildWidget(Ogre::String const &name)
    {
        MyGUI::Widget *child = nullptr;

        for(auto const & parent : mMyGUIData.layout)
        {
            child = parent->findWidget(name);

            if(nullptr != child)
                break;
        }

        return child;
    }

    void UIPanel::insertMyGUICustomWidgets(MyGUI::Widget *&widget)
    {
        if(hasWidgetKey(widget, UIPanel::SteelInsertWidget))
        {
            Ogre::String widgetType = widget->getUserString(UIPanel::SteelInsertWidget);

            // clear widgets
            MyGUI::EnumeratorWidgetPtr widgetIterator(widget->getEnumerator());
            MyGUI::Gui::getInstance().destroyWidgets(widgetIterator);

            // generic properties
            Ogre::String controlName = widget->getName() + "_Child"; // its unique child at this point

            if(UIPanel::TreeControl == widgetType)
            {
                MyGUI::TreeControl *treeControl = static_cast<MyGUI::TreeControl *>(widget->createWidgetT(MyGUI::TreeControl::getClassTypeName(), "Tree", MyGUI::IntCoord(MyGUI::IntPoint(), widget->getClientCoord().size()), MyGUI::Align::Stretch, controlName));

                if(hasWidgetKey(widget, UIPanel::SteelTreeControlDataSourceType))
                {
                    Ogre::String dataSourceType = widget->getUserString(UIPanel::SteelTreeControlDataSourceType);

                    if(UIPanel::SteelTreeControlDataSourceType_FileTree == dataSourceType)
                    {
                        MyGUIFileSystemDataSource *dataSource = new MyGUIFileSystemDataSource();
                        {
                            Ogre::String dataSourceRoot = StringUtils::BLANK; // optional
                            Ogre::String dataSourceConfFileBaseName = StringUtils::BLANK; // optional

                            if(hasWidgetKey(widget, UIPanel::SteelTreeControlDataSourceRoot))
                                dataSourceRoot = widget->getUserString(UIPanel::SteelTreeControlDataSourceRoot);

                            if(hasWidgetKey(widget, UIPanel::SteelTreeControlDataSourceConfigFileBaseName))
                                dataSourceConfFileBaseName = widget->getUserString(UIPanel::SteelTreeControlDataSourceConfigFileBaseName);

                            File sourcePath = mUI.engine()->dataDir();

                            if(StringUtils::BLANK != dataSourceRoot)
                                sourcePath = sourcePath / dataSourceRoot;

                            dataSource->init(treeControl, sourcePath.fullPath(), dataSourceConfFileBaseName);
                        }
                        mMyGUIData.treeControlDataSources.push_back(dataSource);
                    }
                    else
                    {
                        Debug::warning(STEEL_METH_INTRO, "with widget ", widget, " invalid data source field ").quotes(UIPanel::SteelTreeControlDataSourceType).endl();
                    }
                }
                else
                {
                    Debug::warning(STEEL_METH_INTRO, "widget ", widget, " is missing a data source field ").quotes(UIPanel::SteelTreeControlDataSourceType).endl();
                }

                MyGUI::TreeControlNode *pRoot = treeControl->getRoot();
                pRoot->setText("root");
                MyGUI::TreeControlNode *pNode = new MyGUI::TreeControlNode("Item0", "Data");
                pRoot->add(pNode);
            }
            else if("Button" == widgetType)
            {
                MyGUI::Button *button = widget->createWidget<MyGUI::Button>(MyGUI::WidgetStyle::Child, "Button", MyGUI::IntCoord(MyGUI::IntPoint(), widget->getClientCoord().size()), MyGUI::Align::Stretch);
                button->setCaption("my button");
            }
            else
            {
                Debug::warning(STEEL_METH_INTRO, "widget ",  widget, " should be inserted an unknown widget type ").quotes(widgetType)(". Skipping.").endl();
            }
        }
    }

    void UIPanel::bindMyGUIWidgetToVariable(MyGUI::Widget *const widget, Ogre::String const &variableName)
    {
        setMyGUIWidgetValue(widget, getMyGUIVariable(variableName));
        Signal signal = getMyGUIVariableUpdateSignal(variableName);
        mSignalToWidgetsBindings.emplace(std::make_pair(signal, std::make_pair(variableName, SignalToWidgetsBindings::mapped_type::second_type()))).first->second.second.insert(widget);
    }

    void UIPanel::OnMyGUIMouseButtonClickForCheckboxToggle(MyGUI::Widget *button)
    {
        MyGUI::Button *checkbox = button->castType<MyGUI::Button>();
        checkbox->setStateSelected(!checkbox->getStateSelected());

        Ogre::String commands = button->getUserString(SteelOnToggle);
        executeWidgetCommands(button, commands);
    }

    void UIPanel::OnMyGUIMouseButtonClick(MyGUI::Widget *button)
    {
        Ogre::String commands = button->getUserString(SteelOnClick);
        executeWidgetCommands(button, commands);
    }

    void UIPanel::OnMyGUIComboAccept(MyGUI::ComboBox *comboBox, size_t index)
    {
        Ogre::String commands = comboBox->getUserString(UIPanel::SteelOnSubmit);
        executeWidgetCommands(comboBox, commands);
    }

    void UIPanel::OnMyGUIComboChangePosition(MyGUI::ComboBox *comboBox, size_t index)
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
        Ogre::String commands = editBox->getUserString(UIPanel::SteelOnSubmit);
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

        if(getMyGUIWidgetValue(widget, value))
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
                Debug::warning(STEEL_METH_INTRO, "unclosed dynamic value ").quotes(in.substr(i))(" in ").quotes(in)(". Skipping.").endl();
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

        // emitting on same value may provoke infinite variable->widget->variable loops
        if(newValue)
        {
            emit(getMyGUIVariableUpdateSignal(key));
        }
    }

    Signal UIPanel::getMyGUIVariableUpdateSignal(Ogre::String const &variableName)
    {
        if(StringUtils::BLANK == variableName)
            return INVALID_SIGNAL;

        return SignalManager::instance().toSignal("__UIPanel__" + mName + "__MyGUIVariable__" + variableName + "__UpdateSignal");
    }

    bool UIPanel::hasEvent(MyGUI::Widget *widget, Ogre::String const &eventName)
    {
        return hasWidgetKey(widget, eventName);
    }

    bool UIPanel::hasWidgetKey(MyGUI::Widget *widget, Ogre::String const &eventName)
    {
        return !widget->getUserString(eventName).empty();
    }

    void UIPanel::onSignal(Signal signal, SignalEmitter *const /*src*/)
    {
        DispatchSignalToBoundWidgets(signal);
    }

    void UIPanel::DispatchSignalToBoundWidgets(Signal signal)
    {
        SignalToWidgetsBindings::iterator it = mSignalToWidgetsBindings.find(signal);

        if(mSignalToWidgetsBindings.end() != it)
        {
            Ogre::String const value = it->second.first;
            SignalToWidgetsBindings::mapped_type::second_type widgets = it->second.second;

            for(MyGUI::Widget * const widget : widgets)
                setMyGUIWidgetValue(widget, value);
        }
    }

    bool UIPanel::getMyGUIWidgetValue(MyGUI::Widget *const widget, Ogre::String &value)
    {
        if(MyGUI::ComboBox::getClassTypeName() == widget->getTypeName())
        {
            MyGUI::ComboBox *downcastedWidget = static_cast<MyGUI::ComboBox *>(widget);
            size_t index = downcastedWidget->getIndexSelected();

            if(MyGUI::ITEM_NONE != index)
                value = downcastedWidget->getItemNameAt(index).asUTF8_c_str();
            else
                value = downcastedWidget->getCaption().asUTF8_c_str();
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
            Debug::error(STEEL_METH_INTRO, "on ", widget, ": unhandled widget type. Skipping.").endl();
            return false;
        }


        return true;
    }

    void UIPanel::setMyGUIWidgetValue(MyGUI::Widget *const widget, Ogre::String const &value)
    {
        if(MyGUI::ComboBox::getClassTypeName() == widget->getTypeName())
        {
            MyGUI::ComboBox *downcastedWidget = static_cast<MyGUI::ComboBox *>(widget);
            size_t index = downcastedWidget->findItemIndexWith(value);

            if(MyGUI::ITEM_NONE != index)
                downcastedWidget->setIndexSelected(index);
        }
        else if(MyGUI::ScrollBar::getClassTypeName() == widget->getTypeName())
        {
            MyGUI::ScrollBar *downcastedWidget = static_cast<MyGUI::ScrollBar *>(widget);
            size_t position = (size_t)Ogre::StringConverter::parseInt(value, 0);
            downcastedWidget->setScrollPosition(position);
        }
        else if(MyGUI::EditBox::getClassTypeName() == widget->getTypeName())
        {
            MyGUI::EditBox *downcastedWidget = static_cast<MyGUI::EditBox *>(widget);
            downcastedWidget->setCaption(value);
        }
        else if(MyGUI::TabControl::getClassTypeName() == widget->getTypeName())
        {
            MyGUI::TabControl *downcastedWidget = static_cast<MyGUI::TabControl *>(widget);
            size_t index = downcastedWidget->findItemIndexWith(value);

            if(MyGUI::ITEM_NONE != index)
                downcastedWidget->setIndexSelected(index);
        }
        else
        {
            Debug::error(STEEL_METH_INTRO, "on ", widget, ": unhandled widget type. Skipping.").endl();
            return;
        }
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

