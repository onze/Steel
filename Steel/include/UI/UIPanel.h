#ifndef STEEL_UIPANEL_H
#define STEEL_UIPANEL_H

#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/EventListener.h>
#include <OgrePrerequisites.h>

#include "steeltypes.h"

#include "tools/File.h"
#include "tools/FileEventListener.h"
#include "SignalEmitter.h"
#include "SignalListener.h"

namespace MyGUI
{
    class Widget;
    typedef std::vector< Widget *> VectorWidgetPtr;
    
    class ComboBox;
    class EditBox;
    class ScrollBar;
    class TabControl;
    class TreeControl;
    class TreeControlNode;
    
    class IResource;
    class ResourceSkin;
}

namespace Steel
{
    class UI;
    class MyGUITreeControlDataSource;
    
    class UIPanel: public Rocket::Core::EventListener, public FileEventListener, public SignalListener, public SignalEmitter
    {
    protected:
        static const std::string SteelOnClick;
        static const std::string SteelOnChange;
        static const std::string SteelOnSubmit;
        static const std::string SteelBind;
        static const std::string SteelInsertWidget;
        static const std::string TreeControl;
        static const std::string SteelTreeControlDataSourceType;
        static const std::string SteelTreeControlDataSourceType_FileTree;
        static const std::string SteelTreeControlDataSourceRoot;
        //commands
        static const Ogre::String commandSeparator;
        static const Ogre::String SteelSetVariable;
        static const Ogre::String SteelCommand;
        
    public:
        UIPanel(UI &ui);
        UIPanel(UI &ui, Ogre::String contextName, File mDocumentFile);
        UIPanel(const UIPanel &o);
        virtual ~UIPanel();
        virtual UIPanel &operator=(const UIPanel &o);

        virtual void init(unsigned int width, unsigned int height);
        virtual void shutdown();

        /// show the underlying document
        virtual void show();
        /// hides the underlying document
        virtual void hide();

        /// to be overloaded by subclasses that want to start listening to their document events
        virtual void onShow() {};
        /// to be overloaded by subclasses that want to stop listening to their document events
        virtual void onHide() {};

        /// true when the panel('s main document) is visible
        bool isVisible();

        inline Rocket::Core::Context *context() {return mContext;}
        
        /// Process the incoming events from mContext
        virtual void ProcessEvent(Rocket::Core::Event &event);

        /// Gets notified when the main document file is modified
        virtual void onFileChangeEvent(File file);
        /// reload Rocket files and update context content
        void reloadContent();
        
        /// Returns true iff the given coodinates hit at least one of the MyGUI widgets.
        bool MyGUIHitTest(int const x, int const y) const;
        
        /// Replaces a $(MyGUI_variables) with its value in the given string
        std::string replaceDynamicValues(std::string const &in) const;

        // getters
        inline unsigned height() const {return mHeight;}
        inline unsigned width() const {return mWidth;}
        
        /// Dispatches signals. //!\\ Subclasses can override it but still want to call this version.
        virtual void onSignal(Signal signal, SignalEmitter *const src = nullptr);

        // MyGUI callbacks
        void OnMyGUIMouseButtonClick(MyGUI::Widget *button);
        void OnMyGUIComboAccept(MyGUI::ComboBox *comboBox, size_t index);
        void OnMyGUIComboChangePosition(MyGUI::ComboBox *comboBox, size_t index);
        void OnMyGUIScrollChangePosition(MyGUI::ScrollBar *scrollBar, size_t index);
        void OnMyGUIEditSelectAccept(MyGUI::EditBox *editBox);
        void OnMyGUITabControlChangeSelect(MyGUI::TabControl *tabControl, size_t index);
        
        /// Parses through the UI tree and sets up widget logic, given their userData
        void setupMyGUIWidgetsLogic(std::vector<MyGUI::Widget *> &widgets);
        void bindMyGUIWidgetToVariable(MyGUI::Widget *const widget, const Ogre::String &variableName);
        void insertMyGUICustomWidgets(MyGUI::Widget *&widget);
        
        void executeWidgetCommands(MyGUI::Widget *widget, Ogre::String const& commandsLine);
        void executeSetVariableCommand(MyGUI::Widget *widget);
        void executeEngineCommand(MyGUI::Widget *widget);
    protected:
        void buildDependences();
        
        
        void DispatchSignalToBoundWidgets(Signal signal);
        
        MyGUI::Widget*const findMyGUIChildWidget(Ogre::String const& name);
        
        // widget inspection helpers
        bool hasEvent(MyGUI::Widget *widget, Ogre::String const& eventName);
        bool hasWidgetKey(MyGUI::Widget *widget, Ogre::String const& eventName);
        
        /// read access to UI shared variables
        Ogre::String getMyGUIVariable(Ogre::String key) const;
        /// write access to UI shared variables
        void setMyGUIVariable(Ogre::String key, Ogre::String value);
        
        /// Returns the signal emitted by the panel when the given variable is updated
        Signal getMyGUIVariableUpdateSignal(Ogre::String const& variableName);
        
        bool getMyGUIWidgetValue(MyGUI::Widget *const widget, Ogre::String& value);
        void setMyGUIWidgetValue(MyGUI::Widget *const widget, Ogre::String const& value);
        
        //not owned
        UI &mUI;
        
        //owned
        /// screen size, in pixels.
        unsigned mWidth, mHeight;
        /// what displays documents
        Rocket::Core::Context *mContext;
        /// rocket code name
        Ogre::String mContextName;
        /// source
        File mDocumentFile;
        /// root
        Rocket::Core::ElementDocument *mDocument;
        
        // MyGUI stuff
        File layoutFile();
        File resourceFile();
        File skinFile();
        typedef std::vector<MyGUITreeControlDataSource *> MyGUITreeControlDataSourceVector;
        struct MyGUIData
        {
            MyGUI::VectorWidgetPtr layout;
            MyGUI::ResourceSkin *skin;
            MyGUI::IResource *resource;
            typedef std::map<Ogre::String, Ogre::String> StringStringMap;
            StringStringMap UIVariables;
            MyGUITreeControlDataSourceVector treeControlDataSources;
        };
        MyGUIData mMyGUIData;
        
        /**
         * Set to true, watches mDocumentFile and reloads it when it chanes.
         * Document state persisence is left to subclasses, except for size and visibility
         */
        bool mAutoReload;
        
        /** Files loaded by the main document, for which we might want to be notified of the changes too.
         * Those files are included as <link href="<path>" reloadonchange="true">
         * Main use case is styleSheets. Note that mAutoreload has to be true at panel's init.
         */
        std::set<File *> mDependencies;
        
        /// {variable update signal: widget to update}
        typedef std::map<Signal, std::pair<Ogre::String, std::set<MyGUI::Widget *>>> SignalToWidgetsBindings;
        SignalToWidgetsBindings mSignalToWidgetsBindings; 
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
