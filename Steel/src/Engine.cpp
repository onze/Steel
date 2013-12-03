/*
 * Engine.cpp
 *
 *  Created on: 2011-05-06
 *      Author: onze
 */

#include <list>
#include <iostream>
#include <unistd.h>
#include <memory>

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
#include "tools/StringUtils.h"
#include "tools/OgreUtils.h"
#include "tests/utests.h"
#include "EngineEventListener.h"
#include "SelectionManager.h"
#include "SignalManager.h"

namespace Steel
{
    const Ogre::String Engine::NONEDIT_MODE_GRABS_INPUT = "Engine::nonEditModeGrabsInput";
    const Ogre::String Engine::COLORED_DEBUG = "Engine::coloredDebug";

    Engine::Engine(Ogre::String confFilename): InputEventListener(),
        mRootDir("."), mConfig(confFilename),
        mRoot(nullptr), mRenderWindow(nullptr), mInputMan(),
        mMustAbortMainLoop(false), mIsInMainLoop(false), mLevel(nullptr), mRayCaster(nullptr), mEditMode(false),
        mCommands()
    {
        setRootDir(File::getCurrentDirectory());
    }

    Engine::~Engine()
    {
        shutdown();
    }

    void Engine::addEngineEventListener(EngineEventListener *listener)
    {
        mListeners.insert(listener);
    }

    void Engine::removeEngineEventListener(EngineEventListener *listener)
    {
        mListeners.erase(listener);
    }

    Level *Engine::createLevel(Ogre::String levelName)
    {
        return new Level(this, dataDir().subfile("levels"), levelName);
    }

    Level *Engine::setCurrentLevel(Level *newLevel)
    {
        if(nullptr != mLevel && nullptr != newLevel && mLevel->name() == newLevel->name())
        {
            Debug::warning("Engine::setCurrentLevel(): ")(newLevel->name())(" is already set as current.").endl();
            return mLevel;
        }

        if(nullptr != mLevel)
        {
            fireOnLevelUnsetEvent();
            // TODO: tell the level it is unset, instead of doing stuff it knows better about
            mLevel->selectionMan()->clearSelection();
        }

        Level *previous = mLevel;
        mLevel = newLevel;

        loadConfig(mConfig);

        if(nullptr != newLevel)
            fireOnLevelSetEvent();

        return previous;
    }

    void Engine::init(Ogre::String plugins, bool fullScreen, unsigned int width, unsigned int height,
                      Ogre::String windowTitle, Ogre::LogListener *logListener)
    {
        std::cout << "Engine::init()" << std::endl;
        preWindowingSetup(plugins, width, height);

        std::cout << "creating window, " << std::cout.flush();
        //window setup
        mRoot->initialise(false);

        Ogre::NameValuePairList opts;
        opts["resolution"] = Ogre::StringConverter::toString((unsigned long) width) + "x"
                             + Ogre::StringConverter::toString((unsigned long) height);
        opts["fullscreen"] = "false";
        opts["vsync"] = "false";
        mRenderWindow = mRoot->createRenderWindow(windowTitle, width, height, false, &opts);

        //borrowed to
        //http://www.ogre3d.org/tikiwiki/MadMarx+Tutorial+10&structure=Tutorials
        unsigned long windowHandle;
        mRenderWindow->getCustomAttribute("WINDOW", &windowHandle);
        mWindowHandle = Ogre::StringConverter::toString(windowHandle);
        postWindowingSetup(mRenderWindow->getWidth(), mRenderWindow->getHeight());
        mInputMan.grabInput(true);
    }

    void Engine::embeddedInit(Ogre::String plugins, std::string windowHandle, unsigned int width, unsigned int height,
                              Ogre::String defaultLog, Ogre::LogListener *logListener)
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

    int Engine::preWindowingSetup(Ogre::String &plugins, unsigned int width, unsigned int height, Ogre::String defaultLog,
                                  Ogre::LogListener *logListener)
    {
        std::cout << "Engine::preWindowingSetup()" << std::endl;
        mConfig.load();

        Debug::init(defaultLog, logListener, mConfig.getSettingAsBool(Engine::COLORED_DEBUG, true));
        Debug::log("Debug setup.").endl();
        Debug::log("cwd: ")(mRootDir).endl();
        mRoot = new Ogre::Root(mRootDir.subfile(plugins).fullPath(), Ogre::StringUtil::BLANK);

        // setup resources
        // Load resource paths from config file
        Ogre::ConfigFile cf;
        cf.load("resources.cfg");


        // Go through all sections & settings in the file
        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

        Ogre::String secName, typeName, archName;

        while(seci.hasMoreElements())
        {
            secName = seci.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
            Ogre::ConfigFile::SettingsMultiMap::iterator i;

            for(i = settings->begin(); i != settings->end(); ++i)
            {
                typeName = i->first;
                archName = i->second;
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName, true);
            }
        }

        auto orm = Ogre::ResourceGroupManager::getSingletonPtr();
        File dir = dataDir();
        orm->addResourceLocation(dir.fullPath(), "FileSystem", "Steel", true);
        orm->addResourceLocation(dir.fullPath(), "FileSystem", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                                 true);
        dir = rawResourcesDir();
        orm->addResourceLocation(dir.subfile("meshes").fullPath(), "FileSystem", "Steel", true);
        orm->addResourceLocation(dir.subfile("textures").fullPath(), "FileSystem", "Steel", true);
        //TODO:add materials and models ?
        orm->initialiseResourceGroup("Steel");

        // setup a renderer
        Ogre::RenderSystemList renderers = mRoot->getAvailableRenderers();

        if(renderers.empty())
        {
            throw std::runtime_error("Could not find a valid renderer.");
        }

        // we need at least one renderer to do anything useful
        Ogre::RenderSystem *renderSystem = renderers[0];

        if(nullptr == renderSystem)
        {
            throw std::runtime_error("Could not find a valid renderer.");
        }

        mRoot->setRenderSystem(renderSystem);

        return 0;
    }

    int Engine::postWindowingSetup(unsigned int width, unsigned int height)
    {
        Debug::log("Engine::postWindowingSetup()").endl().indent();
        int init_code = File::init();

        if(init_code != 0)
        {
            Debug::error("Engine::postWindowingSetup(): error initializing File system.");
            return init_code;
        }

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);

        // Set default mipmap level (NB some APIs ignore this)
        Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
        // initialise all resource groups
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        // makes sure the window is usable (for instance for gui init) once out of init.
        mRenderWindow->update();
        mRoot->clearEventTimes();

        mInputMan.init(this);
        mInputMan.addInputEventListener(this);

        mRayCaster = new RayCaster(this);
        mUI.init(mRenderWindow->getWidth(), mRenderWindow->getHeight(), uiDir(), &mInputMan, mRenderWindow, this);

        setCurrentLevel(createLevel("DefaultLevel"));


        Debug::log.unIndent();

        // engine ready.
        // unit testing
        if(Ogre::StringConverter::parseBool(mConfig.getSetting("Engine::utests"), false))
        {
            Debug::log("Starting unit tests...").endl();
            bool abortOnFail = Ogre::StringConverter::parseBool(mConfig.getSetting("Engine::utests_abort_on_fail"), true);
            bool all_passed = startTests(this, abortOnFail);

            if(!all_passed)
            {
                if(abortOnFail)
                {
                    Debug::error("Aborting.");
                    Debug::log(" (Set Engine::utests_abort_on_fail to true in the config to ignore unit tests results)").endl();
                    assert(false);
                }
            }
        }

        return 0;
    }

    void Engine::shutdown()
    {
        if(mIsInMainLoop && !mMustAbortMainLoop)
        {
            mMustAbortMainLoop = true;
            return;
        }

        Debug::log("Engine::shutdown()...").endl();

        if(nullptr != mRayCaster)
        {
            delete mRayCaster;
            mRayCaster = nullptr;
        }

        mUI.shutdown();
        mInputMan.shutdown();

        if(mLevel != nullptr)
        {
            delete mLevel;
            mLevel = nullptr;
        }

        if(mRenderWindow != nullptr)
        {
            delete mRenderWindow;
            mRenderWindow = nullptr;
        }

        File::shutdown();

        if(nullptr != mRenderWindow)
        {
            Ogre::Root::getSingletonPtr()->destroyRenderTarget(mRenderWindow);
            mRenderWindow = nullptr;
        }

        Ogre::Root::getSingletonPtr()->shutdown();
        Debug::log("Steel: done").endl();
    }

    void Engine::fireOnLevelSetEvent()
    {
        for(auto listener : std::list<EngineEventListener *>(mListeners.begin(), mListeners.end()))
            listener->onLevelSet(mLevel);
    }

    void Engine::fireOnLevelUnsetEvent()
    {
        for(auto listener : std::list<EngineEventListener *>(mListeners.begin(), mListeners.end()))
            listener->onLevelUnset(mLevel);
    }

    void Engine::fireOnBeforeLevelUpdate(float dt)
    {
        for(auto listener : std::list<EngineEventListener *>(mListeners.begin(), mListeners.end()))
            listener->onBeforeLevelUpdate(mLevel, dt);
    }
    
    void Engine::fireOnAfterLevelUpdate()
    {
        for(auto listener : std::list<EngineEventListener *>(mListeners.begin(), mListeners.end()))
            listener->onAfterLevelUpdate(mLevel);
    }

    bool Engine::mainLoop(bool singleLoop)
    {
        mIsInMainLoop = true;
        //see http://altdevblogaday.com/2011/02/23/ginkgos-game-loop/
        mMustAbortMainLoop = false;

        const double ms2us = 1000.;
        Ogre::Timer timer;
        long unsigned frameStart, graphicsStart, engineStart;
        frameStart = graphicsStart = engineStart = timer.getMilliseconds();

        while(!mMustAbortMainLoop)
        {
            processAllCommands();

            frameStart = engineStart;
            mMustAbortMainLoop = !mRoot->_fireFrameStarted();

            if(!processInputs())
            {
                mIsInMainLoop = false;
                mMustAbortMainLoop = true;
                return false;
            }

            // update file watching
            File::dispatchEvents();

            if(nullptr != mLevel)
            {
                float dt = float(timer.getMilliseconds() - graphicsStart) / 1000.f;
                fireOnBeforeLevelUpdate(dt);
                mLevel->update(dt);
                fireOnAfterLevelUpdate();
            }
            else
                SignalManager::instance().fireEmittedSignals();

            graphicsStart = timer.getMilliseconds();
            mStats.lastEngineDuration = static_cast<double>(graphicsStart - engineStart);

            // render
            mRoot->_updateAllRenderTargets();
            mRenderWindow->update();
            mRoot->_fireFrameRenderingQueued();

            if(!mRoot->_fireFrameEnded())
                break;

            engineStart = timer.getMilliseconds();
            mStats.lastGraphicFrameDuration = static_cast<double>(engineStart - graphicsStart);
            mStats.lastFullFrameDuration = static_cast<double>(engineStart - frameStart);

            double dt = 1000. / 30. - mStats.lastFullFrameDuration;

            if(dt > 0)
                usleep(static_cast<useconds_t>(dt * ms2us));

            if(singleLoop)
                break;
        }

        mIsInMainLoop = false;
        return true;
    }

    bool Engine::keyReleased(Input::Code key, Input::Event const &evt)
    {
        if(mEditMode)
        {
            //EDITOR MODE
            switch(key)
            {
                case Input::Code::KC_GRAVE:
                    stopEditMode();
                    break;

                default:
                    break;
            }
        }
        else
        {
            //GAMING MODE
            switch(key)
            {
                case Input::Code::KC_GRAVE:
                    startEditMode();
                    break;

                default:
                    break;
            }
        }

        return true;
    }

    void Engine::pickAgents(Selection &selection, int x, int y)
    {
        //get nodes that collide
        std::list<Ogre::SceneNode *> nodes;
        Ogre::Real _x = float(x) / float(mRenderWindow->getWidth());
        Ogre::Real _y = float(y) / float(mRenderWindow->getHeight());
        Ogre::Ray ray = mLevel->camera()->cam()->getCameraToViewportRay(_x, _y);

        if(!mRayCaster->fromRay(ray, nodes))
        {
            return;
        }

        //retrieve Agent's that own them
        mLevel->getAgentsIdsFromSceneNodes(nodes, selection);
    }

    bool Engine::processInputs()
    {
        if(mMustAbortMainLoop)
            return false;

        //update inputs
        mInputMan.update();

        //process keyboard
        bool moveCam = false;
        float dx = .0f, dy = .0f, dz = .0f, speed = .5f;

        for(auto const & code : mInputMan.codesPressed())
        {
            if(mEditMode)
            {
                // ONLY IN edit mode
                switch(code)
                {
                    default:
                        break;
                }
            }
            else
            {
                if(mInputMan.isKeyDown(Input::Code::KC_LCONTROL))
                    speed *= 2.f;

                // ONLY NOT IN edit mode
                switch(code)
                {
                    case Input::Code::KC_W:
                        dz -= speed;
                        moveCam = true;
                        break;

                    case Input::Code::KC_A:
                        dx -= speed;
                        moveCam = true;
                        break;

                    case Input::Code::KC_S:
                        dz += speed;
                        moveCam = true;
                        break;

                    case Input::Code::KC_D:
                        dx += speed;
                        moveCam = true;
                        break;

                    case Input::Code::KC_SPACE:
                        dy += speed;
                        moveCam = true;
                        break;

                    case Input::Code::KC_LSHIFT:
                        dy -= speed;
                        moveCam = true;
                        break;

                    default:
                        break;
                }
            }

            // ALL THE TIME
            switch(code)
            {
                case Input::Code::KC_ESCAPE:
                    return false;
                    break;

                default:
                    break;
            }
        }

        if(moveCam)
            mLevel->camera()->translate(dx, dy, dz, speed);

        //process mouse
        if(mInputMan.hasMouseMoved())
        {
            Ogre::Vector2 move = mInputMan.mouseMove();

            if(!mEditMode)
            {
                mLevel->camera()->lookTowards(-float(move.x), -float(move.y), .0f, .1f);
//                 Debug::log("cam pos: ")(mCamera->camNode()->getPosition())(" rot:")(mCamera->camNode()->getOrientation()).endl();
            }
        }

        mInputMan.resetFrameBasedData();
        return true;

    }

    bool Engine::processCommand(std::vector<Ogre::String> command)
    {
        if(command[0] == "level")
        {
            command.erase(command.begin());
            mLevel->processCommand(command);
        }
        else if(command[0] == "ui")
        {
            command.erase(command.begin());
            return mUI.processCommand(command);
        }
        else if(command[0] == "editor")
        {
            command.erase(command.begin());
            return mUI.editor().processCommand(command);
        }
        else if(command[0] == "reloadConfig")
        {
            loadConfig(mConfig);
        }
        else if(command[0] == "set_level")
        {
            if(command.size() > 1)
            {
                auto newName = StringUtils::join(command, ".", 1);

                if(newName != mLevel->name())
                {
                    // params should be valid now
                    Level *level = createLevel(newName);
                    Level *prev = setCurrentLevel(level);

                    if(level->getSavefile().exists())
                        level->load();

                    delete prev;
                }
                else
                {
                    Debug::warning("Engine::processCommand(): a new level requires a new name. Command was:");
                    Debug::warning(StringUtils::join(command, ".")).endl();
                    return false;
                }
            }
            else
            {
                Debug::warning("Engine::processCommand(): invalid command (missing level name ?):");
                Debug::warning(StringUtils::join(command, ".")).endl();
                return false;
            }
        }
        else if(command[0] == "register")
        {
            command.erase(command.begin());
            registerCommand(command);
        }
        else
        {
            Debug::warning("Engine::processCommand(): unknown command ");
            Debug::warning(StringUtils::join(command, ".")).endl();
            return false;
        }

        return true;
    }

    void Engine::processAllCommands()
    {
        while(!mCommands.empty())
        {
            std::vector<Ogre::String> command = mCommands.front();
            mCommands.pop_front();

            if(!processCommand(command))
                break;
        }
    }

    void Engine::registerCommand(Ogre::String rawCommand)
    {
        mCommands.push_back(StringUtils::split(std::string(rawCommand), std::string(".")));
    }

    void Engine::registerCommand(std::vector<Ogre::String> command)
    {
        mCommands.push_back(command);
    }

    void Engine::saveConfig(ConfigFile &config)
    {
        mUI.saveConfig(config);
        config.save();
    }

    void Engine::loadConfig(ConfigFile &config)
    {
        config.load();
        mUI.loadConfig(config);
    }

    void Engine::redraw()
    {
        mRoot->_fireFrameStarted();
        mRoot->_updateAllRenderTargets();
        mRenderWindow->update();
        mRoot->_fireFrameRenderingQueued();
        mRoot->_fireFrameEnded();
    }

    void Engine::resizeWindow(int width, int height)
    {

        if(mRenderWindow)
        {
            mRenderWindow->resize(width, height);
            mRenderWindow->windowMovedOrResized();

            if(mLevel->camera())
            {
                Ogre::Real aspectRatio = Ogre::Real(width) / Ogre::Real(height);
                mLevel->camera()->cam()->setAspectRatio(aspectRatio);
            }
        }
    }

    void Engine::setRootDir(Ogre::String rootdir)
    {
        setRootDir(File(rootdir));
    }

    void Engine::setRootDir(File rootdir)
    {
        std::string s = "Steel::Engine: setting application root dir to ";

        if(Debug::isInit)
            Debug::log(s)(rootdir.fullPath()).endl();
        else
            std::cout << (s + rootdir.fullPath()) << std::endl;

        mRootDir = rootdir;
        mConfig = mRootDir.subfile(mConfig.file().fileName());
    }

    void Engine::startEditMode()
    {
//         Debug::log("Engine::startEditMode()").endl();
        mEditMode = true;
        mInputMan.grabInput(false);
        mUI.startEditMode();
    }

    void Engine::stopEditMode()
    {
//         Debug::log("Engine::stopEditMode()").endl();
        mEditMode = false;
        mUI.stopEditMode();
        mInputMan.grabInput(mConfig.getSettingAsBool(Engine::NONEDIT_MODE_GRABS_INPUT, true));
    }

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
