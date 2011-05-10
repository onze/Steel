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
	mSceneManager(0), mRenderWindow(0), mViewport(0), mCamera(0)
{
	cout << "Engine::Engine()" << endl;
}

Engine::~Engine()
{

}

void Engine::init(Ogre::String plugins, bool fullScreen, int width, int height, Ogre::String windowTitle)
{
	preWindowingSetup(plugins, width, height);
	mRenderWindow = mRoot->initialise(true, windowTitle);
	postWindowingSetup();

}

void Engine::embeddedInit(	Ogre::String plugins,
    	                  	std::string windowHandle,
							int width,
							int height)
{

	preWindowingSetup(plugins, width, height);

	//false so that no window is opened
	mRoot->initialise(false);

	Ogre::NameValuePairList viewConfig;
	viewConfig["parentWindowHandle"] = windowHandle.c_str();

	mRenderWindow = mRoot->createRenderWindow("Steel embedded window", width, height, false, &viewConfig);

	postWindowingSetup();

}
bool Engine::preWindowingSetup(Ogre::String &plugins, int width, int height)
{

	mRoot = new Ogre::Root(plugins);

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
	mRoot->getRenderSystem()->setConfigOption("Full Screen", "No");
	renderSystem->setConfigOption(	"Video Mode",
									Ogre::StringConverter::toString(width) + "x"
											+ Ogre::StringConverter::toString(height));

	return true;
}

bool Engine::postWindowingSetup()
{
	// Set default mipmap level (NB some APIs ignore this)
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	// initialise all resource groups
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

	mSceneManager = mRoot->createSceneManager("DefaultSceneManager");

	// Create the camera
	mCamera = mSceneManager->createCamera("PlayerCam");

	// Position it at 500 in Z direction
	mCamera->setPosition(Ogre::Vector3(0, 0, 80));
	// Look back along -Z
	mCamera->lookAt(Ogre::Vector3(0, 0, -300));
	mCamera->setNearClipDistance(5);

	// Create one viewport, entire window
	mViewport = mRenderWindow->addViewport(mCamera);
	mViewport->setBackgroundColour(Ogre::ColourValue(0, 0, 0));

	// Alter the camera aspect ratio to match the viewport
	mCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));

	Ogre::Entity* ogreHead = mSceneManager->createEntity("Head", "ogrehead.mesh");

	Ogre::SceneNode* headNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
	headNode->attachObject(ogreHead);

	// Set ambient light
	mSceneManager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	// Create a light
	Ogre::Light* l = mSceneManager->createLight("MainLight");
	l->setPosition(20, 80, 50);
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
			mCamera->setAspectRatio(aspectRatio);
		}
	}
}

bool Engine::update(void)
{
	mRoot->_fireFrameRenderingQueued();

	mRenderWindow->update();
	mRoot->_updateAllRenderTargets();
	mRoot->_fireFrameEnded();

	return true;
}


}
