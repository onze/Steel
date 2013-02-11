#ifndef STEEL_INPUTMANAGER_H_
#define STEEL_INPUTMANAGER_H_

#include <string>
#include <list>

#include <OIS.h>
#include <OgreVector2.h>

#include <OgreWindowEventUtilities.h>

namespace Steel
{
    class Engine;
    class UI;
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

            void grabInput(bool exclusive=true);

            inline bool isKeyDown(OIS::KeyCode key)
            {
                return NULL==mKeyboard?false:mKeyboard->isKeyDown(key);
            }
            inline bool isModifierDown(OIS::Keyboard::Modifier mod)
            {
                return NULL==mKeyboard?false:mKeyboard->isModifierDown(mod);
            }

            bool keyPressed(const OIS::KeyEvent& evt);
            bool keyReleased(const OIS::KeyEvent& evt);
            bool mouseMoved(const OIS::MouseEvent& evt);
            bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);

            void releaseInput();
            void resetFrameBasedData();
            void resetAllData();

            void update();

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
            /// Absolute on-screen mouse position, in pixels.
            inline Ogre::Vector2 &mousePos()
            {
                return mMousePos;
            };
            /// Absolute on-screen mouse position, in pixels.
            inline Ogre::Vector2 &mousePosAtLastMousePressed()
            {
                return mMousePosAtLastMousePressed;
            };
            inline OIS::Mouse* mouse()
            {
                return mMouse;
            };
            inline OIS::Keyboard* keyboard()
            {
                return mKeyboard;
            };

            //setters
            /// move the mouse to the given window position
            void setMousePosition(Ogre::Vector2 &pos);

        protected:
            /// save the state of the mouse for later. (currenlty, only position is saved)
            void pushMouseState();
            /// load the last saved state of the mouse. (currenlty, only position is restored)
            void popMouseState();
            void _grabInput(bool exclusive=true);
            void _releaseInput();
            OIS::ParamList getOISparams(bool exclusive);

            //not owned
            Engine *mEngine;
            UI *mUI;

            //owned
            /// create mMouse and mKeyboard according to parameters.
            OIS::InputManager* mOISInputManager;
            ///true if the input is grabbed
            bool mIsInputGrabbed;
            ///true if the current grab is exclusive
            bool mIsGrabExclusive;

            ///true as long as a requested delayed grab has not been processed
            bool mDelayedInputGrabRequested;
            ///true as long as a requested delayed release has not been processed
            bool mDelayedInputReleaseRequested;
            bool mDelayedRequestIsExclusive;

            OIS::Mouse* mMouse;
            OIS::Keyboard* mKeyboard;

            std::list<OIS::KeyCode> mKeysPressed;
            bool mHasMouseMoved;
            /// Move since last known position, last known position.
            Ogre::Vector2 mMouseMove,mMousePos,mLastMouseMove;
            
            Ogre::Vector2 mMousePosAtLastMousePressed;

            /// used by push/popMouseState. Store mouse position only.
            std::list<Ogre::Vector2> mMouseStateStack;
    };

}

#endif /* STEEL_INPUTMANAGER_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
