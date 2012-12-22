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
#include <OgreResourceGroupManager.h>

#include "Engine.h"
#include "Camera.h"
#include "Level.h"
#include "Agent.h"
#include "OgreModelManager.h"
#include "Debug.h"
#include "RayCaster.h"
#include "tools/File.h"
#include "tests/utests.h"

using namespace std;

namespace Steel
{

    Engine::Engine() :
        mRootDir(""), mSceneManager(NULL), mRenderWindow(NULL), mViewport(NULL),
        mCamera(NULL), mInputMan(), mMustAbortMainLoop(false),
        mLevel(NULL), mRayCaster(NULL),mEditMode(false)
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

    Level *Engine::createLevel(Ogre::String levelName)
    {
        //single level only
        if (mLevel != NULL)
            delete mLevel;
        mLevel = new Level(mRootDir.subfile("data").subfile("levels"), levelName, mSceneManager, mCamera);

        return mLevel;
    }

    void Engine::clearSelection()
    {
        Agent *agent;
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->setSelected(false);
        }
        mSelection.clear();
    }

    void Engine::deleteSelection()
    {
        if (!hasSelection())
            return;
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
            mLevel->deleteAgent(*it);
    }

    void Engine::init(Ogre::String plugins,
                      bool fullScreen,
                      unsigned int width,
                      unsigned int height,
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
    }

    void Engine::embeddedInit(Ogre::String plugins,
                              std::string windowHandle,
                              unsigned int width,
                              unsigned int height,
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
    int Engine::preWindowingSetup(Ogre::String &plugins,
                                  unsigned int width,
                                  unsigned int height,
                                  Ogre::String defaultLog,
                                  Ogre::LogListener *logListener)
    {
        std::cout << "Engine::preWindowingSetup()" << std::endl;
        Debug::init(defaultLog, logListener);
        Debug::log("Debug setup.").endl();
        Debug::log("cwd: ")(mRootDir).endl();
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
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName,true);
            }
        }

        Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Steel");

        // setup a renderer
        Ogre::RenderSystemList renderers = mRoot->getAvailableRenderers();
        assert( !renderers.empty());
        // we need at least one renderer to do anything useful

        Ogre::RenderSystem *renderSystem = renderers[0];
        assert(renderSystem);

        mRoot->setRenderSystem(renderSystem);

        return 0;
    }

    int Engine::postWindowingSetup(unsigned int width, unsigned int height)
    {
        Debug::log("Engine::postWindowingSetup()").endl().indent();
        int init_code=File::init();
        if(init_code!=0)
        {
            Debug::error("Engine::postWindowingSetup(): error initializing File system.");
            return init_code;
        }

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
        Ogre::Real aspectRatio=Ogre::Real(mViewport->getActualWidth())
                               / Ogre::Real(mViewport->getActualHeight());
        mCamera->cam()->setAspectRatio(aspectRatio);

        //	Ogre::Entity* ogreHead = mSceneManager->createEntity("HeadMesh", "ogrehead.mesh");
        //	Ogre::SceneNode* headNode = mSceneManager->getRootSceneNode()->createChildSceneNode("HeadNode",Ogre::Vector3(0, 0, 0));
        //	headNode->attachObject(ogreHead);

        // Set ambient light
        mSceneManager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

        // Create a light
        Ogre::Light* l = mSceneManager->createLight("MainLight");
        l->setPosition(20, 80, 50);

        mRayCaster = new RayCaster(mSceneManager);

        mRoot->clearEventTimes();

        mInputMan.init(this,&mUI);

        // makes sure the window is usable (for instance for gui init) once out of init.
        mRenderWindow->update();

        mUI.init(width,height,mRootDir.subfile("data/ui"),&mInputMan,mSceneManager,mRenderWindow,this);
        Debug::log.unIndent();
        // unit testing
        if(true)
        {
            Debug::log("Starting unit tests...").endl();
            start_tests();
            Debug::log("unit tests done.").endl();
        }
        mInputMan.grabInput(true);
        return 0;
    }

    void Engine::shutdown()
    {
        if(!mMustAbortMainLoop)
        {
            mMustAbortMainLoop=true;
            return;
        }
        Debug::log("Engine::shutdown()").endl();
        if (mLevel != NULL)
        {
            delete mLevel;
            mLevel = NULL;
        }
        mUI.shutdown();
        mInputMan.shutdown();
        Rocket::Core::Shutdown();
        File::shutdown();
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


        Ogre::Timer timer;
        while (!mMustAbortMainLoop)
        {
            mRoot->_fireFrameStarted();

            // escape is a builting show stopper
            if (!processInputs())
                return false;

            // update file watching
            File::dispatchToFiles();

            mRoot->_updateAllRenderTargets();
            mRenderWindow->update();
            mRoot->_fireFrameRenderingQueued();
            if(!mRoot->_fireFrameEnded())
                break;

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

    bool Engine::keyPressed(const OIS::KeyEvent& evt)
    {
        return true;
    }

    bool Engine::keyReleased(const OIS::KeyEvent& evt)
    {

        if(mEditMode)
        {
            //EDITOR MODE
            switch(evt.key)
            {
                case OIS::KC_GRAVE:
                    stopEditMode();
                    break;
                case OIS::KC_R:
                    mUI.editor().reloadContent();
                    break;
                case OIS::KC_S:
                    if(mInputMan.isKeyDown(OIS::KC_LCONTROL))
                        if(mLevel!=NULL)
                            mLevel->save();
                default:
//                     Debug::log("Engine::keyReleased: ")(evt.key).endl();
                    break;
            }
        }
        else
        {
            //GAMING MODE
            switch(evt.key)
            {
                case OIS::KC_GRAVE:
                    startEditMode();
                    break;
                case OIS::KC_R:
                case OIS::KC_S:
                default:
//                     Debug::log("Engine::keyReleased: ")(evt.key).endl();
                    break;
            }
        }
        return true;
    }

    bool Engine::mouseMoved(const OIS::MouseEvent& evt)
    {
        return true;
    }

    bool Engine::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        return true;
    }

    bool Engine::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        return true;
    }

    bool Engine::processInputs()
    {
        if(mMustAbortMainLoop)
            return false;
        //update inputs
        mInputMan.update();

        //process keyboard
        float dx = .0f, dy = .0f, dz = .0f, speed = .5f;
        for (list<OIS::KeyCode>::iterator it = mInputMan.keysPressed().begin(); it != mInputMan.keysPressed().end(); ++it)
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
                    return false;
                    break;
                default:
                    break;
            }
        }
        mCamera->translate(dx, dy, dz);

        //process mouse
        if (mInputMan.hasMouseMoved())
        {
            Ogre::Vector2 move = mInputMan.mouseMove();
            mCamera->lookTowards(-float(move.y), -float(move.x), .0f, .1f);
        }
        mInputMan.resetFrameBasedData();
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
        std::string s="Steel::Engine: setting application root dir to ";
        if(Debug::isInit)
            Debug::log(s)(rootdir.fullPath()).endl();
        else
            std::cout<<(s+rootdir.fullPath())<<std::endl;
        mRootDir = rootdir;
    }

    void Engine::setSelectedAgents(std::list<AgentId> selection, bool selected)
    {
        Debug::log("Engine::setSelectedAgents(): ");
        //unselect last selection if any
        if (hasSelection())
            clearSelection();
        Agent *agent;
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

    void Engine::startEditMode()
    {
        Debug::log("Engine::startEditMode()").endl();
        mEditMode=true;
        mUI.startEditMode();
        mInputMan.grabInput(false);
    }

    void Engine::stopEditMode()
    {
        Debug::log("Engine::stopEditMode()").endl();
        mEditMode=false;
        mUI.stopEditMode();
        mInputMan.grabInput(true);
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




// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
