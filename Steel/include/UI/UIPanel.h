#ifndef UIPANEL_H
#define UIPANEL_H
#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/EventListener.h>
#include <OgrePrerequisites.h>
#include <tools/File.h>

namespace Steel
{
    class UIPanel:public Rocket::Core::EventListener, FileEventListener
    {
        public:
            UIPanel();
            UIPanel(Ogre::String contextName,File mDocumentFile);
            UIPanel(const UIPanel& other);
            virtual ~UIPanel();
            virtual UIPanel& operator=(const UIPanel& other);

            virtual void init(unsigned int mWidth, unsigned int mHeight);
            virtual void shutdown();
            /// show the underlying document
            virtual void show();
            /// hides the underlying document
            virtual void hide();
            /// to be overloaded by subclasses that want to start listening to their document events
            virtual void onShow(){};
            /// to be overloaded by subclasses that want to stop listening to their document events
            virtual void onHide(){};
            
            inline Rocket::Core::Context *context(){return mContext;}
            /// Process the incoming events from mContext.
            virtual void ProcessEvent(Rocket::Core::Event& event);
            
            /// Gets notified when the main document file is modified
            void onFileChangeEvent(File &file);
            
        protected:
            ///not owned
            ///owned
            Rocket::Core::Context *mContext;
            Ogre::String mContextName;
            File mDocumentFile;
            Rocket::Core::ElementDocument *mDocument;
            bool mAutoReload;
    };
}
#endif // UIPANEL_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
