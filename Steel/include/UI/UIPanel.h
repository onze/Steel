#ifndef STEEL_UIPANEL_H
#define STEEL_UIPANEL_H

#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/EventListener.h>
#include <OgrePrerequisites.h>

#include <tools/File.h>

namespace Steel
{
    class UIPanel: public Rocket::Core::EventListener, public FileEventListener
    {
        public:
            UIPanel();
            UIPanel(Ogre::String contextName, File mDocumentFile);
            UIPanel(const UIPanel& other);
            virtual ~UIPanel();
            virtual UIPanel& operator=(const UIPanel& other);

            virtual void init(unsigned int width, unsigned int height);
            virtual void shutdown();
            /// show the underlying document
            virtual void show();
            /// hides the underlying document
            virtual void hide();
            /// to be overloaded by subclasses that want to start listening to their document events
            virtual void onShow()
            {
            };
            /// to be overloaded by subclasses that want to stop listening to their document events
            virtual void onHide()
            {
            };
            /// true when the panel('s main document) is visible
            bool isVisible();

            inline Rocket::Core::Context *context()
            {
                return mContext;
            }
            /// Process the incoming events from mContext
            virtual void ProcessEvent(Rocket::Core::Event& event);

            /// Gets notified when the main document file is modified
            virtual void onFileChangeEvent(File file);
            /// reload Rocket files and update context content
            void reloadContent();

            // getters
            inline unsigned height()
            {
                return mHeight;
            }
            inline unsigned width()
            {
                return mWidth;
            }

        protected:

            ///not owned
            ///owned
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
