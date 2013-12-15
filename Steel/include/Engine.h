#ifndef STEEL_ENGINE_H
#define STEEL_ENGINE_H

#include <string>
#include <list>

#include <OgreRoot.h>
#include <OgreLog.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreString.h>
#include <OISKeyboard.h>

#include "steeltypes.h"
#include "InputManager.h"
#include "tools/File.h"
#include "tools/ConfigFile.h"
#include "UI/UI.h"
#include "InputEventListener.h"

namespace Steel
{
    class Camera;
    class Level;
    class RayCaster;
    class EngineEventListener;

    class Engine: public InputEventListener
    {
    public:
        
        /// Reference lookup table setting name
        static const Ogre::String REFERENCE_PATH_LOOKUP_TABLE_SETTING;
        
        static const Ogre::String NONEDIT_MODE_GRABS_INPUT;
        static const Ogre::String COLORED_DEBUG;

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

        Engine(Ogre::String confFilename = Ogre::StringUtil::BLANK);
        virtual ~Engine();

        /// The given listener will be notified of engine event: onLevelSet, onLevelUnset.
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
        void embeddedInit(Ogre::String plugins, std::string windowHandle, unsigned int width, unsigned int height,
                          Ogre::String defaultLog = Ogre::String("ogre_log.log"), Ogre::LogListener *logListener = nullptr);

        /**
         * game-side/standalone init.
         * Automaticaly grabs mouse/keyboard inputs.
         */
        void init(Ogre::String plugins, bool fullScreen = false, unsigned int width = 800, unsigned int height = 600,
                  Ogre::String windowTitle = Ogre::String("Steel Window"), Ogre::LogListener *logListener = nullptr);

        bool mainLoop(bool singleLoop = false);
        
        /// InputEventListener interface.
        bool onInputEvent(Input::Event const &evt);

        /**
         * Takes window coordinates and lists into the given list all Agents whose OgreModel collides with a ray going from the camera
         * center to the given coordinates.
         * see http://www.ogre3d.org/tikiwiki/Raycasting+to+the+polygon+level
         */
        void pickAgents(Selection &selection, int x, int y);

        /// execute a serialized command. Return true if the next command can be processed before the next frame.
        bool processCommand(std::vector<Ogre::String> command);

        void redraw();
        /// adds a command that will be executed at the beginning of next frame.
        void registerCommand(std::vector<Ogre::String> command);
        /// adds a command that will be executed at the beginning of next frame.
        void registerCommand(Ogre::String rawCommand);

        void loadConfig(ConfigFile &config);
        void saveConfig(ConfigFile &config);

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

        /// Called to resize the window.
        void resizeWindow(int width, int height);
        
        /// Edit mode ghost cam update. Can be called during an onInputEvent to have the same camera control as in edit mode.
        void updateGhostCam();
        
        /**
         * Replaces references' special $keys by specific values (paths set in the conf).
         * The call is recursive (keys can reference other keys).
         */
        void resolveReferencePaths(Ogre::String const &src, Ogre::String &dst);

        ////////////////////////////////////////////////
        //getters
        inline InputManager *inputMan() {return &mInputMan;}

        inline Ogre::RenderWindow *renderWindow() {return mRenderWindow;}

        inline File rootDir() {return mRootDir;}
        inline File dataDir() {return mRootDir.subfile("data");}
        inline File rawResourcesDir() {return dataDir().subfile("raw_resources");}
        inline File resourcesDir() {return dataDir().subfile("resources");}
        inline File uiDir() {return dataDir().subfile("ui");}

        inline std::string &windowHandle() {return mWindowHandle;}

        /// Returns a pointer to the current level, or nullptr if none is currently set.
        inline Level *level() {return mLevel;}

        inline const Stats &stats() const {return mStats;}

        //setters

        /// Sets application main directory.
        void setRootDir(File rootdir);

        /// Sets application main directory.
        void setRootDir(Ogre::String rootdir);

        inline ConfigFile &config() {return mConfig;}

    private:
        /// Fired when the main new level has been set
        void fireOnLevelSetEvent();
        /// Fired when the main level has been unset
        void fireOnLevelUnsetEvent();
        /// Fired right before the main level is updated. Right moment to inject additional input.
        void fireOnBeforeLevelUpdate(float dt);
        /// Fired right after the main level is updated. Last step before next frame is the graphic update.
        void fireOnAfterLevelUpdate();
        /// Fired right after the engine enters edit mode.
        void fireOnStartEditMode();
        /// Fired right after the engine leaves edit mode.
        void fireOnStopEditMode();
        
        bool keyReleased(Input::Code key, Input::Event const &evt);
        
        /// invoke processCommand on all registered commands
        void processAllCommands();

        /**
         * set up stuff that does not depend on standalone/embedded status,
         * nor on having the windowing system ready;
         * Returns 0 if successfull, returns an error code otherwise.
         */
        int preWindowingSetup(Ogre::String &plugins, unsigned int width, unsigned int height, Ogre::String defaultLog =
                                  Ogre::String("steel_default_log.log"), Ogre::LogListener *logListener = nullptr);
        /**
         * set up stuff that does not depend on standalone/embedded status,
         * but that depend on having the windowing system ready;
         * Returns 0 if successfull, returns an error code otherwise.
         */
        int postWindowingSetup(unsigned int width, unsigned int height);
        
        bool processInputs();
        
        /**
         * Reads the conf to create the lookup table used to resolve dynamic resource locations in
         * models reference descriptor files.
         */
        void setupReferencePathsLookupTable(Ogre::String const &source);

        File mRootDir;
        ConfigFile mConfig;
        Ogre::Root *mRoot;
        Ogre::RenderWindow *mRenderWindow;

        InputManager mInputMan;
        std::string mWindowHandle;
        bool mMustAbortMainLoop;
        bool mIsInMainLoop;

        /// Current level
        Level *mLevel;

        /// Object that handles all this raycasting thingy.
        RayCaster *mRayCaster;
        UI mUI;

        bool mEditMode;
        Stats mStats;

        /// commands that will be executed at the beginning of next frame
        std::list<std::vector<Ogre::String> > mCommands;

        /// Called back about engine events.
        std::set<EngineEventListener *> mListeners;
        
        /// internal representation of REFERENCE_PATH_LOOKUP_TABLE attribute of the application conf file.
        std::map<Ogre::String, Ogre::String> mReferencePathsLookupTable;
    };
}

#endif /* STEEL_ENGINE_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

