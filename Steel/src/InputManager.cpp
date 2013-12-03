/*
 * InputManager.cpp
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */
#include "InputManager.h"

#include <iostream>
#include <assert.h>

#if defined OIS_WIN32_PLATFORM

#elif defined OIS_LINUX_PLATFORM

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#endif

using namespace std;

#include "Debug.h"
#include "Engine.h"
#include "InputEventListener.h"
#include "UI/UI.h"
#include <SignalManager.h>

namespace Steel
{

    class equals
    {
    public:
        equals(Input::Code _v) : v(_v) {};
        bool operator()(const Input::Code &value)
        {
            return value == v;
        }
    protected:
        Input::Code v;
    };
    
    InputManager::ModifierCodeIdentifierMap InputManager::sOISModiferToInputCodeMap;
    InputManager::KeyCodeIdentifierMap InputManager::sOISKeyToInputCodeMap;
    InputManager::MouseButtonCodeIdentifierMap InputManager::sOISMouseToInputCodeMap;
    
    InputManager::InputManager() :
        mEngine(nullptr), mOISInputManager(nullptr), mIsInputGrabbed(false), mIsGrabExclusive(false),
        mDelayedInputReleaseRequested(false), mDelayedRequestIsExclusive(false),
        mMouse(nullptr), mKeyboard(nullptr), mCodesPressed(std::list<Input::Code>()), mHasMouseMoved(false),
        mMouseMove(Ogre::Vector2::ZERO), mMousePos(Ogre::Vector2(-1.f, -1.f)), mMouseStateStack(std::list<Ogre::Vector2>()),
        mActionsRegister()
    {
        if(sOISKeyToInputCodeMap.size() == 0 && sOISMouseToInputCodeMap.size() == 0)
            InputManager::buildCodesMaps();
    }

    InputManager::~InputManager()
    {
        mActionsRegister.clear();
    }

    void InputManager::init(Engine *engine)
    {
        mEngine = engine;
        Debug::log("InputManager::init()").endl();
        auto window = mEngine->renderWindow();
        Ogre::WindowEventUtilities::addWindowEventListener(window, this);
        mMousePos = Ogre::Vector2(window->getWidth() / 2, window->getHeight() / 2);

        setMousePosition(mMousePos);
        mMouseStateStack.clear();
        resetAllData();
    }

    void InputManager::shutdown()
    {
        mMouseStateStack.clear();
        releaseInput();
        Ogre::WindowEventUtilities::removeWindowEventListener(mEngine->renderWindow(), this);
    }

    void InputManager::resetFrameBasedData()
    {
        mHasMouseMoved = false;
        mMouseMove = Ogre::Vector2::ZERO;
    }

    void InputManager::resetAllData()
    {
        resetFrameBasedData();
        mActionsRegister.clear();
        mCodesPressed.clear();
    }

    OIS::ParamList InputManager::getOISparams(bool exclusive)
    {
        OIS::ParamList params;
        size_t windowHnd = 0;
        mEngine->renderWindow()->getCustomAttribute("WINDOW", &windowHnd);
        std::ostringstream windowHndStr;
        windowHndStr << windowHnd;

        params.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

        if(exclusive)
        {
#if defined OIS_WIN32_PLATFORM
            //TODO: test these
            params.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
            params.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_EXCLUSIVE")));
            params.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
            params.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_EXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
            params.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("true")));
            params.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("true")));
            params.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
            params.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("false")));
#endif
        }
        else
        {
#if defined OIS_WIN32_PLATFORM
            //TODO: test these
            params.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND")));
            params.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
            params.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
            params.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined OIS_LINUX_PLATFORM
            params.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
            params.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
            params.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
            params.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("false")));
#endif
        }

        return params;
    }

    void InputManager::grabInput(bool exclusive)
    {
        mDelayedInputGrabRequested = true;
        mDelayedRequestIsExclusive = exclusive;
    }

    void InputManager::_grabInput(bool exclusive)
    {
        if(mIsInputGrabbed)
        {
            if(mIsGrabExclusive == exclusive)
                return;
            else
                _releaseInput();
        }

//         Debug::log("InputManager::_grabInput(exclusive=")(exclusive)(") ");


        assert(mOISInputManager == nullptr);
        assert(mKeyboard == nullptr);
        assert(mMouse == nullptr);

        mIsGrabExclusive = exclusive;
        assert(mOISInputManager == nullptr);
        auto params = getOISparams(exclusive);
        mOISInputManager = OIS::InputManager::createInputSystem(params);

        bool bufferedMouse = true;
        mMouse = static_cast<OIS::Mouse *>(mOISInputManager->createInputObject(OIS::OISMouse, bufferedMouse));
        mMouse->setEventCallback(this);

        const OIS::MouseState &ms = mMouse->getMouseState();
        ms.width = mEngine->renderWindow()->getWidth();
        ms.height = mEngine->renderWindow()->getHeight();

        if(mIsGrabExclusive)
        {
//             Debug::log("keeping mouse within ")(ms.width)(" and ")(ms.height).endl();
            pushMouseState();
        }
        else
        {
//             Debug::log("keeping mouse free to leave the window.").endl();
            popMouseState();
        }

        bool bufferedKeys = true;
        mKeyboard = static_cast<OIS::Keyboard *>(mOISInputManager->createInputObject(OIS::OISKeyboard, bufferedKeys));
        mKeyboard->setEventCallback(this);
        mCodesPressed.clear();
        mMousePos = Ogre::Vector2::ZERO;
        mIsInputGrabbed = true;
        mDelayedInputGrabRequested = false;
        assert(mOISInputManager != nullptr);
        assert(mKeyboard != nullptr);
        assert(mMouse != nullptr);
    }

    void InputManager::releaseInput()
    {
        mDelayedInputReleaseRequested = true;
    }

    void InputManager::_releaseInput()
    {
        if(mMouse != nullptr)
        {
            mOISInputManager->destroyInputObject(mMouse);
            mMouse = nullptr;
        }

        if(mKeyboard != nullptr)
        {
            mOISInputManager->destroyInputObject(mKeyboard);
            mKeyboard = nullptr;
        }

        if(mOISInputManager != nullptr)
        {
            OIS::InputManager::destroyInputSystem(mOISInputManager);
            mOISInputManager = nullptr;
        }

        mIsInputGrabbed = false;
        mDelayedInputReleaseRequested = false;
    }

    bool InputManager::keyPressed(const OIS::KeyEvent &evt)
    {
        Input::Code code = getKeyInputCode(evt.key);
        mCodesPressed.push_back(code);
        mCodesPressed.unique();

        fireInputEvent( {code, Input::Type::DOWN, Input::Device::KEYBOARD, evt.text});
        return true;
    }

    bool InputManager::keyReleased(const OIS::KeyEvent &evt)
    {
        //  cout << "InputManager::keyReleased()" << endl;
        Input::Code code = getKeyInputCode(evt.key);

        if(!isKeyDown(code))
            return true;

        mCodesPressed.remove_if(equals(code));

        fireInputEvent( {code, Input::Type::UP, Input::Device::KEYBOARD, evt.text});

        return true;
    }

    bool InputManager::mouseMoved(const OIS::MouseEvent &evt)
    {
        mHasMouseMoved = true;
        const OIS::MouseState ms = evt.state;

        mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);
        mMouseMove = Ogre::Vector2(ms.X.rel, ms.Y.rel);

        // OIS bug
        if(mMouseMove.length() > 300.f)
            mMouseMove = mLastMouseMove;

//         Debug::log("mMouseMove: ")(mMouseMove)("(")(mMouseMove.length())(")").endl();
        mLastMouseMove = mMouseMove;

        fireInputEvent( {Input::Code::MC_POINTER, Input::Type::MOVE, Input::Device::MOUSE, mMousePos, mMouseMove});
        return true;
    }

    bool InputManager::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
    {
        OIS::MouseState ms = evt.state;
        mMousePosAtLastMousePressed = mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);

        fireInputEvent( {getMouseInputCode(id), Input::Type::DOWN, Input::Device::MOUSE, mMousePos, mMouseMove});
        return true;
    }

    bool InputManager::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
    {
        OIS::MouseState ms = evt.state;
        mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);

        fireInputEvent( {getMouseInputCode(id), Input::Type::UP, Input::Device::MOUSE, mMousePos, mMouseMove});
        return true;
    }

    void InputManager::pushMouseState()
    {
        if(mMouse == nullptr)
        {
            Debug::warning("InputManager::pushMouseState(): no mMouse defined, aborting. (try grabbing first).").endl();
            return;
        }

        mMouseStateStack.push_front(mMousePos);
    }

    void InputManager::popMouseState()
    {
        if(mMouseStateStack.size() == 0)
        {
            Debug::warning("InputManager::popMouseState(): mouse states stack is empty, using (0,0).").endl();
            auto window = mEngine->renderWindow();
            mMouseStateStack.push_front(Ogre::Vector2(window->getWidth() / 2, window->getHeight() / 2));
        }

        Ogre::Vector2 pos = mMouseStateStack.front();
        mMouseStateStack.pop_front();
        setMousePosition(pos);
    }

    void InputManager::setMousePosition(Ogre::Vector2 &pos)
    {
        if(mMouse == nullptr)
        {
//             Debug::warning("InputManager::setMousePosition(): no mMouse defined, aborting. (try grabbing first).").endl();
            return;
        }

#if defined OIS_WIN32_PLATFORM
        //TODO: go see SetCursorPos
#elif defined OIS_LINUX_PLATFORM

        unsigned long windowHandle;
        mEngine->renderWindow()->getCustomAttribute("WINDOW", &windowHandle);

        Display *display = XOpenDisplay(0);
        XWarpPointer(display, 0, windowHandle, 0, 0, 0, 0, pos.x, pos.y);
        XCloseDisplay(display);

        // set internal data to right values (let OIS deal with its internals though.)
        mMousePos = pos;

        //TODO: fix OIS bug (?) that make mouse coords at (-6,-6) of real coords
        OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
        mutableMouseState.X.abs = 6;
        mutableMouseState.Y.abs = 6;
#endif
    }

    void InputManager::update()
    {
        // Pump window messages for nice behaviour
        Ogre::WindowEventUtilities::messagePump();

        if(mDelayedInputReleaseRequested)
            _releaseInput();

        if(mDelayedInputGrabRequested)
            _grabInput(mDelayedRequestIsExclusive);

        if(mOISInputManager == nullptr)
            return;

        fireModifiers();

        if(nullptr != mKeyboard)
            mKeyboard->capture();

        if(nullptr != mMouse)
            mMouse->capture();
    }

    void InputManager::fireModifiers()
    {
        if(nullptr == mKeyboard)
            return;

        for(auto const & pair : sOISModiferToInputCodeMap)
        {
            bool wasPressed = isKeyDown(pair.second);
            bool isPressed = mKeyboard->isModifierDown(pair.first);

            if(!wasPressed && isPressed)
            {
                fireInputEvent( {pair.second, Input::Type::DOWN, Input::Device::KEYBOARD, Input::Event::NOT_TEXT});
            }
            else if(wasPressed && !isPressed)
            {
                fireInputEvent( {pair.second, Input::Type::UP, Input::Device::KEYBOARD, Input::Event::NOT_TEXT});
            }
        }
    }

    void InputManager::windowResized(Ogre::RenderWindow *rw)
    {
        unsigned int width, height, depth;
        int left, top;
        rw->getMetrics(width, height, depth, left, top);

        if(nullptr != mMouse)
        {
            const OIS::MouseState &ms = mMouse->getMouseState();
            ms.width = width;
            ms.height = height;
        }

        mEngine->resizeWindow(width, height);
    }

    void InputManager::windowClosed(Ogre::RenderWindow *rw)
    {
        //Only close for window that created OIS (the main window in these demos)
        if(rw == mEngine->renderWindow())
        {
            mEngine->shutdown();
        }
    }

    void InputManager::addInputEventListener(InputEventListener *listener)
    {
        mListeners.insert(listener);
    }

    void InputManager::removeInputEventListener(InputEventListener *listener)
    {
        mListeners.erase(listener);
    }
    
    void InputManager::registerAction(Steel::Input::Code const code, Input::Type const type, Signal const signal)
    {
        mActionsRegister.emplace(std::make_pair(code, type), std::list<Tag>()).first->second.push_back(signal);
    }

    void InputManager::fireInputEvent(Input::Event evt)
    {
        // fire raw event
        for(auto listener : std::list<InputEventListener *>(mListeners.begin(), mListeners.end()))
            listener->onInputEvent(evt);
        
        // fire action
        auto it = mActionsRegister.find(std::make_pair(evt.code, evt.type));
        if(mActionsRegister.end()!=it)
        {
            for(Signal const &sig:it->second)
                SignalManager::instance().emit(sig);
            SignalManager::instance().fireEmittedSignals();
        }
    }
    
    Input::Code InputManager::getModifierInputCode(OIS::Keyboard::Modifier mod)
    {
        auto it = sOISModiferToInputCodeMap.find(mod);
        return sOISModiferToInputCodeMap.end() == it ? Input::Code::UNASSIGNED : it->second;
    }

    Input::Code InputManager::getKeyInputCode(OIS::KeyCode key)
    {
        auto it = sOISKeyToInputCodeMap.find(key);
        return sOISKeyToInputCodeMap.end() == it ? Input::Code::UNASSIGNED : it->second;
    }

    Input::Code InputManager::getMouseInputCode(OIS::MouseButtonID btn)
    {
        auto it = sOISMouseToInputCodeMap.find(btn);
        return sOISMouseToInputCodeMap.end() == it ? Input::Code::UNASSIGNED : it->second;
    }

    void InputManager::buildCodesMaps()
    {
        sOISKeyToInputCodeMap[OIS::KC_UNASSIGNED] = Input::Code::UNASSIGNED;
        sOISKeyToInputCodeMap[OIS::KC_ESCAPE] = Input::Code::KC_ESCAPE;
        sOISKeyToInputCodeMap[OIS::KC_1] = Input::Code::KC_1;
        sOISKeyToInputCodeMap[OIS::KC_2] = Input::Code::KC_2;
        sOISKeyToInputCodeMap[OIS::KC_3] = Input::Code::KC_3;
        sOISKeyToInputCodeMap[OIS::KC_4] = Input::Code::KC_4;
        sOISKeyToInputCodeMap[OIS::KC_5] = Input::Code::KC_5;
        sOISKeyToInputCodeMap[OIS::KC_6] = Input::Code::KC_6;
        sOISKeyToInputCodeMap[OIS::KC_7] = Input::Code::KC_7;
        sOISKeyToInputCodeMap[OIS::KC_8] = Input::Code::KC_8;
        sOISKeyToInputCodeMap[OIS::KC_9] = Input::Code::KC_9;
        sOISKeyToInputCodeMap[OIS::KC_0] = Input::Code::KC_0;
        sOISKeyToInputCodeMap[OIS::KC_MINUS] = Input::Code::KC_MINUS;
        sOISKeyToInputCodeMap[OIS::KC_EQUALS] = Input::Code::KC_EQUALS;
        sOISKeyToInputCodeMap[OIS::KC_BACK] = Input::Code::KC_BACK;
        sOISKeyToInputCodeMap[OIS::KC_TAB] = Input::Code::KC_TAB;
        sOISKeyToInputCodeMap[OIS::KC_Q] = Input::Code::KC_Q;
        sOISKeyToInputCodeMap[OIS::KC_W] = Input::Code::KC_W;
        sOISKeyToInputCodeMap[OIS::KC_E] = Input::Code::KC_E;
        sOISKeyToInputCodeMap[OIS::KC_R] = Input::Code::KC_R;
        sOISKeyToInputCodeMap[OIS::KC_T] = Input::Code::KC_T;
        sOISKeyToInputCodeMap[OIS::KC_Y] = Input::Code::KC_Y;
        sOISKeyToInputCodeMap[OIS::KC_U] = Input::Code::KC_U;
        sOISKeyToInputCodeMap[OIS::KC_I] = Input::Code::KC_I;
        sOISKeyToInputCodeMap[OIS::KC_O] = Input::Code::KC_O;
        sOISKeyToInputCodeMap[OIS::KC_P] = Input::Code::KC_P;
        sOISKeyToInputCodeMap[OIS::KC_LBRACKET] = Input::Code::KC_LBRACKET;
        sOISKeyToInputCodeMap[OIS::KC_RBRACKET] = Input::Code::KC_RBRACKET;
        sOISKeyToInputCodeMap[OIS::KC_RETURN] = Input::Code::KC_RETURN;
        sOISKeyToInputCodeMap[OIS::KC_LCONTROL] = Input::Code::KC_LCONTROL;
        sOISKeyToInputCodeMap[OIS::KC_A] = Input::Code::KC_A;
        sOISKeyToInputCodeMap[OIS::KC_S] = Input::Code::KC_S;
        sOISKeyToInputCodeMap[OIS::KC_D] = Input::Code::KC_D;
        sOISKeyToInputCodeMap[OIS::KC_F] = Input::Code::KC_F;
        sOISKeyToInputCodeMap[OIS::KC_G] = Input::Code::KC_G;
        sOISKeyToInputCodeMap[OIS::KC_H] = Input::Code::KC_H;
        sOISKeyToInputCodeMap[OIS::KC_J] = Input::Code::KC_J;
        sOISKeyToInputCodeMap[OIS::KC_K] = Input::Code::KC_K;
        sOISKeyToInputCodeMap[OIS::KC_L] = Input::Code::KC_L;
        sOISKeyToInputCodeMap[OIS::KC_SEMICOLON] = Input::Code::KC_SEMICOLON;
        sOISKeyToInputCodeMap[OIS::KC_APOSTROPHE] = Input::Code::KC_APOSTROPHE;
        sOISKeyToInputCodeMap[OIS::KC_GRAVE] = Input::Code::KC_GRAVE;
        sOISKeyToInputCodeMap[OIS::KC_LSHIFT] = Input::Code::KC_LSHIFT;
        sOISKeyToInputCodeMap[OIS::KC_BACKSLASH] = Input::Code::KC_BACKSLASH;
        sOISKeyToInputCodeMap[OIS::KC_Z] = Input::Code::KC_Z;
        sOISKeyToInputCodeMap[OIS::KC_X] = Input::Code::KC_X;
        sOISKeyToInputCodeMap[OIS::KC_C] = Input::Code::KC_C;
        sOISKeyToInputCodeMap[OIS::KC_V] = Input::Code::KC_V;
        sOISKeyToInputCodeMap[OIS::KC_B] = Input::Code::KC_B;
        sOISKeyToInputCodeMap[OIS::KC_N] = Input::Code::KC_N;
        sOISKeyToInputCodeMap[OIS::KC_M] = Input::Code::KC_M;
        sOISKeyToInputCodeMap[OIS::KC_COMMA] = Input::Code::KC_COMMA;
        sOISKeyToInputCodeMap[OIS::KC_PERIOD] = Input::Code::KC_PERIOD;
        sOISKeyToInputCodeMap[OIS::KC_SLASH] = Input::Code::KC_SLASH;
        sOISKeyToInputCodeMap[OIS::KC_RSHIFT] = Input::Code::KC_RSHIFT;
        sOISKeyToInputCodeMap[OIS::KC_MULTIPLY] = Input::Code::KC_MULTIPLY;
        sOISKeyToInputCodeMap[OIS::KC_LMENU] = Input::Code::KC_LMENU;
        sOISKeyToInputCodeMap[OIS::KC_SPACE] = Input::Code::KC_SPACE;
        sOISKeyToInputCodeMap[OIS::KC_CAPITAL] = Input::Code::KC_CAPITAL;
        sOISKeyToInputCodeMap[OIS::KC_F1] = Input::Code::KC_F1;
        sOISKeyToInputCodeMap[OIS::KC_F2] = Input::Code::KC_F2;
        sOISKeyToInputCodeMap[OIS::KC_F3] = Input::Code::KC_F3;
        sOISKeyToInputCodeMap[OIS::KC_F4] = Input::Code::KC_F4;
        sOISKeyToInputCodeMap[OIS::KC_F5] = Input::Code::KC_F5;
        sOISKeyToInputCodeMap[OIS::KC_F6] = Input::Code::KC_F6;
        sOISKeyToInputCodeMap[OIS::KC_F7] = Input::Code::KC_F7;
        sOISKeyToInputCodeMap[OIS::KC_F8] = Input::Code::KC_F8;
        sOISKeyToInputCodeMap[OIS::KC_F9] = Input::Code::KC_F9;
        sOISKeyToInputCodeMap[OIS::KC_F10] = Input::Code::KC_F10;
        sOISKeyToInputCodeMap[OIS::KC_NUMLOCK] = Input::Code::KC_NUMLOCK;
        sOISKeyToInputCodeMap[OIS::KC_SCROLL] = Input::Code::KC_SCROLL;
        sOISKeyToInputCodeMap[OIS::KC_NUMPAD7] = Input::Code::KC_NUMPAD7;
        sOISKeyToInputCodeMap[OIS::KC_NUMPAD8] = Input::Code::KC_NUMPAD8;
        sOISKeyToInputCodeMap[OIS::KC_NUMPAD9] = Input::Code::KC_NUMPAD9;
        sOISKeyToInputCodeMap[OIS::KC_SUBTRACT] = Input::Code::KC_SUBTRACT;
        sOISKeyToInputCodeMap[OIS::KC_NUMPAD4] = Input::Code::KC_NUMPAD4;
        sOISKeyToInputCodeMap[OIS::KC_NUMPAD5] = Input::Code::KC_NUMPAD5;
        sOISKeyToInputCodeMap[OIS::KC_NUMPAD6] = Input::Code::KC_NUMPAD6;
        sOISKeyToInputCodeMap[OIS::KC_ADD] = Input::Code::KC_ADD;
        sOISKeyToInputCodeMap[OIS::KC_NUMPAD1] = Input::Code::KC_NUMPAD1;
        sOISKeyToInputCodeMap[OIS::KC_NUMPAD2] = Input::Code::KC_NUMPAD2;
        sOISKeyToInputCodeMap[OIS::KC_NUMPAD3] = Input::Code::KC_NUMPAD3;
        sOISKeyToInputCodeMap[OIS::KC_NUMPAD0] = Input::Code::KC_NUMPAD0;
        sOISKeyToInputCodeMap[OIS::KC_DECIMAL] = Input::Code::KC_DECIMAL;
        sOISKeyToInputCodeMap[OIS::KC_OEM_102] = Input::Code::KC_OEM_102;
        sOISKeyToInputCodeMap[OIS::KC_F11] = Input::Code::KC_F11;
        sOISKeyToInputCodeMap[OIS::KC_F12] = Input::Code::KC_F12;
        sOISKeyToInputCodeMap[OIS::KC_F13] = Input::Code::KC_F13;
        sOISKeyToInputCodeMap[OIS::KC_F14] = Input::Code::KC_F14;
        sOISKeyToInputCodeMap[OIS::KC_F15] = Input::Code::KC_F15;
        sOISKeyToInputCodeMap[OIS::KC_KANA] = Input::Code::KC_KANA;
        sOISKeyToInputCodeMap[OIS::KC_ABNT_C1] = Input::Code::KC_ABNT_C1;
        sOISKeyToInputCodeMap[OIS::KC_CONVERT] = Input::Code::KC_CONVERT;
        sOISKeyToInputCodeMap[OIS::KC_NOCONVERT] = Input::Code::KC_NOCONVERT;
        sOISKeyToInputCodeMap[OIS::KC_YEN] = Input::Code::KC_YEN;
        sOISKeyToInputCodeMap[OIS::KC_ABNT_C2] = Input::Code::KC_ABNT_C2;
        sOISKeyToInputCodeMap[OIS::KC_NUMPADEQUALS] = Input::Code::KC_NUMPADEQUALS;
        sOISKeyToInputCodeMap[OIS::KC_PREVTRACK] = Input::Code::KC_PREVTRACK;
        sOISKeyToInputCodeMap[OIS::KC_AT] = Input::Code::KC_AT;
        sOISKeyToInputCodeMap[OIS::KC_COLON] = Input::Code::KC_COLON;
        sOISKeyToInputCodeMap[OIS::KC_UNDERLINE] = Input::Code::KC_UNDERLINE;
        sOISKeyToInputCodeMap[OIS::KC_KANJI] = Input::Code::KC_KANJI;
        sOISKeyToInputCodeMap[OIS::KC_STOP] = Input::Code::KC_STOP;
        sOISKeyToInputCodeMap[OIS::KC_AX] = Input::Code::KC_AX;
        sOISKeyToInputCodeMap[OIS::KC_UNLABELED] = Input::Code::KC_UNLABELED;
        sOISKeyToInputCodeMap[OIS::KC_NEXTTRACK] = Input::Code::KC_NEXTTRACK;
        sOISKeyToInputCodeMap[OIS::KC_NUMPADENTER] = Input::Code::KC_NUMPADENTER;
        sOISKeyToInputCodeMap[OIS::KC_RCONTROL] = Input::Code::KC_RCONTROL;
        sOISKeyToInputCodeMap[OIS::KC_MUTE] = Input::Code::KC_MUTE;
        sOISKeyToInputCodeMap[OIS::KC_CALCULATOR] = Input::Code::KC_CALCULATOR;
        sOISKeyToInputCodeMap[OIS::KC_PLAYPAUSE] = Input::Code::KC_PLAYPAUSE;
        sOISKeyToInputCodeMap[OIS::KC_MEDIASTOP] = Input::Code::KC_MEDIASTOP;
        sOISKeyToInputCodeMap[OIS::KC_VOLUMEDOWN] = Input::Code::KC_VOLUMEDOWN;
        sOISKeyToInputCodeMap[OIS::KC_VOLUMEUP] = Input::Code::KC_VOLUMEUP;
        sOISKeyToInputCodeMap[OIS::KC_WEBHOME] = Input::Code::KC_WEBHOME;
        sOISKeyToInputCodeMap[OIS::KC_NUMPADCOMMA] = Input::Code::KC_NUMPADCOMMA;
        sOISKeyToInputCodeMap[OIS::KC_DIVIDE] = Input::Code::KC_DIVIDE;
        sOISKeyToInputCodeMap[OIS::KC_SYSRQ] = Input::Code::KC_SCREENSHOT;
        sOISKeyToInputCodeMap[OIS::KC_RMENU] = Input::Code::KC_RMENU;
        sOISKeyToInputCodeMap[OIS::KC_PAUSE] = Input::Code::KC_PAUSE;
        sOISKeyToInputCodeMap[OIS::KC_HOME] = Input::Code::KC_HOME;
        sOISKeyToInputCodeMap[OIS::KC_UP] = Input::Code::KC_UP;
        sOISKeyToInputCodeMap[OIS::KC_PGUP] = Input::Code::KC_PGUP;
        sOISKeyToInputCodeMap[OIS::KC_LEFT] = Input::Code::KC_LEFT;
        sOISKeyToInputCodeMap[OIS::KC_RIGHT] = Input::Code::KC_RIGHT;
        sOISKeyToInputCodeMap[OIS::KC_END] = Input::Code::KC_END;
        sOISKeyToInputCodeMap[OIS::KC_DOWN] = Input::Code::KC_DOWN;
        sOISKeyToInputCodeMap[OIS::KC_PGDOWN] = Input::Code::KC_PGDOWN;
        sOISKeyToInputCodeMap[OIS::KC_INSERT] = Input::Code::KC_INSERT;
        sOISKeyToInputCodeMap[OIS::KC_DELETE] = Input::Code::KC_DELETE;
        sOISKeyToInputCodeMap[OIS::KC_LWIN] = Input::Code::KC_LWIN;
        sOISKeyToInputCodeMap[OIS::KC_RWIN] = Input::Code::KC_RWIN;
        sOISKeyToInputCodeMap[OIS::KC_APPS] = Input::Code::KC_APPS;
        sOISKeyToInputCodeMap[OIS::KC_POWER] = Input::Code::KC_POWER;
        sOISKeyToInputCodeMap[OIS::KC_SLEEP] = Input::Code::KC_SLEEP;
        sOISKeyToInputCodeMap[OIS::KC_WAKE] = Input::Code::KC_WAKE;
        sOISKeyToInputCodeMap[OIS::KC_WEBSEARCH] = Input::Code::KC_WEBSEARCH;
        sOISKeyToInputCodeMap[OIS::KC_WEBFAVORITES] = Input::Code::KC_WEBFAVORITES;
        sOISKeyToInputCodeMap[OIS::KC_WEBREFRESH] = Input::Code::KC_WEBREFRESH;
        sOISKeyToInputCodeMap[OIS::KC_WEBSTOP] = Input::Code::KC_WEBSTOP;
        sOISKeyToInputCodeMap[OIS::KC_WEBFORWARD] = Input::Code::KC_WEBFORWARD;
        sOISKeyToInputCodeMap[OIS::KC_WEBBACK] = Input::Code::KC_WEBBACK;
        sOISKeyToInputCodeMap[OIS::KC_MYCOMPUTER] = Input::Code::KC_MYCOMPUTER;
        sOISKeyToInputCodeMap[OIS::KC_MAIL] = Input::Code::KC_MAIL;
        sOISKeyToInputCodeMap[OIS::KC_MEDIASELECT] = Input::Code::KC_MEDIASELECT;
        
        // modifiers (as regular keycodes)
        sOISModiferToInputCodeMap[OIS::Keyboard::Ctrl] = Input::Code::KC_LCONTROL;
        sOISModiferToInputCodeMap[OIS::Keyboard::Alt] = Input::Code::KC_ALT;
        sOISModiferToInputCodeMap[OIS::Keyboard::Shift] = Input::Code::KC_LSHIFT;
        
        // mouse
        sOISMouseToInputCodeMap[OIS::MouseButtonID::MB_Left] = Input::Code::MC_LEFT;
        sOISMouseToInputCodeMap[OIS::MouseButtonID::MB_Right] = Input::Code::MC_RIGHT;
        sOISMouseToInputCodeMap[OIS::MouseButtonID::MB_Middle] = Input::Code::MC_MIDDLE;
        sOISMouseToInputCodeMap[OIS::MouseButtonID::MB_Button3] = Input::Code::MC_WHEEL;
        sOISMouseToInputCodeMap[OIS::MouseButtonID::MB_Button4] = Input::Code::MC_BUTTON4;
        sOISMouseToInputCodeMap[OIS::MouseButtonID::MB_Button5] = Input::Code::MC_BUTTON5;
        sOISMouseToInputCodeMap[OIS::MouseButtonID::MB_Button6] = Input::Code::MC_BUTTON6;
        sOISMouseToInputCodeMap[OIS::MouseButtonID::MB_Button7] = Input::Code::MC_BUTTON7;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


