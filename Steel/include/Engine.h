#ifndef STEEL_ENGINE_H_
#define STEEL_ENGINE_H_

#include <string>
#include <list>

#include <OgreRoot.h>
#include <OgreLog.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreString.h>

#include "steeltypes.h"
#include "InputManager.h"
#include "tools/File.h"
#include "tools/ConfigFile.h"
#include "UI/UI.h"
#include "SelectionManager.h"

namespace Steel
{
    class Camera;
    class Level;
    class RayCaster;
    class EngineEventListener;

    class Engine
    {
        public:
            class Stats
            {
                public:
                    /// msec
                    double lastGraphicFrameDuration;
                    /// includes eveything except rendering. msec
                    double lastEngineDuration;
                    /// msec
                    double lastFullFrameDuration;
            };

            Engine(Ogre::String confFilename="");
            virtual ~Engine();

            /// The given listener will be notified of engine event..
            void addEngineEventListener(EngineEventListener *listener);
            /// Remove the given listener from the set of notified listeners.
            void removeEngineEventListener(EngineEventListener *listener);

            inline void abortMainLoop()
            {
                mMustAbortMainLoop = true;
            }

            /// Creates and returns a new level.
            Level *createLevel(Ogre::String name);

            /**
             * init from an app that already has created the engine's rendering window.
             * Does not grab any input (this can be done with a call to grabInputs).
             */
            void embeddedInit(Ogre::String plugins,
                              std::string windowHandle,
                              unsigned int width,
                              unsigned int height,
                              Ogre::String defaultLog = Ogre::String("ogre_log.log"),
                              Ogre::LogListener *logListener = NULL);

            /// warns all EngineListeners that a new level has been set
            void fireOnLevelSetEvent();
            /// warns all EngineListeners that a new level has been set
            void fireOnLevelUnsetEvent();

            /**
             * game-side/standalone init.
             * Automaticaly grabs mouse/keyboard inputs.
             */
            void init(Ogre::String plugins,
                      bool fullScreen = false,
                      unsigned int width = 800,
                      unsigned int height = 600,
                      Ogre::String windowTitle = Ogre::String("Steel Window"),
                      Ogre::LogListener *logListener = NULL);

            bool mainLoop(bool singleLoop = false);

            // implements the OIS keyListener and mouseListener interfaces,
            // as this allows the inputManager to seamlessly pass events as they arrive.
            bool keyPressed(const OIS::KeyEvent& evt);
            bool keyReleased(const OIS::KeyEvent& evt);
            bool mouseMoved(const OIS::MouseEvent& evt);
            bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);

            /**
             * Takes window coordinates and lists into the given list all Agents whose OgreModel collides with a ray going from the camera
             * center to the given coordinates.
             * see http://www.ogre3d.org/tikiwiki/Raycasting+to+the+polygon+level
             */
            void pickAgents(Selection& selection, int x, int y);

            /// execute a serialized command. Return true if the next command can be processed before the next frame.
            bool processCommand(std::vector<Ogre::String> command);

            void redraw();
            /// adds a command that will be executed at the beginning of next frame.
            void registerCommand(std::vector<Ogre::String> command);
            /// adds a command that will be executed at the beginning of next frame.
            void registerCommand(Ogre::String rawCommand);

            void reloadConfig();

            /**
             * Sets the given level as the new one. Returns the previous level, that still needs to be deleted.
             * TODO: implement level background unloading ?
             * Since the current level holds the Ogre::SceneManager on which most of what we want
             * to display relies, this same everything needs to be shutdown before proceeding, so that
             * it can be reinstanciated with the new level's sceneManager. Therefore, this method is a bit
             * expensive, and may make the engine not as responsive until it exits.
             */
            Level *setCurrentLevel(Level *newLevel);
            void shutdown();

            void startEditMode();
            void stopEditMode();

            /**
             * called to resize the window.
             */
            void resizeWindow(int width, int height);

            ////////////////////////////////////////////////
            //getters
            inline InputManager *inputMan()
            {
                return &mInputMan;
            }

            inline Ogre::RenderWindow *renderWindow()
            {
                return mRenderWindow;
            }

            inline File rootDir()
            {
                return mRootDir;
            }

            inline File dataDir()
            {
                return mRootDir.subfile("data");
            }

            inline File rawResourcesDir()
            {
                return dataDir().subfile("raw_resources");
            }

            inline SelectionManager &selectionMan()
            {
                return mSelectionMan;
            }

            inline std::string &windowHandle()
            {
                return mWindowHandle;
            }

            inline Level *level()
            {
                return mLevel;
            }
//             inline UI *ui()
//             {
//                 return &mUI;
//             }

            inline const Stats &stats() const
            {
                return mStats;
            }

            //setters

            /// Sets application main directory.
            void setRootDir(File rootdir);

            /// Sets application main directory.
            void setRootDir(Ogre::String rootdir);

            inline ConfigFile &config()
            {
                return mConfig;
            }

        protected:
            /// invoke processCommand on all registered commands
            void processAllCommands();

            /**
             * set up stuff that does not depend on standalone/embedded status,
             * nor on having the windowing system ready;
             * Returns 0 if successfull, returns an error code otherwise.
             */
            int preWindowingSetup(Ogre::String &plugins,
                                  unsigned int width,
                                  unsigned int height,
                                  Ogre::String defaultLog =Ogre::String("steel_default_log.log"),
                                  Ogre::LogListener *logListener = NULL);
            /**
             * set up stuff that does not depend on standalone/embedded status,
             * but that depend on having the windowing system ready;
             * Returns 0 if successfull, returns an error code otherwise.
             */
            int postWindowingSetup(unsigned int width, unsigned int height);

            /// high level input processing
            bool processInputs();

            File mRootDir;
            ConfigFile mConfig;
            Ogre::Root *mRoot;
            Ogre::RenderWindow *mRenderWindow;

            InputManager mInputMan;
            std::string mWindowHandle;
            bool mMustAbortMainLoop;

            /**
             * current level.
             */
            Level *mLevel;

            /**
             * object that handles all this raycasting thingies.
             */
            RayCaster *mRayCaster;
            UI mUI;

            bool mEditMode;
            Stats mStats;

            /// commands that will be executed at the beginning of next frame
            std::list<std::vector<Ogre::String> > mCommands;

            /// Called back about engine events.
            std::set<EngineEventListener *> mListeners;

            /// Manages selections of Agents.
            SelectionManager mSelectionMan;

    };
}

#endif /* STEEL_ENGINE_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

