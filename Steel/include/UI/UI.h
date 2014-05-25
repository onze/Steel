#ifndef STEEL_UI_H
#define STEEL_UI_H

#include <vector>

#include <OgreSceneManager.h>
#include <OgreRenderQueueListener.h>
#include <OIS.h>

#include "EngineEventListener.h"
#include "InputSystem/InputEventListener.h"
#include "Editor.h"
#include "HUD.h"

namespace MyGUI
{
    class Gui;
    class OgrePlatform;
    class KeyCode;
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
    class UI: public InputEventListener
    {
    public:
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

        void startEditMode();
        void stopEditMode();

        bool processCommand(std::vector<Ogre::String> command);
        void reload();

        ///maps OIS mouse/key codes to Steel's
        void buildCodeMaps();
        int getMouseIdentifier(Input::Code button) const;

        bool keyPressed(Input::Code key, Input::Event const &evt);
        bool keyReleased(Input::Code key, Input::Event const &evt);
        bool mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt);
        bool mousePressed(Input::Code button, Input::Event const &evt);
        bool mouseReleased(Input::Code button, Input::Event const &evt);
        bool mouseWheeled(int delta, Input::Event const &evt);

        void shutdown();

        void loadConfig(ConfigFile const &config);
        void saveConfig(ConfigFile &config) const;

        ////getters
        Ogre::String resourceGroup() const {return "UI";}
        Editor &editor() {return mEditor;}
        Engine *const engine() const {return mEngine;}
        File UIDataDir() const {return mUIDataDir;}

    private:
        MyGUI::KeyCode getMyGUIKeyIdentifier(Input::Code key) const;

        // not owned
        InputManager *mInputMan = nullptr;
        Ogre::RenderWindow *mWindow = nullptr;
        Engine *mEngine = nullptr;

        // owned
        
        // mygui stuff
        MouseIdentifierMap mMouseIdentifiers;
        typedef std::map<Steel::Input::Code, MyGUI::KeyCode> MyGUIKeyMap;
        struct MyGUIData
        {
            MyGUI::Gui *gui = nullptr;
            MyGUI::OgrePlatform *platform = nullptr;
            MyGUIKeyMap keyMap;
        };
        MyGUIData mMyGUIData;

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
