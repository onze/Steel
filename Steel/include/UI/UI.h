#ifndef STEEL_UI_H
#define STEEL_UI_H

#include <vector>

#include <Rocket/Core.h>
#include <OgreSceneManager.h>
#include <OgreRenderQueueListener.h>
#include <OIS.h>

#include "UI/Editor.h"
#include "UI/HUD.h"
#include "tools/File.h"
#include "EngineEventListener.h"
#include <InputEventListener.h>

namespace Steel
{
    class Engine;
    class InputManager;
    class RenderInterfaceOgre3D;
    /**
     * Instanciate underlying UI and dispatches input controllers event to ui and inputManager.
     * Ogre's window is created by the engine, but its events are grabbed in here too.
     */
    class UI: public Rocket::Core::SystemInterface, Ogre::RenderQueueListener, EngineEventListener, InputEventListener
    {
        public:
            typedef std::map<Input::Code, Rocket::Core::Input::KeyIdentifier> KeyIdentifierMap;
            typedef std::map<Input::Code, int> MouseIdentifierMap;
            UI();
            UI(const UI& other);
            virtual ~UI();
            virtual UI& operator=(const UI& other);

            void init(unsigned int width, unsigned int height, File UIDataDir, InputManager *inputMan,
                      Ogre::RenderWindow *window, Engine *engine);

            /// Gets the number of seconds elapsed since the start of the application.
            virtual float GetElapsedTime();
            /// Logs the specified message.
            virtual bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message);

            void startEditMode();
            void stopEditMode();
            
            bool processCommand(std::vector<Ogre::String> command);
            void reload();

            //those 4 methods are direcly copied from the libRocket ogre sample
            /// Called from Ogre before a queue group is rendered.
            virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);
            /// Called from Ogre after a queue group is rendered.
            virtual void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation);
            /// Configures Ogre's rendering system for rendering Rocket.
            void configureRenderSystem();
            /// Builds an OpenGL-style orthographic projection matrix.
            void buildProjectionMatrix(Ogre::Matrix4& matrix);
            
            ///maps OIS mouse/key codes to Rocket's
            void buildCodeMaps();
            Rocket::Core::Input::KeyIdentifier getKeyIdentifier(Input::Code key) const;
            int getMouseIdentifier(Input::Code button) const;
            int getKeyModifierState();

            bool keyPressed(Input::Code key, Input::Event const &evt);
            bool keyReleased(Input::Code key, Input::Event const &evt);
            bool mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt);
            bool mousePressed(Input::Code button, Input::Event const &evt);
            bool mouseReleased(Input::Code button, Input::Event const &evt);

            /// called when a new level becomes the current level.
            void onLevelSet(Level *level);
            /// called right before a level is unset (becomes not current anymore).
            void onLevelUnset(Level *level);

            void shutdown();

            void loadConfig(ConfigFile const &config);
            void saveConfig(ConfigFile &config) const;

            ////getters
            Editor &editor()
            {
                return mEditor;
            }

            File dataDir()
            {
                return mUIDataDir;
            }

            KeyIdentifierMap &keyIdentifiers()
            {
                return mKeyIdentifiers;
            }
        protected:
            // not owned
            InputManager *mInputMan;
            Ogre::RenderWindow *mWindow;
            Engine *mEngine;

            // owned
            unsigned int mWidth;
            unsigned int mHeight;

            /// used to know how long has passed since Engin startup (libRocket has to know)
            Ogre::Timer mTimer;
            RenderInterfaceOgre3D *mRocketRenderInterface;
            Rocket::Core::Context* mMainContext;

            /// maps OIS key codes to rocket ones for input injection into Rocket UI
            KeyIdentifierMap mKeyIdentifiers;
            MouseIdentifierMap mMouseIdentifiers;
            ///editor panel
            Editor mEditor;
            ///hud panel
            HUD mHUD;
            ///root dir for UI data
            File mUIDataDir;
            /// state flag
            bool mEditMode;
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
