/*
 * Engine.cpp
 *
 *  Created on: 2011-05-06
 *      Author: onze
 */

#include <iostream>

#include <OgreStringConverter.h>
#include <OgreEntity.h>
#include <OgreConfigFile.h>

#include "Engine.h"

using namespace std;

namespace Steel
{

Engine::Engine() :
	mSceneManager(0), mRenderWindow(0), mViewport(0), mCamera(0), mInputMan(0), mIsGrabbingInputs(false),
			mMustAbortMainLoop(false)
{
}

Engine::~Engine()
{
	delete mRoot;
}

void Engine::init(Ogre::String plugins, bool fullScreen, int width, int height, Ogre::String windowTitle)
{
	preWindowingSetup(plugins, width, height);

	//window setup
	mRenderWindow = mRoot->initialise(false);

	Ogre::NameValuePairList opts;
	opts["resolution"] = Ogre::StringConverter::toString(width) + "x" + Ogre::StringConverter::toString(height);
	opts["fullscreen"] = "false";
	opts["vsync"] = "false";
	mRenderWindow = mRoot->createRenderWindow(windowTitle, width, height, false, &opts);

	//borrowed to
	//http://www.ogre3d.org/tikiwiki/MadMarx+Tutorial+10&structure=Tutorials
	unsigned long windowHandle;
	mRenderWindow->getCustomAttribute("WINDOW", &windowHandle);
	mWindowHandle = Ogre::StringConverter::toString(windowHandle);

	postWindowingSetup(mRenderWindow->getWidth(), mRenderWindow->getHeight());
	grabInputs();
}

void Engine::embeddedInit(	Ogre::String plugins,
							std::string windowHandle,
							int width,
							int height,
							Ogre::String defaultLog)
{

	preWindowingSetup(plugins, width, height, defaultLog);

	//false so that no window is opened
	mRoot->initialise(false);

	Ogre::NameValuePairList viewConfig;
	viewConfig["parentWindowHandle"] = windowHandle.c_str();
	mWindowHandle = windowHandle;

	mRenderWindow = mRoot->createRenderWindow("Steel embedded window", width, height, false, &viewConfig);

	postWindowingSetup(width, height);

}
bool Engine::preWindowingSetup(Ogre::String &plugins, int width, int height, Ogre::String defaultLog)
{

	mRoot = new Ogre::Root(plugins, "", defaultLog);

	// setup resources
	// Load resource paths from config file
	Ogre::ConfigFile cf;
	cf.load("resources.cfg");

	// Go through all sections & settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
		}
	}

	// setup a renderer
	Ogre::RenderSystemList renderers = mRoot->getAvailableRenderers();
	assert( !renderers.empty() ); // we need at least one renderer to do anything useful

	Ogre::RenderSystem *renderSystem = renderers[0];
	assert( renderSystem );

	mRoot->setRenderSystem(renderSystem);

	return true;
}

bool Engine::postWindowingSetup(int width, int height)
{
	// Set default mipmap level (NB some APIs ignore this)
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	// initialise all resource groups
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC);

	// Create the camera
	mCamera = new Camera(mSceneManager);

	mCamera->cam()->setNearClipDistance(5);

	// Create one viewport, entire window
	mViewport = mRenderWindow->addViewport(mCamera->cam());
	mViewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));

	// Alter the camera aspect ratio to match the viewport
	mCamera->cam()->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));

	Ogre::Entity* ogreHead = mSceneManager->createEntity("HeadMesh", "ogrehead.mesh");
	Ogre::SceneNode* headNode = mSceneManager->getRootSceneNode()->createChildSceneNode("HeadNode",
																						Ogre::Vector3(0, 0, 0));
	headNode->attachObject(ogreHead);

	// Set ambient light
	mSceneManager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	// Create a light
	Ogre::Light* l = mSceneManager->createLight("MainLight");
	l->setPosition(20, 80, 50);

	mRoot->clearEventTimes();
	mInputMan = new InputManager(this);
	return true;
}

void Engine::resizeWindow(int width, int height)
{

	if (mRenderWindow)
	{
		mRenderWindow->resize(width, height);
		mRenderWindow->windowMovedOrResized();
		if (mCamera)
		{
			Ogre::Real aspectRatio = Ogre::Real(width) / Ogre::Real(height);
			mCamera->cam()->setAspectRatio(aspectRatio);
		}
	}
}

bool Engine::mainLoop(bool singleLoop)
{
	mMustAbortMainLoop = false;
	while (!mMustAbortMainLoop)
	{
		mRoot->_fireFrameStarted();
		if (!processInputs())
			return false;

		mRoot->_updateAllRenderTargets();
		mRenderWindow->update();
		mRoot->_fireFrameRenderingQueued();
		mRoot->_fireFrameEnded();

		if (singleLoop)
			break;
	}
	return true;
}

void Engine::redraw(void)
{
	mRoot->_fireFrameStarted();
	mRoot->_updateAllRenderTargets();
	mRenderWindow->update();
	mRoot->_fireFrameRenderingQueued();
	mRoot->_fireFrameEnded();
}

void Engine::grabInputs(void)
{
	cout << "Engine::grabInputs(void)" << endl;
	mInputMan->grab();
	mIsGrabbingInputs = true;
}

void Engine::releaseInputs(void)
{
	cout << "Engine::releaseInputs(void)" << endl;
	mInputMan->release();
	mIsGrabbingInputs = false;
}

bool Engine::processInputs(void)
{
	//update inputs
	mInputMan->update();

	//process keyboard
	float dx = .0f, dy = .0f, dz = .0f, speed = 2.f;
	for (list<OIS::KeyCode>::iterator it = mInputMan->keysPressed().begin(); it != mInputMan->keysPressed().end(); ++it)
	{
		switch (*it)
		{
			case OIS::KC_W:
				dz -= speed;
				break;
			case OIS::KC_A:
				dx -= speed;
				break;
			case OIS::KC_S:
				dz += speed;
				break;
			case OIS::KC_D:
				dx += speed;
				break;
			case OIS::KC_SPACE:
				dy += speed;
				break;
			case OIS::KC_LSHIFT:
				dy -= speed;
				break;
			case OIS::KC_ESCAPE:
				mInputMan->resetAllData();
				if (mIsGrabbingInputs)
					releaseInputs();
				return false;
				break;
			default:
				break;
		}
	}
	mCamera->translate(dx, dy, dz);

	//process mouse
	if (mInputMan->hasMouseMoved())
	{
		Ogre::Vector2 move = mInputMan->mouseMove();
		mCamera->lookTowards(-float(move.y), -float(move.x), .0f, .1f);
	}
	mInputMan->resetFrameBasedData();
	return true;

}

}
