/*
 * Engine.cpp
 *
 *  Created on: 2011-05-06
 *      Author: onze
 */

#include <iostream>

#include <OGRE/OgreStringConverter.h>
#include <OGRE/OgreEntity.h>
#include <OGRE/OgreConfigFile.h>
#include <OGRE/OgreResourceGroupManager.h>

#include "Engine.h"
#include "OgreModelManager.h"
#include "Debug.h"

using namespace std;

namespace Steel
{

Engine::Engine() :
		mRootDir(""), mSceneManager(NULL), mRenderWindow(NULL), mViewport(NULL), mCamera(NULL), mInputMan(NULL), mIsGrabbingInputs(false), mMustAbortMainLoop(false), mLevel(NULL), mRayCaster(NULL)
{
	mRootDir = File::getCurrentDirectory();
	mSelection = std::list<AgentId>();
}

Engine::~Engine()
{
	//custom structs

	if (mLevel != NULL)
	{
		delete mLevel;
		mLevel = NULL;
	}
	if (mInputMan != NULL)
	{
		delete mInputMan;
		mInputMan = NULL;
	}

	//ogre structs
	if (mSceneManager != NULL)
	{
		delete mCamera;
		mCamera = NULL;
		mSceneManager->clearScene();
		mSceneManager->destroyAllCameras();
	}
	if (mRenderWindow != NULL)
	{
		delete mRenderWindow;
		mRenderWindow = NULL;
	}
	if (mRoot != NULL)
	{
		mRoot->destroySceneManager(mSceneManager);
	}
}

Level *Engine::createLevel(Ogre::String name)
{
	//single level only for now
	if (mLevel != NULL)
		delete mLevel;
	mLevel = new Level(mRootDir.subdir("levels"), name, mSceneManager, mCamera);

	return mLevel;
}

void Engine::deleteSelection()
{
	if (!hasSelection())
		return;
	for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
		mLevel->deleteAgent(*it);
}

void Engine::grabInputs()
{
	Debug::log("Engine::grabInputs()").endl();
	mInputMan->grab();
	mIsGrabbingInputs = true;
}

void Engine::init(	Ogre::String plugins,
					bool fullScreen,
					int width,
					int height,
					Ogre::String windowTitle,
					Ogre::LogListener *logListener)
{
	std::cout << "Engine::init()" << std::endl;
	preWindowingSetup(plugins, width, height);

	std::cout << "creating window, " << std::cout.flush();
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
							Ogre::String defaultLog,
							Ogre::LogListener *logListener)
{

	preWindowingSetup(plugins, width, height, defaultLog, logListener);

	//false so that no window is opened
	mRoot->initialise(false);

	Ogre::NameValuePairList viewConfig;
	viewConfig["parentWindowHandle"] = windowHandle.c_str();
	mWindowHandle = windowHandle;

	mRenderWindow = mRoot->createRenderWindow("Steel embedded window", width, height, false, &viewConfig);

	postWindowingSetup(width, height);

}
bool Engine::preWindowingSetup(	Ogre::String &plugins,
								int width,
								int height,
								Ogre::String defaultLog,
								Ogre::LogListener *logListener)
{
	std::cout << "Engine::preWindowingSetup()" << std::endl;
	Debug::init(defaultLog, logListener);
	Debug::log("Debug setup.").endl();
	mRoot = new Ogre::Root(plugins, "");

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
	assert( !renderers.empty());
	// we need at least one renderer to do anything useful

	Ogre::RenderSystem *renderSystem = renderers[0];
	assert( renderSystem);

	mRoot->setRenderSystem(renderSystem);

	return true;
}

bool Engine::postWindowingSetup(int width, int height)
{
	std::cout << "Engine::postWindowingSetup()" << std::endl;
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

	//	Ogre::Entity* ogreHead = mSceneManager->createEntity("HeadMesh", "ogrehead.mesh");
	//	Ogre::SceneNode* headNode = mSceneManager->getRootSceneNode()->createChildSceneNode("HeadNode",
	//																						Ogre::Vector3(0, 0, 0));
	//	headNode->attachObject(ogreHead);

	// Set ambient light
	mSceneManager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

	// Create a light
	Ogre::Light* l = mSceneManager->createLight("MainLight");
	l->setPosition(20, 80, 50);

	mRoot->clearEventTimes();
	mInputMan = new InputManager(this);

	mRayCaster = new RayCaster(mSceneManager);

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
	//see http://altdevblogaday.com/2011/02/23/ginkgos-game-loop/
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

void Engine::pickAgents(std::list<ModelId> &selection, int x, int y)
{
	std::cout << "Engine::pickAgents()" << endl;
	//get nodes that collide
	std::list<Ogre::SceneNode *> nodes;
	Ogre::Real _x = float(x) / float(mRenderWindow->getWidth());
	Ogre::Real _y = float(y) / float(mRenderWindow->getHeight());
	Ogre::Ray ray = mCamera->cam()->getCameraToViewportRay(_x, _y);
	if (!mRayCaster->fromRay(ray, nodes))
	{
		Debug::log("Engine::selectAgents(): raycast failed.").endl();
		return;
	}

	//retrieve Agent's that own them
	mLevel->getAgentsIdsFromSceneNodes(nodes, selection);
}

bool Engine::processInputs()
{
	//update inputs
	mInputMan->update();

	//process keyboard
	float dx = .0f, dy = .0f, dz = .0f, speed = .5f;
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

void Engine::redraw()
{
	mRoot->_fireFrameStarted();
	mRoot->_updateAllRenderTargets();
	mRenderWindow->update();
	mRoot->_fireFrameRenderingQueued();
	mRoot->_fireFrameEnded();
}

void Engine::releaseInputs()
{
	cout << "Engine::releaseInputs()" << endl;
	mInputMan->release();
	mIsGrabbingInputs = false;
}

void Engine::rotateSelection(Ogre::Vector3 rotation)
{
	//TODO:make selection child of a node on which the rotation is applied
	Agent *agent;
	for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
	{
		agent = mLevel->getAgent(*it);
		if (agent == NULL)
			continue;
		agent->ogreModel()->rotate(rotation);
	}
}

Ogre::Vector3 Engine::selectionPosition()
{
	if (!hasSelection())
		return Ogre::Vector3(.0f, .0f, .0f);
	Ogre::Vector3 pos = Ogre::Vector3::ZERO;
	Agent *agent;
	for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
	{
		agent = mLevel->getAgent(*it);
		if (agent == NULL)
			continue;
		pos += agent->ogreModel()->position();
	}
	return pos / Ogre::Real(mSelection.size());
}

void Engine::setRootDir(Ogre::String rootdir)
{
	setRootDir(File(rootdir));
}

void Engine::setRootDir(File rootdir)
{
	Debug::log("Steel::Engine: setting application root dir to ")(rootdir.fullPath()).endl();
	mRootDir = rootdir;
}

void Engine::setSelectedAgents(std::list<AgentId> selection, bool selected)
{
	Debug::log("Engine::setSelectedAgents(): ");
	//unselect last selection if any
	Agent *agent;
	for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
	{
		agent = mLevel->getAgent(*it);
		if (agent == NULL)
			continue;
		agent->ogreModel()->setSelected(false);
	}
	mSelection.clear();

	//process actual selections
	for (std::list<AgentId>::iterator it = selection.begin(); it != selection.end(); ++it)
	{
		agent = mLevel->getAgent(*it);
		Debug::log(*it)(", ");
		if (agent == NULL)
			continue;
		mSelection.push_back(agent->id());
		agent->setSelected(true);
	}
	Debug::log.endl();

}

void Engine::setSelectionPosition(Ogre::Vector3 pos)
{
	Agent *agent;
	for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
	{
		agent = mLevel->getAgent(*it);
		if (agent == NULL)
			continue;
		agent->ogreModel()->setPosition(pos);
	}
}

void Engine::shutdown()
{
	Debug::log("Engine::shutdown()").endl();
	if (mLevel != NULL)
	{
		delete mLevel;
		mLevel = NULL;
	}
}

void Engine::translateSelection(Ogre::Vector3 t)
{
	Agent *agent;
	for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
	{
		agent = mLevel->getAgent(*it);
		if (agent == NULL)
			continue;
		agent->ogreModel()->translate(t);
	}
}

}
