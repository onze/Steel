#ifndef STEEL_UIPANEL_H
#define STEEL_UIPANEL_H

#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/EventListener.h>
#include <OgrePrerequisites.h>

#include "steeltypes.h"
#include "tools/File.h"
#include "tools/FileEventListener.h"

namespace MyGUI
{
    class Widget;
    typedef std::vector< Widget *> VectorWidgetPtr;
    
    class ComboBox;
    class ScrollBar;
    
    class IResource;
}

namespace Steel
{
    class UI;
    
    class UIPanel: public Rocket::Core::EventListener, public FileEventListener
    {
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

        // getters
        inline unsigned height() const {return mHeight;}
        inline unsigned width() const {return mWidth;}

    protected:
        void buildDependences();
        
        /// Parses through the UI tree and sets up widget logic, given their userData
        void setupMyGUIWidgetsLogic(std::vector<MyGUI::Widget *> &widgets);
        
        // MyGUI callbacks
        void OnMyGUIMouseButtonClick(MyGUI::Widget *button);
        void OnMyGUIComboAccept(MyGUI::ComboBox *comboBox, size_t index);
        void OnMyGUIScrollChangePosition(MyGUI::ScrollBar *scrollBar, size_t index);
        
        void executeWidgetCommands(MyGUI::Widget *widget, Ogre::String const& commandsLine);
        void executeSetVariableCommand(MyGUI::Widget *widget);
        void executeEngineCommand(MyGUI::Widget *widget);
        
        // widget inspection helpers
        bool hasEvent(MyGUI::Widget *widget, Ogre::String const& eventName);
        bool hasWidgetKey(MyGUI::Widget *widget, Ogre::String const& eventName);
        
        /// write access to UI shared variables
        void setVariable(Ogre::String key, Ogre::String value);
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
        struct MyGUIData
        {
            MyGUI::VectorWidgetPtr layout;
            MyGUI::IResource *resource;
            typedef std::map<Ogre::String, Ogre::String> StringStringMap;
            StringStringMap UIVariables;
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
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
