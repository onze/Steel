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

namespace Steel
{

class Engine
{
public:
	Engine();
	virtual ~Engine();
	void resizeWindow(int width, int height);
	bool update(void);
	/**
	 * game-side/standalone init.
	 */
	void init(	Ogre::String plugins,
				bool fullScreen = false,
				int width = 800,
				int height = 600,
				Ogre::String windowTitle = Ogre::String("Steel Window"));
	/**
	 *
	 */
	void embeddedInit(	Ogre::String plugins,
	                  	std::string windowHandle,
						int width,
						int height);

	inline Ogre::RenderWindow *renderWindow(){return mRenderWindow;};
private:
	bool preWindowingSetup(Ogre::String &plugins, int width, int height);
	bool postWindowingSetup();
	Ogre::Root *mRoot;
	Ogre::SceneManager *mSceneManager;
	Ogre::RenderWindow *mRenderWindow;
	Ogre::Viewport *mViewport;
	Ogre::Camera *mCamera;
};

}

#endif /* ENGINE_H_ */
