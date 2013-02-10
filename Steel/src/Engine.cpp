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
#include <tools/StringUtils.h>
#include <tools/OgreUtils.h>
#include "tests/utests.h"
#include <EngineEventListener.h>

using namespace std;

namespace Steel
{

    Engine::Engine() :
        mRootDir(""), mRenderWindow(NULL), mInputMan(), mMustAbortMainLoop(false),
        mLevel(NULL), mRayCaster(NULL),mSelection(std::list<AgentId>()),mEditMode(false),
        mCommands(std::list<std::vector<Ogre::String> >())
    {
        mRootDir = File::getCurrentDirectory();
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
        auto it=mListeners.find(listener);
        if(it!=mListeners.end())
            mListeners.erase(it);
    }

    Level *Engine::createLevel(Ogre::String levelName)
    {
        //      Ogre::Entity* ogreHead = mSceneManager->createEntity("HeadMesh", "ogrehead.mesh");
        //      Ogre::SceneNode* headNode = mSceneManager->getRootSceneNode()->createChildSceneNode("HeadNode",Ogre::Vector3(0, 0, 0));
        //      headNode->attachObject(ogreHead);

        // Create a light
        //         Ogre::Light* l = mSceneManager->createLight("MainLight");
        //         l->setPosition(20, 80, 50);

        return new Level(this,mRootDir.subfile("data").subfile("levels"), levelName);
    }

    Level *Engine::setCurrentLevel(Level *newLevel)
    {
        if(NULL!=mLevel && NULL!=newLevel && mLevel->name()==newLevel->name())
        {
            Debug::warning("Engine::setCurrentLevel(): ")(newLevel->name())(" is already set as current.").endl();
            return mLevel;
        }
        if(NULL!=mLevel)
            fireOnLevelUnsetEvent();

        Level *previous=mLevel;
        mLevel=newLevel;

        if(NULL!=newLevel)
            fireOnLevelSetEvent();
        return previous;
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
        mRoot->initialise(false);

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
        mInputMan.grabInput(true);
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
        auto orm=Ogre::ResourceGroupManager::getSingletonPtr();
        File dir=mRootDir.subfile("data");
        orm->addResourceLocation(dir.fullPath(), "FileSystem", "Steel",true);
        orm->addResourceLocation(dir.fullPath(), "FileSystem", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,true);
        dir=dir.subfile("raw_resources");
        orm->addResourceLocation(dir.subfile("meshes").fullPath(), "FileSystem","Steel",true);
        orm->addResourceLocation(dir.subfile("textures").fullPath(), "FileSystem","Steel",true);
        //TODO:add materials and models ?
        orm->initialiseResourceGroup("Steel");

        // setup a renderer
        Ogre::RenderSystemList renderers = mRoot->getAvailableRenderers();
        assert(!renderers.empty());
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

        Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
        Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);

        // Set default mipmap level (NB some APIs ignore this)
        Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
//         Ogre::ResourceGroupManager::getSingleton().addResourceLocation(mRootDir.subfile("data").subfile("resources").subfile("meshes"),"FileSystem", "Steel",true);
        // initialise all resource groups
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        // makes sure the window is usable (for instance for gui init) once out of init.
        mRenderWindow->update();
        mRoot->clearEventTimes();

        mRayCaster = new RayCaster(this);
        mUI.init(mRenderWindow->getWidth(),
                 mRenderWindow->getHeight(),
                 mRootDir.subfile("data/ui"),
                 &mInputMan,
                 mRenderWindow,
                 this);

        setCurrentLevel(createLevel("DefaultLevel"));

        mInputMan.init(this,&mUI);

        Debug::log.unIndent();
        // unit testing
        if(true)
        {
            Debug::log("Starting unit tests...").endl();
            start_tests();
            Debug::log("unit tests done.").endl();
        }
        return 0;
    }

    void Engine::shutdown()
    {
        if(!mMustAbortMainLoop)
        {
            mMustAbortMainLoop=true;
            return;
        }
        Debug::log("Engine::shutdown()...").endl();
        mUI.shutdown();
        mInputMan.shutdown();
        if (mLevel != NULL)
        {
            delete mLevel;
            mLevel = NULL;
        }
        if (mRenderWindow != NULL)
        {
            delete mRenderWindow;
            mRenderWindow = NULL;
        }
        File::shutdown();
        if(NULL!=mRenderWindow)
        {
            Ogre::Root::getSingletonPtr()->destroyRenderTarget(mRenderWindow);
            mRenderWindow=NULL;
        }
        Ogre::Root::getSingletonPtr()->shutdown();
        Debug::log("Steel: done").endl();
    }

    void Engine::fireOnLevelSetEvent()
    {
        std::vector<EngineEventListener *>listeners(mListeners.begin(),mListeners.end());
        for(auto it=listeners.begin(); it!=listeners.end(); ++it)
            (*it)->onLevelSet(mLevel);
    }

    void Engine::fireOnLevelUnsetEvent()
    {
        std::vector<EngineEventListener *>listeners(mListeners.begin(),mListeners.end());
        for(auto it=listeners.begin(); it!=listeners.end(); ++it)
            (*it)->onLevelUnset(mLevel);
    }

    bool Engine::mainLoop(bool singleLoop)
    {

        //see http://altdevblogaday.com/2011/02/23/ginkgos-game-loop/
        mMustAbortMainLoop = false;


        //debug
//         mUI.editor().processCommand("engine.level.load.MyLevel");
//         mUI.editor().processCommand("editorbrush.mode.terraform");
//         mUI.editor().processCommand("engine.level.load.PG01-dev");
//         mUI.editor().processCommand("editorbrush.mode.terraform");
//         mUI.editor().processCommand("engine.level.instanciate./media/a0/cpp/1210/usmb/install_dir/data/models/Ogre/seaweed.model");


        const double ms2us=1000.;

        Ogre::Timer timer;
        long unsigned frameStart,graphicsStart,engineStart;
        frameStart=graphicsStart=engineStart=timer.getMilliseconds();

        while (!mMustAbortMainLoop)
        {
            processAllCommands();

            frameStart= engineStart;
            mMustAbortMainLoop=!mRoot->_fireFrameStarted();

            if (!processInputs())
                return false;
            // update file watching
            File::dispatchEvents();

            if(NULL!=mLevel)
                mLevel->update(float(timer.getMilliseconds()-graphicsStart)/1000.f);

            graphicsStart=timer.getMilliseconds();
            mStats.lastEngineDuration=static_cast<double>(graphicsStart-engineStart);

            // render
            mRoot->_updateAllRenderTargets();
            mRenderWindow->update();
            mRoot->_fireFrameRenderingQueued();
            if(!mRoot->_fireFrameEnded())
                break;

            engineStart=timer.getMilliseconds();
            mStats.lastGraphicFrameDuration=static_cast<double>(engineStart-graphicsStart);
            mStats.lastFullFrameDuration=static_cast<double>(engineStart-frameStart);

            double dt=1000./30.-mStats.lastFullFrameDuration;

            if(dt>0)
                usleep(static_cast<useconds_t>(dt*ms2us));

            if (singleLoop)
                break;
        }
        return true;
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

    void Engine::pickAgents(std::list<AgentId> &selection, int x, int y)
    {
        //get nodes that collide
        std::list<Ogre::SceneNode *> nodes;
        Ogre::Real _x = float(x) / float(mRenderWindow->getWidth());
        Ogre::Real _y = float(y) / float(mRenderWindow->getHeight());
        Ogre::Ray ray = mLevel->camera()->cam()->getCameraToViewportRay(_x, _y);
        if (!mRayCaster->fromRay(ray, nodes))
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
        bool moveCam=false;
        float dx = .0f, dy = .0f, dz = .0f, speed = .5f;
        for (list<OIS::KeyCode>::iterator it = mInputMan.keysPressed().begin(); it != mInputMan.keysPressed().end(); ++it)
        {
            if(mEditMode)
            {
                // ONLY IN edit mode
                switch (*it)
                {
                    default:
                        break;
                }
            }
            else
            {
                if(mInputMan.isModifierDown(OIS::Keyboard::Ctrl))
                    speed*=2.f;
                // ONLY NOT IN edit mode
                switch (*it)
                {
                    case OIS::KC_W:
                        dz -= speed;
                        moveCam=true;
                        break;
                    case OIS::KC_A:
                        dx -= speed;
                        moveCam=true;
                        break;
                    case OIS::KC_S:
                        dz += speed;
                        moveCam=true;
                        break;
                    case OIS::KC_D:
                        dx += speed;
                        moveCam=true;
                        break;
                    case OIS::KC_SPACE:
                        dy += speed;
                        moveCam=true;
                        break;
                    case OIS::KC_LSHIFT:
                        dy -= speed;
                        moveCam=true;
                        break;
                    default:
                        break;
                }
            }
            // ALL THE TIME
            switch (*it)
            {
                case OIS::KC_ESCAPE:
                    return false;
                    break;
                default:
                    break;
            }
        }

        if(moveCam)
            mLevel->camera()->translate(dx, dy, dz,speed);

        //process mouse
        if (mInputMan.hasMouseMoved())
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
        if(command[0]=="level")
        {
            command.erase(command.begin());
            mLevel->processCommand(command);
        }
        else if(command[0]=="set_level")
        {
            if(command.size()>1)
            {
                auto newName=StringUtils::join(command,".",1);
                if(newName!=mLevel->name())
                {
                    // params should be valid now
                    Level *level=createLevel(newName);
                    Level *prev=setCurrentLevel(level);
                    if(level->getSavefile().exists())
                        level->load();
                    delete prev;
                }
                else
                {
                    Debug::warning("Engine::processCommand(): a new level requires a new name. Command was:");
                    Debug::warning(StringUtils::join(command,".")).endl();
                }
            }
            else
            {
                Debug::warning("Engine::processCommand(): invalid command (missing level name ?):");
                Debug::warning(StringUtils::join(command,".")).endl();
            }
        }
        else
        {
            Debug::warning("Engine::processCommand(): unknown command ");
            Debug::warning(StringUtils::join(command,".")).endl();
        }
        return true;
    }

    void Engine::processAllCommands()
    {
        while(!mCommands.empty())
        {
            std::vector<Ogre::String> command=mCommands.front();
            mCommands.pop_front();
            if(!processCommand(command))
                break;
        }
    }

    void Engine::registerCommand(std::vector<Ogre::String> command)
    {
        mCommands.push_back(command);
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

        if (mRenderWindow)
        {
            mRenderWindow->resize(width, height);
            mRenderWindow->windowMovedOrResized();
            if (mLevel->camera())
            {
                Ogre::Real aspectRatio = Ogre::Real(width) / Ogre::Real(height);
                mLevel->camera()->cam()->setAspectRatio(aspectRatio);
            }
        }
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
            agent->rotate(rotation);
        }
    }

    Ogre::Vector3 Engine::selectionPosition()
    {
        if (!hasSelection())
            return Ogre::Vector3::ZERO;
        return OgreUtils::mean(selectionPositions());
    }

    std::vector<Ogre::Vector3> Engine::selectionPositions()
    {
        std::vector<Ogre::Vector3> pos;
        if (!hasSelection())
            return pos;
        Agent *agent;
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            pos.push_back(agent->position());
        }
        return pos;
    }

    std::vector<Ogre::Quaternion> Engine::selectionRotations()
    {
        std::vector<Ogre::Quaternion> rots;
        if (!hasSelection())
            return rots;
        Agent *agent;
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            rots.push_back(agent->rotation());
        }
        return rots;
    }

    std::vector<Ogre::Vector3> Engine::selectionScales()
    {
        std::vector<Ogre::Vector3> scales;
        if (!hasSelection())
            return scales;
        Agent *agent;
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            scales.push_back(agent->scale());
        }
        return scales;
    }

    Ogre::Quaternion Engine::selectionOrientationFromCenter()
    {
        if (!hasSelection())
            return Ogre::Quaternion::IDENTITY;

        AgentId aid=mSelection.front();
        Agent *agent=mLevel->getAgent(aid);
        if (agent == NULL)
        {
            Debug::error("Engine::selectionOrientationFromCenter(): selection's first item (agent ")(aid)(") is not valid.").endl();
            return Ogre::Quaternion::IDENTITY;
        }
        return selectionPosition().getRotationTo(agent->position(),Ogre::Vector3::UNIT_Z);
    }

    std::vector<Ogre::Quaternion> Engine::selectionOrientationsFromCenter()
    {
        std::vector<Ogre::Quaternion> rots;
        if (!hasSelection())
            return rots;
        Agent *agent;
        auto mean=selectionPosition();;
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            rots.push_back(mean.getRotationTo(agent->position(),Ogre::Vector3::UNIT_Z));
        }
        return rots;
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

    void Engine::setSelectedAgents(std::list<AgentId> selection, bool replacePrevious)
    {
        if (replacePrevious)
            clearSelection();
        Agent *agent;
        //process actual selections
        for (std::list<AgentId>::iterator it = selection.begin(); it != selection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (NULL ==agent )
                continue;
            mSelection.push_back(agent->id());
            agent->setSelected(true);
        }
    }

    void Engine::removeFromSelection(const std::list<AgentId> &selection)
    {
        for (auto it_sel= selection.begin(); it_sel!= selection.end(); ++it_sel)
        {
            auto aid=*it_sel;
            if (INVALID_ID==aid)
                break;
            while(true)
            {
                auto it=std::find(mSelection.begin(),mSelection.end(),aid);
                if(mSelection.end()==it)
                    break;
                mSelection.erase(it);

                auto agent = mLevel->getAgent(aid);
                if (NULL == agent)
                    continue;
                agent->setSelected(false);
            }
        }
    }

    bool Engine::isSelected(AgentId aid)
    {
        return std::find(mSelection.begin(),mSelection.end(),aid)!=mSelection.end();
    }

    void Engine::setSelectionPosition(const Ogre::Vector3 &pos)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        Ogre::Vector3 diff=pos-OgreUtils::mean(selectionPositions());
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->move(diff);
        }
    }

    void Engine::setSelectionPositions(const std::vector<Ogre::Vector3> &pos)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        assert(pos.size()==mSelection.size());
        auto it_pos=pos.begin();
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->setPosition(*(it_pos++));
        }
    }

    void Engine::moveSelection(const Ogre::Vector3 &dpos)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->move(dpos);
        }
    }

    void Engine::moveSelection(const std::vector<Ogre::Vector3> &dpos)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        assert(dpos.size()==mSelection.size());
        auto it_pos=dpos.begin();
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->move(*(it_pos++));
        }
    }

    void Engine::setSelectionRotations(const std::vector<Ogre::Quaternion> &rots)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        assert(rots.size()==mSelection.size());
        auto it_rot=rots.begin();
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->setRotation(*(it_rot++));
        }
    }

    void Engine::rotateSelectionRotationAroundCenter(const Ogre::Radian &angle, const Ogre::Vector3 &axis)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        Ogre::Vector3 center=selectionPosition();
        auto rotation=Ogre::Quaternion(angle,axis);
        auto plane=Ogre::Plane(axis,center);
        if(mSelection.size()==1)
        {
            agent = mLevel->getAgent(mSelection.front());
            if (agent == NULL)
                return;
            agent->rotate(rotation);
        }
        else
        {
            for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
            {
                agent = mLevel->getAgent(*it);
                if (agent == NULL)
                    continue;;

                auto pos=agent->position();
                auto proj=plane.projectVector(pos);
                // rotated projection (rotated within plane space, but kept in world space)
                auto rotatedProj=rotation*(proj-center)+center;
                // deproject it as far as it was
                rotatedProj+=pos-proj;
                agent->setPosition(rotatedProj);
                agent->rotate(rotation);
            }
        }
    }

    void Engine::rescaleSelection(const Ogre::Vector3 &scale)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->rescale(scale);
        }
    }

    void Engine::setSelectionScales(const std::vector<Ogre::Vector3> &scales)
    {
        if (!hasSelection())
            return;

        Agent *agent;
        assert(scales.size()==mSelection.size());
        auto it_sca=scales.begin();
        for (std::list<AgentId>::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->setScale(*(it_sca++));
        }
    }

    void Engine::startEditMode()
    {
//         Debug::log("Engine::startEditMode()").endl();
        mEditMode=true;
        mUI.startEditMode();
        mInputMan.grabInput(false);
    }

    void Engine::stopEditMode()
    {
//         Debug::log("Engine::stopEditMode()").endl();
        mEditMode=false;
        mUI.stopEditMode();
        mInputMan.grabInput(true);
    }

}




// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
