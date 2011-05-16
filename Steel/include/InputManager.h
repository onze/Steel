/*
 * InputManager.h
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

#include <string>
#include <list>

#include <OgreWindowEventUtilities.h>
#include <OIS/OIS.h>

#include "Engine.h"

namespace Steel
{

class InputManager: public Ogre::WindowEventListener, OIS::MouseListener, OIS::KeyListener
{
private:
	InputManager();

public:

	InputManager(Engine *);
	virtual ~InputManager();

	virtual bool keyPressed(const OIS::KeyEvent& evt);
	virtual bool keyReleased(const OIS::KeyEvent& evt);
	virtual bool mouseMoved(const OIS::MouseEvent& evt);
	virtual bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
	virtual bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);

	void grab(void);
	void release(void);

	void update(void);
	void close(void);
	void resetFrameBasedData(void);
	void resetAllData(void);

	/**
	 * called by OIS when the window has been resized.
	 */
	virtual void windowResized(Ogre::RenderWindow* rw);
	virtual void windowClosed(Ogre::RenderWindow* rw);

	//getters
	inline std::list<OIS::KeyCode> &keysPressed(void){return mKeysPressed;};
	inline bool hasMouseMoved(){return mHasMouseMoved;};
	inline Ogre::Vector2 &mouseMove(){return mMouseMove;};
	inline OIS::Mouse* mouse(void){return mMouse;};
	inline OIS::Keyboard* keyboard(void){return mKeyboard;};

protected:
	Steel::Engine *mEngine;
	OIS::InputManager* mInputManager;
	OIS::Mouse* mMouse;
	OIS::Keyboard* mKeyboard;

	std::list<OIS::KeyCode> mKeysPressed;
	bool mHasMouseMoved;
	/**
	 * move since last known position, and last known position.
	 */
	Ogre::Vector2 mMouseMove,mMousePos;
};

}

#endif /* INPUTMANAGER_H_ */
