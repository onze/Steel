/*
 * InputManager.cpp
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */
#include <iostream>
#include "InputManager.h"
using namespace std;

#include <assert.h>

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

InputManager::InputManager(Steel::Engine *engine) :
		mEngine(engine), mInputManager(0), mMouse(0), mKeyboard(0), mKeysPressed(std::list<
				OIS::KeyCode>()), mHasMouseMoved(false), mMousePos(Ogre::Vector2(	-1.f,
																					-1.f))
{

}

InputManager::~InputManager()
{
	release();
}

void InputManager::resetFrameBasedData(void)
{
	mHasMouseMoved = false;
	mMouseMove = Ogre::Vector2::ZERO;
}

void InputManager::resetAllData(void)
{
	resetFrameBasedData();
	mKeysPressed.clear();
}

void InputManager::grab(void)
{
	if (!mInputManager)
	{
		cout << "InputManager::grab()" << endl;
		OIS::ParamList params;

		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		mEngine->renderWindow()->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		params.insert(std::make_pair(	std::string("WINDOW"),
										windowHndStr.str()));

		//		params.insert(std::make_pair(std::string("WINDOW"), mEngine->windowHandle()));
		mInputManager = OIS::InputManager::createInputSystem(params);

		bool bufferedKeys = true;
		mKeyboard =
				static_cast<OIS::Keyboard*>(mInputManager->createInputObject(	OIS::OISKeyboard,
																				bufferedKeys));
		mKeyboard->setEventCallback(this);

		bool bufferedMouse = true;
		mMouse =
				static_cast<OIS::Mouse*>(mInputManager->createInputObject(	OIS::OISMouse,
																			bufferedMouse));
		mMouse->setEventCallback(this);

		const OIS::MouseState &ms = mMouse->getMouseState();
		ms.width = mEngine->renderWindow()->getWidth();
		ms.height = mEngine->renderWindow()->getHeight();
		cout << "keeping mouse within " << ms.width << " and " << ms.height
				<< endl;

		mKeysPressed.clear();
		Ogre::WindowEventUtilities::addWindowEventListener(	mEngine->renderWindow(),
															this);
		resetAllData();
	}
}

bool InputManager::keyPressed(const OIS::KeyEvent& evt)
{
//	cout << "InputManager::keyPressed()" << endl;
	mKeysPressed.push_front(evt.key);
	mKeysPressed.unique();
	return true;
}

bool InputManager::keyReleased(const OIS::KeyEvent& evt)
{
//	cout << "InputManager::keyReleased()" << endl;
	mKeysPressed.remove_if(equals(evt.key));
	return true;
}

bool InputManager::mouseMoved(const OIS::MouseEvent& evt)
{
	mHasMouseMoved = true;

	OIS::MouseState ms = evt.state;

	mMouseMove += Ogre::Vector2(ms.X.rel, ms.Y.rel);
	mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);

	return true;
}

bool InputManager::mousePressed(const OIS::MouseEvent& evt,
								OIS::MouseButtonID id)
{
	OIS::MouseState ms = evt.state;
	mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);
	return true;
}

bool InputManager::mouseReleased(	const OIS::MouseEvent& evt,
									OIS::MouseButtonID id)
{
	OIS::MouseState ms = evt.state;
	mMousePos = Ogre::Vector2(ms.X.abs, ms.Y.abs);
	return true;
}

void InputManager::release(void)
{
	if (mInputManager)
	{
		cout << "InputManager::release()" << endl;
		mInputManager->destroyInputObject(mMouse);
		mInputManager->destroyInputObject(mKeyboard);

		OIS::InputManager::destroyInputSystem(mInputManager);
		mInputManager = NULL;
	}
}

void InputManager::update(void)
{
	// Pump window messages for nice behaviour
	Ogre::WindowEventUtilities::messagePump();

	if (mInputManager == NULL)
		return;

	mKeyboard->capture();
	mMouse->capture();

}

void InputManager::windowResized(Ogre::RenderWindow* rw)
{
	cout << "InputManager::windowResized():" << endl;

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
		Ogre::WindowEventUtilities::removeWindowEventListener(	mEngine->renderWindow(),
																this);
		windowClosed(rw);
		release();
	}
}
}
