
#include <Ogre.h>

#include <MyGUI_OgrePlatform.h>
#include <MyGUI.h>
#include "tools/3dParties/MyGUI/TreeControl.h"
#include "tools/3dParties/MyGUI/TreeControlItem.h"

#include "UI/UI.h"
#include "Debug.h"
#include "InputSystem/InputManager.h"
#include "tools/OgreUtils.h"
#include "tools/StringUtils.h"
#include "Level.h"
#include "Engine.h"

namespace Steel
{
    UI::UI(): InputEventListener(),
        mInputMan(nullptr), mWindow(nullptr), mEngine(nullptr),
        mMouseIdentifiers(),
        mMyGUIData(),
        mEditor(*this), mHUD(*this), mUIDataDir(), mEditMode(false)
    {
        buildCodeMaps();
    }

    UI::UI(const UI &o): InputEventListener(o),
        mInputMan(o.mInputMan), mWindow(o.mWindow), mEngine(o.mEngine),
        mMouseIdentifiers(o.mMouseIdentifiers),
        mMyGUIData(o.mMyGUIData),
        mEditor(o.mEditor), mHUD(o.mHUD), mUIDataDir(o.mUIDataDir), mEditMode(o.mEditMode)
    {}

    UI::~UI()
    {
        shutdown();
    }

    UI &UI::operator=(const UI &o)
    {
        Debug::error("not implemented").endl().breakHere();

        if(this != &o)
        {
            mMyGUIData = o.mMyGUIData;

            mEditor = o.mEditor;
            mHUD = o.mHUD;
            mEditMode = o.mEditMode;
        }

        return *this;
    }

    void UI::loadConfig(ConfigFile const &config)
    {
        mEditor.loadConfig(config);
        mHUD.loadConfig(config);
    }

    void UI::saveConfig(ConfigFile &config) const
    {
        mEditor.saveConfig(config);
        mHUD.saveConfig(config);
    }

    bool UI::processCommand(std::vector<Ogre::String> command)
    {
        while(command.size() > 0 && command[0] == "ui")
            command.erase(command.begin());

        if(0 == command.size())
            return false;

        if(command[0] == "reload")
        {
            reload();
        }
        else
        {
            Debug::warning("UI::processCommand(): unknown command ");
            Debug::warning(StringUtils::join(command, ".")).endl();
            return false;
        }

        return true;
    }

    void UI::reload()
    {
        Debug::log(STEEL_METH_INTRO).endl();

        mHUD.reloadContent();
        mEditor.reloadContent();
    }

    void UI::shutdown()
    {
        Debug::log(STEEL_METH_INTRO).endl();
        stopEditMode();
        mHUD.shutdown();
        mEditor.shutdown();

        // MyGui shutdown
        {

            if(nullptr != mMyGUIData.gui)
            {
                MyGUI::FactoryManager &factory = MyGUI::FactoryManager::getInstance();
                factory.unregisterFactory<MyGUI::TreeControl>("Widget");
                factory.unregisterFactory<MyGUI::TreeControlItem>("Widget");

                mMyGUIData.gui->destroyAllChildWidget();
                mMyGUIData.gui->shutdown();
                delete mMyGUIData.gui;
                mMyGUIData.gui = nullptr;
            }

            if(nullptr != mMyGUIData.platform)
            {
                mMyGUIData.platform->shutdown();
                delete mMyGUIData.platform;
                mMyGUIData.platform = nullptr;
            }
        }

        if(mInputMan != nullptr)
            mInputMan = nullptr;
    }

    void UI::init(File UIDataDir,
                  InputManager *inputMan,
                  Ogre::RenderWindow *window,
                  Engine *engine)
    {
        Debug::log(STEEL_METH_INTRO).endl();
        
        mUIDataDir = UIDataDir.subfile("current");
        mInputMan = inputMan;
        mWindow = window;
        mEngine = engine;

        mEditMode = false;
        mInputMan->addInputEventListener(this);

        auto orm = Ogre::ResourceGroupManager::getSingletonPtr();
        orm->addResourceLocation(mUIDataDir.fullPath(), "FileSystem", resourceGroup(), true);
        orm->initialiseResourceGroup(resourceGroup());
        orm->loadResourceGroup(resourceGroup());

        // MyGui init
        {
            auto rgm = Ogre::ResourceGroupManager::getSingletonPtr();
            rgm->addResourceLocation(mUIDataDir / "MyGUI_Media", "FileSystem", resourceGroup(), true);
            rgm->initialiseResourceGroup(resourceGroup());

            mMyGUIData.platform = new MyGUI::OgrePlatform();
            mMyGUIData.platform->initialise(mWindow, engine->level()->sceneManager(), resourceGroup(), "steel_gui.log");

            mMyGUIData.gui = new MyGUI::Gui();
            mMyGUIData.gui->initialise("MyGUI_Core.xml");

            MyGUI::FactoryManager &factory = MyGUI::FactoryManager::getInstance();
            factory.registerFactory<MyGUI::TreeControl>("Widget");
            factory.registerFactory<MyGUI::TreeControlItem>("Widget");
        }

        //UI init
        mEditor.init(mWindow->getWidth(), mWindow->getHeight(), mEngine);
        mHUD.init(mWindow->getWidth(), mWindow->getHeight(), mEngine);
    }

    void UI::onResize(int width, int height)
    {
    }

    void UI::startEditMode()
    {
        if(!mEditMode)
        {
            mEditMode = true;
            mEditor.show();
        }
    }

    void UI::stopEditMode()
    {
        if(mEditMode)
        {
            mEditMode = false;
            mEditor.hide();
        }
    }

    bool UI::keyPressed(Input::Code key, Input::Event const &evt)
    {
        MyGUI::InputManager::getInstance().injectKeyPress(getMyGUIKeyIdentifier(evt.code), (MyGUI::Char)evt.text);

        if(mEditMode)
            mEditor.keyPressed(key, evt);

        return true;
    }

    bool UI::keyReleased(Input::Code key, Input::Event const &evt)
    {
        MyGUI::InputManager::getInstance().injectKeyRelease(getMyGUIKeyIdentifier(evt.code));

        if(mEditMode)
            mEditor.keyReleased(key, evt);

        return true;
    }

    bool UI::mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt)
    {
        auto const mouseState = mInputMan->mouse()->getMouseState();
        MyGUI::InputManager::getInstance().injectMouseMove(mouseState.X.abs, mouseState.Y.abs, mouseState.Z.abs);

        if(mEditMode)
            mEditor.mouseMoved(position, evt);

        return true;
    }

    MyGUI::KeyCode UI::getMyGUIKeyIdentifier(Input::Code key) const
    {
        auto it = mMyGUIData.keyMap.find(key);

        if(mMyGUIData.keyMap.cend() == it)
            return MyGUI::KeyCode::None;

        return it->second;
    }

    int UI::getMouseIdentifier(Input::Code button) const
    {
        auto it = mMouseIdentifiers.find(button);

        if(mMouseIdentifiers.cend() == it)
            return 0; // left btn

        return it->second;
    }

    bool UI::mousePressed(Input::Code button, Input::Event const &evt)
    {
        auto const mouseState = mInputMan->mouse()->getMouseState();
        MyGUI::InputManager::getInstance().injectMousePress(mouseState.X.abs, mouseState.Y.abs, MyGUI::MouseButton::Enum(getMouseIdentifier(button)));

        if(mEditMode)
            mEditor.mousePressed(button, evt);

        return true;
    }

    bool UI::mouseReleased(Input::Code button, Input::Event const &evt)
    {
        auto const mouseState = mInputMan->mouse()->getMouseState();
        MyGUI::InputManager::getInstance().injectMouseRelease(mouseState.X.abs, mouseState.Y.abs, MyGUI::MouseButton::Enum(getMouseIdentifier(button)));

        if(mEditMode)
            mEditor.mouseReleased(button, evt);

        return true;
    }

    bool UI::mouseWheeled(int delta, Input::Event const &evt)
    {
        if(mEditMode)
            mEditor.mouseWheeled(delta, evt);

        return true;
    }

    void UI::buildCodeMaps()
    {
        // mouseMoved
        //MB_Left = 0, MB_Right, MB_Middle,
        //MB_Button3, MB_Button4, MB_Button5, MB_Button6, MB_Button7
        mMouseIdentifiers[Input::Code::MC_LEFT] = 0;
        mMouseIdentifiers[Input::Code::MC_RIGHT] = 1;
        mMouseIdentifiers[Input::Code::MC_MIDDLE] = 2;
        mMouseIdentifiers[Input::Code::MC_BUTTON3] = 3;
        mMouseIdentifiers[Input::Code::MC_BUTTON4] = 4;
        mMouseIdentifiers[Input::Code::MC_BUTTON5] = 5;
        mMouseIdentifiers[Input::Code::MC_BUTTON6] = 6;
        mMouseIdentifiers[Input::Code::MC_BUTTON7] = 7;

        // MyGUI stuff
#define MAP_CODE(STEEL_CODE, MYGUI_CODE) mMyGUIData.keyMap[STEEL_CODE] = MYGUI_CODE
        MAP_CODE(Input::Code::KC_UNKONWN, MyGUI::KeyCode::None);
        MAP_CODE(Input::Code::KC_ESCAPE, MyGUI::KeyCode::Escape);
        MAP_CODE(Input::Code::KC_1, MyGUI::KeyCode::One);
        MAP_CODE(Input::Code::KC_2, MyGUI::KeyCode::Two);
        MAP_CODE(Input::Code::KC_3, MyGUI::KeyCode::Three);
        MAP_CODE(Input::Code::KC_4, MyGUI::KeyCode::Four);
        MAP_CODE(Input::Code::KC_5, MyGUI::KeyCode::Five);
        MAP_CODE(Input::Code::KC_6, MyGUI::KeyCode::Six);
        MAP_CODE(Input::Code::KC_7, MyGUI::KeyCode::Seven);
        MAP_CODE(Input::Code::KC_8, MyGUI::KeyCode::Eight);
        MAP_CODE(Input::Code::KC_9, MyGUI::KeyCode::Nine);
        MAP_CODE(Input::Code::KC_0, MyGUI::KeyCode::Zero);
        MAP_CODE(Input::Code::KC_MINUS, MyGUI::KeyCode::Minus);/* - on main keyboard */
        MAP_CODE(Input::Code::KC_EQUALS, MyGUI::KeyCode::Equals);
        MAP_CODE(Input::Code::KC_BACKSPACE, MyGUI::KeyCode::Backspace);
        MAP_CODE(Input::Code::KC_TAB, MyGUI::KeyCode::Tab);
        MAP_CODE(Input::Code::KC_A, MyGUI::KeyCode::A);
        MAP_CODE(Input::Code::KC_B, MyGUI::KeyCode::B);
        MAP_CODE(Input::Code::KC_C, MyGUI::KeyCode::C);
        MAP_CODE(Input::Code::KC_D, MyGUI::KeyCode::D);
        MAP_CODE(Input::Code::KC_E, MyGUI::KeyCode::E);
        MAP_CODE(Input::Code::KC_F, MyGUI::KeyCode::F);
        MAP_CODE(Input::Code::KC_G, MyGUI::KeyCode::G);
        MAP_CODE(Input::Code::KC_H, MyGUI::KeyCode::H);
        MAP_CODE(Input::Code::KC_I, MyGUI::KeyCode::I);
        MAP_CODE(Input::Code::KC_J, MyGUI::KeyCode::J);
        MAP_CODE(Input::Code::KC_K, MyGUI::KeyCode::K);
        MAP_CODE(Input::Code::KC_L, MyGUI::KeyCode::L);
        MAP_CODE(Input::Code::KC_M, MyGUI::KeyCode::M);
        MAP_CODE(Input::Code::KC_N, MyGUI::KeyCode::N);
        MAP_CODE(Input::Code::KC_O, MyGUI::KeyCode::O);
        MAP_CODE(Input::Code::KC_P, MyGUI::KeyCode::P);
        MAP_CODE(Input::Code::KC_Q, MyGUI::KeyCode::Q);
        MAP_CODE(Input::Code::KC_R, MyGUI::KeyCode::R);
        MAP_CODE(Input::Code::KC_S, MyGUI::KeyCode::S);
        MAP_CODE(Input::Code::KC_T, MyGUI::KeyCode::T);
        MAP_CODE(Input::Code::KC_U, MyGUI::KeyCode::U);
        MAP_CODE(Input::Code::KC_V, MyGUI::KeyCode::V);
        MAP_CODE(Input::Code::KC_W, MyGUI::KeyCode::W);
        MAP_CODE(Input::Code::KC_X, MyGUI::KeyCode::X);
        MAP_CODE(Input::Code::KC_Y, MyGUI::KeyCode::Y);
        MAP_CODE(Input::Code::KC_Z, MyGUI::KeyCode::Z);
        MAP_CODE(Input::Code::KC_LBRACKET, MyGUI::KeyCode::LeftBracket);
        MAP_CODE(Input::Code::KC_RBRACKET, MyGUI::KeyCode::RightBracket);
        MAP_CODE(Input::Code::KC_RETURN, MyGUI::KeyCode::Return);
        MAP_CODE(Input::Code::KC_LCONTROL, MyGUI::KeyCode::LeftControl);
        MAP_CODE(Input::Code::KC_SEMICOLON, MyGUI::KeyCode::Semicolon);
        MAP_CODE(Input::Code::KC_APOSTROPHE, MyGUI::KeyCode::Apostrophe);
        MAP_CODE(Input::Code::KC_GRAVE, MyGUI::KeyCode::Grave);
        MAP_CODE(Input::Code::KC_LSHIFT, MyGUI::KeyCode::LeftShift);
        MAP_CODE(Input::Code::KC_BACKSLASH, MyGUI::KeyCode::Backslash);
        MAP_CODE(Input::Code::KC_COMMA, MyGUI::KeyCode::Comma);
        MAP_CODE(Input::Code::KC_PERIOD, MyGUI::KeyCode::Period);
        MAP_CODE(Input::Code::KC_SLASH, MyGUI::KeyCode::Slash);
        MAP_CODE(Input::Code::KC_RSHIFT, MyGUI::KeyCode::RightShift);
        MAP_CODE(Input::Code::KC_LALT, MyGUI::KeyCode::LeftAlt);
        MAP_CODE(Input::Code::KC_SPACE, MyGUI::KeyCode::Space);
        MAP_CODE(Input::Code::KC_CAPITAL, MyGUI::KeyCode::Capital);
        MAP_CODE(Input::Code::KC_F1, MyGUI::KeyCode::F1);
        MAP_CODE(Input::Code::KC_F2, MyGUI::KeyCode::F2);
        MAP_CODE(Input::Code::KC_F3, MyGUI::KeyCode::F3);
        MAP_CODE(Input::Code::KC_F4, MyGUI::KeyCode::F4);
        MAP_CODE(Input::Code::KC_F5, MyGUI::KeyCode::F5);
        MAP_CODE(Input::Code::KC_F6, MyGUI::KeyCode::F6);
        MAP_CODE(Input::Code::KC_F7, MyGUI::KeyCode::F7);
        MAP_CODE(Input::Code::KC_F8, MyGUI::KeyCode::F8);
        MAP_CODE(Input::Code::KC_F9, MyGUI::KeyCode::F9);
        MAP_CODE(Input::Code::KC_F10, MyGUI::KeyCode::F10);
        MAP_CODE(Input::Code::KC_F11, MyGUI::KeyCode::F11);
        MAP_CODE(Input::Code::KC_F12, MyGUI::KeyCode::F12);
        MAP_CODE(Input::Code::KC_F13, MyGUI::KeyCode::F13);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_F14, MyGUI::KeyCode::F14);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_F15, MyGUI::KeyCode::F15);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_NUMLOCK, MyGUI::KeyCode::NumLock);
        MAP_CODE(Input::Code::KC_SCROLL, MyGUI::KeyCode::ScrollLock);
        MAP_CODE(Input::Code::KC_NUMPAD1, MyGUI::KeyCode::Numpad1);
        MAP_CODE(Input::Code::KC_NUMPAD2, MyGUI::KeyCode::Numpad2);
        MAP_CODE(Input::Code::KC_NUMPAD3, MyGUI::KeyCode::Numpad3);
        MAP_CODE(Input::Code::KC_NUMPAD4, MyGUI::KeyCode::Numpad4);
        MAP_CODE(Input::Code::KC_NUMPAD5, MyGUI::KeyCode::Numpad5);
        MAP_CODE(Input::Code::KC_NUMPAD6, MyGUI::KeyCode::Numpad6);
        MAP_CODE(Input::Code::KC_NUMPAD7, MyGUI::KeyCode::Numpad7);
        MAP_CODE(Input::Code::KC_NUMPAD8, MyGUI::KeyCode::Numpad8);
        MAP_CODE(Input::Code::KC_NUMPAD9, MyGUI::KeyCode::Numpad9);
        MAP_CODE(Input::Code::KC_NUMPAD0, MyGUI::KeyCode::Numpad0);
        MAP_CODE(Input::Code::KC_NUMPADENTER, MyGUI::KeyCode::NumpadEnter);
        MAP_CODE(Input::Code::KC_NUMPADCOMMA, MyGUI::KeyCode::NumpadComma);/* , on numeric keypad (NEC PC98) */
        MAP_CODE(Input::Code::KC_NUMPADEQUALS, MyGUI::KeyCode::NumpadEquals);/* = on numeric keypad (NEC PC98) */
        MAP_CODE(Input::Code::KC_DECIMAL, MyGUI::KeyCode::Decimal);/* . on numeric keypad */
        MAP_CODE(Input::Code::KC_OEM_102, MyGUI::KeyCode::OEM_102);/* < > | on UK/Germany keyboards */
        MAP_CODE(Input::Code::KC_KANA, MyGUI::KeyCode::Kana);/* (Japanese keyboard)            */
        MAP_CODE(Input::Code::KC_ABNT_C1, MyGUI::KeyCode::ABNT_C1);/* / ? on Portugese (Brazilian) keyboards */
        MAP_CODE(Input::Code::KC_ABNT_C2, MyGUI::KeyCode::ABNT_C2);/* Numpad . on Portugese (Brazilian) keyboards */
        MAP_CODE(Input::Code::KC_CONVERT, MyGUI::KeyCode::Convert);/* (Japanese keyboard)            */
        MAP_CODE(Input::Code::KC_NOCONVERT, MyGUI::KeyCode::NoConvert);/* (Japanese keyboard)            */
        MAP_CODE(Input::Code::KC_YEN, MyGUI::KeyCode::Yen);/* (Japanese keyboard)            */
        MAP_CODE(Input::Code::KC_PREVTRACK, MyGUI::KeyCode::PrevTrack);/* Previous Track (KC_CIRCUMFLEX on Japanese keyboard) */
        MAP_CODE(Input::Code::KC_AT, MyGUI::KeyCode::At);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_COLON, MyGUI::KeyCode::Colon);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_UNDERLINE, MyGUI::KeyCode::Underline);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_KANJI, MyGUI::KeyCode::Kanji);/* (Japanese keyboard)            */
        MAP_CODE(Input::Code::KC_STOP, MyGUI::KeyCode::Stop);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_AX, MyGUI::KeyCode::AX);/*                     (Japan AX) */
        MAP_CODE(Input::Code::KC_UNLABELED, MyGUI::KeyCode::Unlabeled);/*                        (J3100) */
        MAP_CODE(Input::Code::KC_NEXTTRACK, MyGUI::KeyCode::NextTrack);
        MAP_CODE(Input::Code::KC_RCONTROL, MyGUI::KeyCode::RightControl);
        MAP_CODE(Input::Code::KC_MUTE, MyGUI::KeyCode::Mute);
        MAP_CODE(Input::Code::KC_CALCULATOR, MyGUI::KeyCode::Calculator);
        MAP_CODE(Input::Code::KC_PLAYPAUSE, MyGUI::KeyCode::PlayPause);
        MAP_CODE(Input::Code::KC_MEDIASTOP, MyGUI::KeyCode::MediaStop);
        MAP_CODE(Input::Code::KC_VOLUMEDOWN, MyGUI::KeyCode::VolumeDown);
        MAP_CODE(Input::Code::KC_VOLUMEUP, MyGUI::KeyCode::VolumeUp);
        MAP_CODE(Input::Code::KC_WEBHOME, MyGUI::KeyCode::WebHome);
        MAP_CODE(Input::Code::KC_ADD, MyGUI::KeyCode::Add);/* + on numeric keypad */
        MAP_CODE(Input::Code::KC_SUBTRACT, MyGUI::KeyCode::Subtract);/* - on numeric keypad */
        MAP_CODE(Input::Code::KC_MULTIPLY, MyGUI::KeyCode::Multiply);/* * on numeric keypad */
        MAP_CODE(Input::Code::KC_DIVIDE, MyGUI::KeyCode::Divide);/* / on numeric keypad */
        MAP_CODE(Input::Code::KC_SCREENSHOT, MyGUI::KeyCode::SysRq);
//         MAP_CODE(Input::Code::KC_, MyGUI::KeyCode::RightAlt);
        MAP_CODE(Input::Code::KC_PAUSE, MyGUI::KeyCode::Pause);
        MAP_CODE(Input::Code::KC_HOME, MyGUI::KeyCode::Home);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_END, MyGUI::KeyCode::End);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_UP, MyGUI::KeyCode::ArrowUp);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_LEFT, MyGUI::KeyCode::ArrowLeft);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_RIGHT, MyGUI::KeyCode::ArrowRight);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_DOWN, MyGUI::KeyCode::ArrowDown);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_PGUP, MyGUI::KeyCode::PageUp);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_PGDOWN, MyGUI::KeyCode::PageDown);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_INSERT, MyGUI::KeyCode::Insert);
        MAP_CODE(Input::Code::KC_DELETE, MyGUI::KeyCode::Delete);
        MAP_CODE(Input::Code::KC_LWIN, MyGUI::KeyCode::LeftWindows);
        MAP_CODE(Input::Code::KC_RWIN, MyGUI::KeyCode::RightWindows);
        MAP_CODE(Input::Code::KC_APPS, MyGUI::KeyCode::AppMenu);
        MAP_CODE(Input::Code::KC_POWER, MyGUI::KeyCode::Power);
        MAP_CODE(Input::Code::KC_SLEEP, MyGUI::KeyCode::Sleep);
        MAP_CODE(Input::Code::KC_WAKE, MyGUI::KeyCode::Wake);
        MAP_CODE(Input::Code::KC_WEBSEARCH, MyGUI::KeyCode::WebSearch);
        MAP_CODE(Input::Code::KC_WEBFAVORITES, MyGUI::KeyCode::WebFavorites);
        MAP_CODE(Input::Code::KC_WEBREFRESH, MyGUI::KeyCode::WebRefresh);
        MAP_CODE(Input::Code::KC_WEBSTOP, MyGUI::KeyCode::WebStop);
        MAP_CODE(Input::Code::KC_WEBFORWARD, MyGUI::KeyCode::WebForward);
        MAP_CODE(Input::Code::KC_WEBBACK, MyGUI::KeyCode::WebBack);
        MAP_CODE(Input::Code::KC_MYCOMPUTER, MyGUI::KeyCode::MyComputer);
        MAP_CODE(Input::Code::KC_MAIL, MyGUI::KeyCode::Mail);
        MAP_CODE(Input::Code::KC_MEDIASELECT, MyGUI::KeyCode::MediaSelect);
#undef MAP_CODE
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

