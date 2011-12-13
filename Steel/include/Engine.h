/*
 * Engine.h
 *
 *  Created on: 2011-05-06
 *      Author: onze
 */

#ifndef ENGINE_H_
#define ENGINE_H_

#include <string>
#include <list>

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreLog.h>
#include <OGRE/OgreViewport.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreString.h>

#include "Camera.h"
#include "Level.h"
#include "RayCaster.h"

namespace Steel
{

class InputManager;

class Engine
{
public:
	Engine();
	virtual ~Engine();

	inline void abortMainLoop(void)
	{
		mMustAbortMainLoop = true;
	}
	;
	Level *createLevel(Ogre::String name);
	void deleteSelection();
	/**
	 * init from an app that already has created the engine's rendering window.
	 * Does not grab any input (this can be done with a call to grabInputs).
	 */
	void embeddedInit(	Ogre::String plugins,
						std::string windowHandle,
						int width,
						int height,
						Ogre::String defaultLog = Ogre::String("ogre_log.log"),
						Ogre::LogListener *logListener = NULL);
	void grabInputs(void);
	inline bool hasSelection()
	{
		return !mSelection.empty();
	}
	/**
	 * game-side/standalone init.
	 * Automaticaly grabs mouse/keyboard inputs.
	 */
	void init(	Ogre::String plugins,
				bool fullScreen = false,
				int width = 800,
				int height = 600,
				Ogre::String windowTitle = Ogre::String("Steel Window"),
				Ogre::LogListener *logListener = NULL);

	bool mainLoop(bool singleLoop = false);
	/**
	 * Takes window coordinates and lists under the given list all Agent's that collide with a ray going from the camera
	 * center to the given coordinates.
	 * see http://www.ogre3d.org/tikiwiki/Raycasting+to+the+polygon+level
	 */
	void pickAgents(std::list<ModelId> &selection, int x, int y);
	void redraw(void);
	void setSelectedAgents(std::list<AgentId> selection, bool selected);
	void shutdown(void);
	void releaseInputs(void);
	/**
	 * called to resize the window.
	 */
	void resizeWindow(int width, int height);
	void rotateSelection(Ogre::Vector3 rotation);
	/**
	 * Returns the mean of all positions of selected things.
	 */
	Ogre::Vector3 selectionPosition();
	void translateSelection(Ogre::Vector3 v);

	////////////////////////////////////////////////
	//getters
	inline Camera *camera(void)
	{
		return mCamera;
	}
	inline InputManager *inputMan(void)
	{
		return mInputMan;
	}
	inline bool isGrabbingInputs(void)
	{
		return mIsGrabbingInputs;
	}
	inline Ogre::RenderWindow *renderWindow(void)
	{
		return mRenderWindow;
	}
	inline Ogre::String rootdir(void)
	{
		return sRootdir;
	}
	inline std::list<AgentId> selection()
	{
		return mSelection;
	}
	inline std::string &windowHandle(void)
	{
		return mWindowHandle;
	}

	//setters
	void setRootdir(Ogre::String rootdir)
	{
		sRootdir = rootdir;
		Level::setPath(rootdir + "/levels/");
	}
	void setSelectionPosition(Ogre::Vector3 pos);
private:
	// <static>
	static Ogre::String sApplicationPath;
	// </static>
	Ogre::String sRootdir;
	bool preWindowingSetup(	Ogre::String &plugins,
							int width,
							int height,
							Ogre::String defaultLog =
									Ogre::String("steel_default_log.log"),
							Ogre::LogListener *logListener = NULL);

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

	/**
	 * current level.
	 */
	Level *mLevel;
	/**
	 * object that handles all this raycasting thingies.
	 */
	RayCaster *mRayCaster;
	std::list<AgentId> mSelection;
};

}

#include "InputManager.h"
#endif /* ENGINE_H_ */
