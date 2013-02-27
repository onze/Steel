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

            /// Set the currenlty selected agents. If replacePrevious is true (default), any previous selection is cancelled.
            void setSelectedAgents(Selection selection, bool replacePrevious=true);
            void clearSelection();
            void deleteSelection();
            void removeFromSelection(const Selection &aids);
            inline bool hasSelection()
            {
                return !mSelection.empty();
            }

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

            /// Return true if the given AgentId is part of the current selection.
            bool isSelected(AgentId aid);
            /// Returns mean position of selected agents.
            Ogre::Vector3 selectionPosition();
            /// Returns positions of selected agents.
            std::vector< Ogre::Vector3 > selectionPositions();

            /// Returns orientations of selected agents' models.
            std::vector<Ogre::Quaternion> selectionRotations();
            /** Returns the shortest arc quaternion to rotate the selection
             * center to the first selected agent's model.*/
            Ogre::Quaternion selectionOrientationFromCenter();
            /** Same as selectionOrientationFromCenter, but for each selected agent's model.*/
            std::vector<Ogre::Quaternion> selectionOrientationsFromCenter();
            /// Returns scale of selected agents' models.
            std::vector<Ogre::Vector3> selectionScales();

            /// Move all selected agents' models by the given amount.
            void moveSelection(const Ogre::Vector3 &dpos);
            /// Move the <i>i</i>th selected agent's model by the <i>i</i>th given amount.
            void moveSelection(const std::vector<Ogre::Vector3> &dpos);
            /// Moves each selected agent away from the selection center.
            void expandSelection(float d);

            /// Move all selected agents' models to the given position.
            void setSelectionPosition(const Ogre::Vector3 &pos);
            /// Move the <i>i</i>th selected agent's model to the <i>i</i>th given position.
            void setSelectionPositions(const std::vector<Ogre::Vector3> &pos);

            /// Rotate the <i>i</i>th selected agent's model to the <i>i</i>th given rotation.
            void setSelectionRotations(const std::vector<Ogre::Quaternion> &rots);

            /** Move all selected agents' models around the selection center position,
             * so that the first agent's model's projection on a plane with the given
             * normal has the given angle to the z axis.
             * */
            void rotateSelectionRotationAroundCenter(const Ogre::Radian &angle, const Ogre::Vector3 &axis);

            /// Rotate all selected agents' models by the given rotation. See OgreModel::rotate for details.
            void rotateSelection(Ogre::Vector3 rotation);

            /// Rescale all selected agents' models by the given factor.
            void rescaleSelection(const Ogre::Vector3 &scale);
            /// Scale the <i>i</i>th selected agent's model to the <i>i</i>th given factor.
            void setSelectionScales(const std::vector<Ogre::Vector3> &scale);

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

            inline Selection selection()
            {
                return mSelection;
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

            /// agents currenlty selected
            Selection mSelection;

            bool mEditMode;
            Stats mStats;

            /// commands that will be executed at the beginning of next frame
            std::list<std::vector<Ogre::String> > mCommands;

            /// Called back about engine events.
            std::set<EngineEventListener *> mListeners;

    };
}

#endif /* STEEL_ENGINE_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

