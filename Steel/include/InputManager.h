/*
 * InputManager.h
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

#include <string>
#include <list>

#include "UI/UI.h"
#include <OgreWindowEventUtilities.h>
#include <OIS.h>

namespace Steel
{
    class Engine;
    /**
     * owned by the engine. Fed input primarily by the UI (possibly by network/automation tools).
     */
    class InputManager:public Ogre::WindowEventListener, OIS::MouseListener, OIS::KeyListener
    {
        public:
            static const int KEYBOARD=1<<0;
            static const int MOUSE=1<<1;

            InputManager();
            virtual ~InputManager();
            void init(Engine *engine, UI *ui);
            void shutdown();

            bool keyPressed(const OIS::KeyEvent& evt);
            bool keyReleased(const OIS::KeyEvent& evt);
            bool mouseMoved(const OIS::MouseEvent& evt);
            bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);

            void grabMouse();
            void grabKeyboard();
            void releaseMouse();
            void releaseKeyboard();
            void releaseAll()
            {
                releaseMouse();
                releaseKeyboard();
            };

            void update();
            void close();
            void resetFrameBasedData();
            void resetAllData();

            inline bool isKeyDown(OIS::KeyCode key)
            {
                return mKeyboard==NULL?false:mKeyboard->isKeyDown(key);
            }
            inline bool isModifierDown(OIS::Keyboard::Modifier mod)
            {
                return mKeyboard==NULL?false:mKeyboard->isModifierDown(mod);
            }

            /**
             * called by OIS when the window has been resized.
             */
            virtual void windowResized(Ogre::RenderWindow* rw);
            virtual void windowClosed(Ogre::RenderWindow* rw);

            //getters
            inline std::list<OIS::KeyCode> &keysPressed()
            {
                return mKeysPressed;
            };
            inline bool hasMouseMoved()
            {
                return mHasMouseMoved;
            };
            inline Ogre::Vector2 &mouseMove()
            {
                return mMouseMove;
            };
            inline OIS::Mouse* mouse()
            {
                return mMouse;
            };
            inline OIS::Keyboard* keyboard()
            {
                return mKeyboard;
            };

        protected:
            //not owned
            Engine *mEngine;
            UI *mUI;

            //owned
            OIS::InputManager* mOISInputManager;
            OIS::Mouse* mMouse;
            OIS::Keyboard* mKeyboard;

            std::list<OIS::KeyCode> mKeysPressed;
            bool mHasMouseMoved;
            /**
             * move since last known position, and last known position.
             */
            Ogre::Vector2 mMouseMove,mMousePos;
    };

}

#endif /* INPUTMANAGER_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
