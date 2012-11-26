
#ifndef UI_H
#define UI_H

#include <Rocket/Core.h>
#include <OgreSceneManager.h>
#include <OgreRenderQueueListener.h>
#include <OIS.h>

#include "UI/Editor.h"
#include "UI/HUD.h"
#include "UI/RenderInterfaceOgre3D.h"
#include <tools/File.h>

namespace Steel
{
    class InputManager;
    /**
     * Instanciate underlying UI and dispatches input controllers event to ui and inputManager.
     * Ogre's window is created by the engine, but its events are grabbed in here too.
     */
    class UI:public Rocket::Core::SystemInterface,Ogre::RenderQueueListener
    {

        public:
            UI();
            UI(const UI& other);
            virtual ~UI();
            virtual UI& operator=(const UI& other);

            void init(unsigned int width,
                      unsigned int height,
                      File UIDataDir, 
                      InputManager *inputMan,
                      Ogre::SceneManager *sceneManager,
                      Ogre::RenderWindow *window);

            /// Gets the number of seconds elapsed since the start of the application.
            virtual float GetElapsedTime();
            /// Logs the specified message.
            virtual bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message);

            void startEditMode();
            void stopEditMode();

            //those 4 methods are direcly copied from the libRocket ogre sample
            /// Called from Ogre before a queue group is rendered.
            virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);
            /// Called from Ogre after a queue group is rendered.
            virtual void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation);
            /// Configures Ogre's rendering system for rendering Rocket.
            void configureRenderSystem();
            /// Builds an OpenGL-style orthographic projection matrix.
            void buildProjectionMatrix(Ogre::Matrix4& matrix);
            ///maps OIS key codes to Rocket's
            void buildKeyMaps();
            int getKeyModifierState();
            
            //implements the OIS keyListener and mouseListener, as this allows the inputManager to pass them on as they arrive.
            bool keyPressed(const OIS::KeyEvent& evt);
            bool keyReleased(const OIS::KeyEvent& evt);
            bool mouseMoved(const OIS::MouseEvent& evt);
            bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            
            void shutdown();
        protected:
            /// not owned references
            InputManager *mInputMan;
            Ogre::RenderWindow *mWindow;

            /// owned references
            unsigned int mWidth,mHeight;
            Ogre::Timer mTimer;
            RenderInterfaceOgre3D *mRocketRenderInterface;
            Rocket::Core::Context* mMainContext;
            /// maps OIS key codes to rocket ones for input injection into Rocket UI
            typedef std::map< OIS::KeyCode, Rocket::Core::Input::KeyIdentifier > KeyIdentifierMap;
            KeyIdentifierMap mKeyIdentifiers;
            ///editor panel
            Editor mEditor;
            ///hud panel
            HUD mHUD;
            File mUIDataDir;
    };
}

#endif // UI_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
