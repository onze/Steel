#ifndef STEEL_UI_H
#define STEEL_UI_H

#include <vector>

#include <OgreSceneManager.h>
#include <OgreRenderQueueListener.h>
#include <OIS.h>

#include <Rocket/Core.h>

#include "UI/RenderInterfaceOgre3D.h"

#include "EngineEventListener.h"
#include "InputSystem/InputEventListener.h"
#include "Editor.h"
#include "HUD.h"

namespace Rocket
{
    class RenderInterfaceOgre3D;
}

namespace Steel
{
    class Engine;
    class InputManager;
    class File;

    /**
     * Instanciate underlying UI and dispatches input controllers event to ui and inputManager.
     * Ogre's window is created by the engine, but its events are grabbed in here too.
     */
    class UI: public Rocket::Core::SystemInterface, public Ogre::RenderQueueListener, public EngineEventListener, public InputEventListener
    {
    public:
        typedef std::map<Input::Code, Rocket::Core::Input::KeyIdentifier> KeyIdentifierMap;
        typedef std::map<Input::Code, int> MouseIdentifierMap;
        UI();
        UI(const UI &o);
        virtual ~UI();
        virtual UI &operator=(const UI &o);

        void init(File UIDataDir,
                  InputManager *inputMan,
                  Ogre::RenderWindow *window,
                  Engine *engine);
        
        /// Notifies of a window resize.
        void onResize(int width, int height);

        /// Gets the number of seconds elapsed since the start of the application.
        virtual float GetElapsedTime();
        /// Logs the specified message.
        virtual bool LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String &message);

        void startEditMode();
        void stopEditMode();

        bool processCommand(std::vector<Ogre::String> command);
        void reload();

        //those 4 methods are direcly copied from the libRocket ogre sample
        /// Called from Ogre before a queue group is rendered.
        virtual void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &skipThisInvocation);
//         /// Called from Ogre after a queue group is rendered.
//         virtual void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String &invocation, bool &repeatThisInvocation);
        /// Configures Ogre's rendering system for UI rendering (libRocket, Gwen).
        void configureRenderSystem();
        /// Builds an OpenGL-style orthographic projection matrix.
        void buildProjectionMatrix(Ogre::Matrix4 &matrix);

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
        bool mouseWheeled(int delta, Input::Event const &evt);

        /// called when a new level becomes the current level.
        void onLevelSet(Level *level);
        /// called right before a level is unset (becomes not current anymore).
        void onLevelUnset(Level *level);

        void shutdown();

        void loadConfig(ConfigFile const &config);
        void saveConfig(ConfigFile &config) const;

        ////getters
        Ogre::String resourceGroup() const {return "UI";}
        Editor &editor() {return mEditor;}
        File dataDir() const {return mUIDataDir;}
        KeyIdentifierMap &keyIdentifiers() {return mKeyIdentifiers;}

    protected:
        // not owned
        InputManager *mInputMan;
        Ogre::RenderWindow *mWindow;
        Engine *mEngine;

        // owned

        /// used to know how long has passed since Engin startup (libRocket has to know)
        Ogre::Timer mTimer;

        // rocket stuff
        Rocket::RenderInterfaceOgre3D *mRocketRenderInterface;
        Rocket::Core::Context *mMainContext;

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
