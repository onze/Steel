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
        mEngine(NULL), mOISInputManager(NULL), mMouse(NULL), mKeyboard(NULL),
        mKeysPressed(std::list<OIS::KeyCode>()), mHasMouseMoved(false),
        mMousePos(Ogre::Vector2(-1.f,-1.f))
    {
    }

    InputManager::~InputManager()
    {

    }

    void InputManager::init(Engine *engine,UI *ui)
    {
        mEngine=engine;
        mUI=ui;

        Debug::log("InputManager::grab()").endl();
        OIS::ParamList params;

        size_t windowHnd = 0;
        std::ostringstream windowHndStr;
        mEngine->renderWindow()->getCustomAttribute("WINDOW", &windowHnd);
        windowHndStr << windowHnd;
        params.insert(std::make_pair(std::string("WINDOW"),windowHndStr.str()));
        #if defined OIS_WIN32_PLATFORM
        //TODO: test these
        params.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
        params.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
        params.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
        params.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
        #elif defined OIS_LINUX_PLATFORM
        params.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
//         params.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
        params.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
        params.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("false")));
        #endif
        mOISInputManager = OIS::InputManager::createInputSystem(params);

        Ogre::WindowEventUtilities::addWindowEventListener( mEngine->renderWindow(),this);
        resetAllData();
    }

    void InputManager::shutdown()
    {
        releaseMouse();
        releaseKeyboard();
        Ogre::WindowEventUtilities::removeWindowEventListener(mEngine->renderWindow(),this);
        OIS::InputManager::destroyInputSystem(mOISInputManager);
        mOISInputManager = NULL;
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

    void InputManager::grabMouse()
    {
        bool bufferedMouse = true;
        
        mMouse = static_cast<OIS::Mouse*>(mOISInputManager->createInputObject(OIS::OISMouse,bufferedMouse));
        mMouse->setEventCallback(this);

        const OIS::MouseState &ms = mMouse->getMouseState();
        ms.width = mEngine->renderWindow()->getWidth();
        ms.height = mEngine->renderWindow()->getHeight();
        Debug::log("keeping mouse within ")(ms.width)(" and ")(ms.height).endl();
    }

    void InputManager::releaseMouse()
    {
        Debug::log("InputManager::releaseMouse()").endl();
        mOISInputManager->destroyInputObject(mMouse);
        mMouse=NULL;
    }

    void InputManager::grabKeyboard()
    {
        bool bufferedKeys = true;
        mKeyboard = static_cast<OIS::Keyboard*>(mOISInputManager->createInputObject(OIS::OISKeyboard,bufferedKeys));
        mKeyboard->setEventCallback(this);
        mKeysPressed.clear();
    }

    void InputManager::releaseKeyboard()
    {
        Debug::log("InputManager::releaseKeyboard()").endl();
        mOISInputManager->destroyInputObject(mKeyboard);
        mKeyboard=NULL;
    }

    bool InputManager::keyPressed(const OIS::KeyEvent& evt)
    {
//	cout << "InputManager::keyPressed()" << endl;
        mKeysPressed.push_back(evt.key);
        mKeysPressed.unique();
        mUI->keyPressed(evt);
        return true;
    }

    bool InputManager::keyReleased(const OIS::KeyEvent& evt)
    {
//	cout << "InputManager::keyReleased()" << endl;
        mKeysPressed.remove_if(equals(evt.key));
        mUI->keyReleased(evt);
        return true;
    }

    bool InputManager::mouseMoved(const OIS::MouseEvent& evt)
    {
        mHasMouseMoved = true;

        OIS::MouseState ms = evt.state;

        mMouseMove += Ogre::Vector2(ms.X.rel, ms.Y.rel);
        mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);
        
        mUI->mouseMoved(evt);
        return true;
    }

    bool InputManager::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        OIS::MouseState ms = evt.state;
        mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);
        
        mUI->mousePressed(evt,id);
        return true;
    }

    bool InputManager::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        OIS::MouseState ms = evt.state;
        mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);
        mUI->mouseReleased(evt,id);
        return true;
    }

    void InputManager::update()
    {
        // Pump window messages for nice behaviour
        Ogre::WindowEventUtilities::messagePump();

        if (mOISInputManager == NULL)
            return;

        if(mKeyboard)
            mKeyboard->capture();
        if(mMouse)
            mMouse->capture();

    }

    void InputManager::windowResized(Ogre::RenderWindow* rw)
    {
        Debug::log("InputManager::windowResized():").endl();

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
