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

#include <OgreRoot.h>
#include <OgreLog.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreString.h>

#include "InputManager.h"
#include "Camera.h"
#include "Level.h"
#include "RayCaster.h"
#include "tools/File.h"
#include "UI/UI.h"

namespace Steel
{

    class InputManager;

    class Engine
    {
        public:
            Engine();
            virtual ~Engine();

            inline void abortMainLoop()
            {
                mMustAbortMainLoop = true;
            }
            Level *createLevel(Ogre::String name);
            void clearSelection();
            void deleteSelection();

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

            inline bool hasSelection()
            {
                return !mSelection.empty();
            }

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
             * Takes window coordinates and lists under the given list all Agent's that collide with a ray going from the camera
             * center to the given coordinates.
             * see http://www.ogre3d.org/tikiwiki/Raycasting+to+the+polygon+level
             */
            void pickAgents(std::list<ModelId> &selection, int x, int y);

            void redraw();
            void setSelectedAgents(std::list<AgentId> selection, bool selected);
            void shutdown();

            void startEditMode();
            void stopEditMode();

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

            inline Camera *camera()
            {
                return mCamera;
            }
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
            inline std::list<AgentId> selection()
            {
                return mSelection;
            }
            inline std::string &windowHandle()
            {
                return mWindowHandle;
            }
//             inline UI *ui()
//             {
//                 return &mUI;
//             }

            //setters

            /**
             * sets application main directory.
             */
            void setRootDir(File rootdir);

            /**
             * sets application main directory.
             */
            void setRootDir(Ogre::String rootdir);

            void setSelectionPosition(Ogre::Vector3 pos);

        private:
            File mRootDir;
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

            Ogre::Root *mRoot;
            Ogre::SceneManager *mSceneManager;
            Ogre::RenderWindow *mRenderWindow;
            Ogre::Viewport *mViewport;
            Camera *mCamera;

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

            std::list<AgentId> mSelection;

            bool mEditMode;
    };

}

#include "InputManager.h"
#endif /* ENGINE_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

