#ifndef UIPANEL_H
#define UIPANEL_H
#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>
#include <OgrePrerequisites.h>

namespace Steel
{
    class UIPanel
    {
        private:
        public:
            UIPanel() {};
            UIPanel(Ogre::String contextName,Ogre::String documentPath);
            UIPanel(const UIPanel& other);
            virtual ~UIPanel();
            virtual UIPanel& operator=(const UIPanel& other);

            virtual void init(unsigned int mWidth, unsigned int mHeight);
            virtual void show();
            virtual void hide();
            
            inline Rocket::Core::Context *context(){return mContext;}
            
        protected:
            ///not owned
            ///owned
            Rocket::Core::Context *mContext;
            Ogre::String mContextName;
            Ogre::String mDocumentPath;
            Rocket::Core::ElementDocument *mDocument;
    };
}
#endif // UIPANEL_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
