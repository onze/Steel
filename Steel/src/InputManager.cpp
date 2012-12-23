/*
 * InputManager.cpp
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */
#include <iostream>
#include "InputManager.h"
#include "Debug.h"
#include "Engine.h"
#include <assert.h>

using namespace std;

class equals
{
    public:
        equals(OIS::KeyCode _v) :
            v(_v)
        {
        }
        ;
        bool operator()(const OIS::KeyCode& value)
        {
            return value == v;
        }
    protected:
        OIS::KeyCode v;
};

namespace Steel
{

    InputManager::InputManager() :
        mEngine(NULL), mUI(NULL),mOISInputManager(NULL),mIsInputGrabbed(false),mIsGrabExclusive(false),
        mMouse(NULL),mKeyboard(NULL), mKeysPressed(std::list<OIS::KeyCode>()), mHasMouseMoved(false),
        mMouseMove(Ogre::Vector2::ZERO),mMousePos(Ogre::Vector2(-1.f,-1.f))
    {
    }

    InputManager::~InputManager()
    {

    }

    void InputManager::init(Engine *engine,UI *ui)
    {
        mEngine=engine;
        mUI=ui;
        Debug::log("InputManager::init()").endl();
        Ogre::WindowEventUtilities::addWindowEventListener( mEngine->renderWindow(),this);
        resetAllData();
    }

    void InputManager::shutdown()
    {
        releaseInput();
        Ogre::WindowEventUtilities::removeWindowEventListener(mEngine->renderWindow(),this);
    }

    void InputManager::resetFrameBasedData()
    {
        mHasMouseMoved = false;
        mMouseMove = Ogre::Vector2::ZERO;
    }

    void InputManager::resetAllData()
    {
        resetFrameBasedData();
        mKeysPressed.clear();
    }

    OIS::ParamList InputManager::getOISparams(bool exclusive)
    {
        OIS::ParamList params;
        size_t windowHnd = 0;
        mEngine->renderWindow()->getCustomAttribute("WINDOW", &windowHnd);
        std::ostringstream windowHndStr;
        windowHndStr << windowHnd;

        params.insert(std::make_pair(std::string("WINDOW"),windowHndStr.str()));
        if(exclusive)
        {
#if defined OIS_WIN32_PLATFORM
            //TODO: test these
            params.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
            params.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
            params.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
            params.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
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
            params.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
            params.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_EXCLUSIVE")));
            params.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
            params.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_EXCLUSIVE")));
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
        mDelayedInputGrabRequested=true;
        mDelayedRequestIsExclusive=exclusive;
    }

    void InputManager::_grabInput(bool exclusive)
    {
        if(mIsInputGrabbed)
        {
            if(mIsGrabExclusive==exclusive)
                return;
            else
                _releaseInput();
        }
        Debug::log("InputManager::_grabInput(exclusive=")(exclusive)(") ");


        assert(mOISInputManager==NULL);
        assert(mKeyboard==NULL);
        assert(mMouse==NULL);

        mIsGrabExclusive=exclusive;
        assert(mOISInputManager==NULL);
        auto params=getOISparams(exclusive);
        mOISInputManager= OIS::InputManager::createInputSystem(params);

        bool bufferedMouse = true;
        mMouse = static_cast<OIS::Mouse*>(mOISInputManager->createInputObject(OIS::OISMouse,bufferedMouse));
        mMouse->setEventCallback(this);

        if(mIsGrabExclusive)
        {
            const OIS::MouseState &ms = mMouse->getMouseState();
            ms.width = mEngine->renderWindow()->getWidth();
            ms.height = mEngine->renderWindow()->getHeight();
            Debug::log("keeping mouse within ")(ms.width)(" and ")(ms.height).endl();
        }
        else
            Debug::log("keeping mouse free to leave the window.").endl();

        bool bufferedKeys = true;
        mKeyboard = static_cast<OIS::Keyboard*>(mOISInputManager->createInputObject(OIS::OISKeyboard,bufferedKeys));
        mKeyboard->setEventCallback(this);
        mKeysPressed.clear();
        mIsInputGrabbed=true;
        mDelayedInputGrabRequested=false;
        assert(mOISInputManager!=NULL);
        assert(mKeyboard!=NULL);
        assert(mMouse!=NULL);
    }

    void InputManager::releaseInput()
    {
        mDelayedInputReleaseRequested=true;
    }

    void InputManager::_releaseInput()
    {
        if(mMouse!=NULL)
        {
            mOISInputManager->destroyInputObject(mMouse);
            mMouse=NULL;
        }
        if(mKeyboard!=NULL)
        {
            mOISInputManager->destroyInputObject(mKeyboard);
            mKeyboard=NULL;
        }
        if(mOISInputManager!=NULL)
        {
            OIS::InputManager::destroyInputSystem(mOISInputManager);
            mOISInputManager=NULL;
        }
        mIsInputGrabbed=false;
        mDelayedInputReleaseRequested=false;
    }

    bool InputManager::keyPressed(const OIS::KeyEvent& evt)
    {
        mKeysPressed.push_back(evt.key);
        mKeysPressed.unique();

        mEngine->keyPressed(evt);
        mUI->keyPressed(evt);
        return true;
    }

    bool InputManager::keyReleased(const OIS::KeyEvent& evt)
    {
//	cout << "InputManager::keyReleased()" << endl;
        mKeysPressed.remove_if(equals(evt.key));

        mEngine->keyReleased(evt);
        mUI->keyReleased(evt);
        return true;
    }

    bool InputManager::mouseMoved(const OIS::MouseEvent& evt)
    {
        mHasMouseMoved = true;

        OIS::MouseState ms = evt.state;
        mMouseMove += Ogre::Vector2(ms.X.rel, ms.Y.rel);
        mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);
//         Debug::log(mMouseMove).endl();

        mEngine->mouseMoved(evt);
        mUI->mouseMoved(evt);
        return true;
    }

    bool InputManager::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        OIS::MouseState ms = evt.state;
        mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);

        mEngine->mousePressed(evt,id);
        mUI->mousePressed(evt,id);
        return true;
    }

    bool InputManager::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        OIS::MouseState ms = evt.state;
        mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);

        mEngine->mouseReleased(evt,id);
        mUI->mouseReleased(evt,id);
        return true;
    }

    void InputManager::update()
    {
        // Pump window messages for nice behaviour
        Ogre::WindowEventUtilities::messagePump();
        
        
        if(mDelayedInputReleaseRequested)
            _releaseInput();
        if(mDelayedInputGrabRequested)
            _grabInput(mDelayedRequestIsExclusive);
        
        if (mOISInputManager == NULL)
            return;

        if(mKeyboard)
            mKeyboard->capture();
        if(mMouse)
            mMouse->capture();
    }

    void InputManager::windowResized(Ogre::RenderWindow* rw)
    {
        unsigned int width, height, depth;
        int left, top;
        rw->getMetrics(width, height, depth, left, top);

        const OIS::MouseState &ms = mMouse->getMouseState();
        ms.width = width;
        ms.height = height;

        mEngine->resizeWindow(width, height);
    }

    void InputManager::windowClosed(Ogre::RenderWindow* rw)
    {
        //Only close for window that created OIS (the main window in these demos)
        if (rw == mEngine->renderWindow())
        {
            mEngine->shutdown();
        }
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


