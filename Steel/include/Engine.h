/*
 * Engine.h
 *
 *  Created on: 2011-05-06
 *      Author: onze
 */

#ifndef ENGINE_H_
#define ENGINE_H_

#include <string>

#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreString.h>

#include "Camera.h"

namespace Steel
{

class InputManager;

class Engine
{
public:
	Engine();
	virtual ~Engine();
	/**
	 * called to resize the window.
	 */
	void resizeWindow(int width, int height);
	bool mainLoop(bool singleLoop = false);
	void redraw(void);
	/**
	 * game-side/standalone init.
	 * Automaticaly grabs mouse/keyboard inputs.
	 */
	void init(	Ogre::String plugins,
				bool fullScreen = false,
				int width = 800,
				int height = 600,
				Ogre::String windowTitle = Ogre::String("Steel Window"));

	/**
	 * init from an app that already has created the engine's rendering window.
	 * Does not grab any input (this can be done with a call to grabInputs).
	 */
	void embeddedInit(	Ogre::String plugins,
						std::string windowHandle,
						int width,
						int height,
						Ogre::String defaultLog = Ogre::String("ogre_log.log"));

	void grabInputs(void);
	void releaseInputs(void);
	//getters
	inline std::string &windowHandle()
	{
		return mWindowHandle;
	}
	;
	inline Ogre::RenderWindow *renderWindow()
	{
		return mRenderWindow;
	}
	;
	inline Camera *camera()
	{
		return mCamera;
	}
	;
	inline bool isGrabbingInputs()
	{
		return mIsGrabbingInputs;
	}
	;
	inline InputManager *inputMan(void)
	{
		return mInputMan;
	}
	;
	inline void abortMainLoop()
	{
		mMustAbortMainLoop = true;
	}
	;
private:
	bool preWindowingSetup(	Ogre::String &plugins,
							int width,
							int height,
							Ogre::String defaultLog = Ogre::String("ogre_log.log"));

	bool postWindowingSetup(int width, int height);

	bool processInputs(void);

	Ogre::Root *mRoot;
	Ogre::SceneManager *mSceneManager;
	Ogre::RenderWindow *mRenderWindow;
	Ogre::Viewport *mViewport;
	Camera *mCamera;

	InputManager *mInputMan;
	std::string mWindowHandle;
	bool mIsGrabbingInputs;
	bool mMustAbortMainLoop;
};

}

#include "InputManager.h"
#endif /* ENGINE_H_ */
