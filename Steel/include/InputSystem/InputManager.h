#ifndef STEEL_INPUTMANAGER_H_
#define STEEL_INPUTMANAGER_H_

#include <OISMouse.h>
#include <OISKeyboard.h>

#include "steeltypes.h"
#include "InputBuffer.h"
#include "Input.h"

namespace Steel
{
    class Engine;
    class UI;
    class InputEventListener;
    
    /**
     * owned by the engine. Fed input primarily by the UI (possibly by network/automation tools).
     */
    class InputManager: public Ogre::WindowEventListener, public OIS::MouseListener, public OIS::KeyListener
    {
    public:
        InputManager();
        virtual ~InputManager();
        void init(Engine *engine);
        void shutdown();
        
        void addInputEventListener(InputEventListener *listener);
        void removeInputEventListener(InputEventListener *listener);
        void fireInputEvent(Input::Event evt);

        void grabInput(bool exclusive = true);

        inline bool isKeyDown(Input::Code code)
        {
            return mCodesPressed.end() != std::find(mCodesPressed.begin(), mCodesPressed.end(), code);
        }

        bool keyPressed(const OIS::KeyEvent &evt);
        bool keyReleased(const OIS::KeyEvent &evt);
        bool mouseMoved(const OIS::MouseEvent &evt);
        bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
        bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);

        /// Registers for input release. This will be done before next frame events.
        void releaseInput();
        /// Process to input release now. Do not call this during the message event dispatching phase.
        void _releaseInput();
        
        void resetFrameBasedData();
        void resetAllData();
        void update();
        
        /////////////////////////////////
        // Actions related stuff
        /// Makes the inputManager fire a signal upon receiving the given input. This can help map input from harware space (A, B) to game space (attack, defend).
        void registerAction(Input::Code const code, Input::Type const type, Signal const signal);
        void unregisterAction(Input::Code const code, Input::Type const type, Signal const signal);
        void unregisterAllActions();
        bool loadActionFile(File const &file);
        

        /**
         * called by OIS when the window has been resized.
         */
        virtual void windowResized(Ogre::RenderWindow *rw);
        virtual void windowClosed(Ogre::RenderWindow *rw);
        
        float windowHeight() const;
        float windowWidth() const;

        //getters
        /// alias to codesPressed
        inline std::list<Input::Code> const &keysPressed() {return codesPressed();}
        /// Input::Code currently pressed
        inline std::list<Input::Code> const &codesPressed() {return mCodesPressed;}
        /// True if the mouse is... moving this frame.
        inline bool hasMouseMoved() {return mHasMouseMoved;}
        /// Mouse position delta this frame
        inline Ogre::Vector2 &mouseMove() {return mMouseMove;}
        /// Absolute on-screen mouse position, in pixels.
        inline Ogre::Vector2 &mousePos() {return mMousePos;}
        /// Absolute on-screen mouse position, in pixels.
        inline Ogre::Vector2 &mousePosAtLastMousePressed() {return mMousePosAtLastMousePressed;}

        inline OIS::Mouse *mouse() {return mMouse;}
        inline OIS::Keyboard *keyboard() {return mKeyboard;}
        inline InputBuffer &inputBuffer() {return mInputBuffer;}

        //setters
        /// move the mouse to the given window position
        void setMousePosition(Ogre::Vector2 const &pos);

    private:
        /// Fires keypress/release events for modifiers
        void fireModifiers();
        
        /// maps OIS key/mouse codes to Steel ones. This is required for mouse/keyboard unification.
        static void buildCodesMaps();
        
        //MODIFIERS
        typedef std::map<OIS::Keyboard::Modifier, Input::Code> ModifierCodeIdentifierMap;
        static ModifierCodeIdentifierMap sOISModiferToInputCodeMap;
        /// safe accessor to sOISModiferToInputCodeMap
        Input::Code getModifierInputCode(OIS::Keyboard::Modifier mod);
        
        //KEYS
        typedef std::map<OIS::KeyCode, Input::Code> KeyCodeIdentifierMap;
        static KeyCodeIdentifierMap sOISKeyToInputCodeMap;
        /// safe accessor to sOISKeyToInputCodeMap
        Input::Code getKeyInputCode(OIS::KeyCode key);
        
        //MOUSE
        typedef std::map<OIS::MouseButtonID, Input::Code> MouseButtonCodeIdentifierMap;
        static MouseButtonCodeIdentifierMap sOISMouseToInputCodeMap;
        /// safe accessor to sOISMouseToInputCodeMap
        Input::Code getMouseInputCode(OIS::MouseButtonID btn);
        
        /// save the state of the mouse for later. (currenlty, only position is saved)
        void pushMouseState();
        /// load the last saved state of the mouse. (currenlty, only position is restored)
        void popMouseState();
        void _grabInput(bool exclusive = true);
        OIS::ParamList getOISparams(bool exclusive);
        
        /// Sends wheel events
        void updateWheelState(OIS::MouseEvent const &evt);

        //not owned
        Engine *mEngine;

        //owned
        /// create mMouse and mKeyboard according to parameters.
        OIS::InputManager *mOISInputManager;
        ///true if the input is grabbed
        bool mIsInputGrabbed;
        ///true if the current grab is exclusive
        bool mIsGrabExclusive;

        ///true as long as a requested delayed grab has not been processed
        bool mDelayedInputGrabRequested;
        ///true as long as a requested delayed release has not been processed
        bool mDelayedInputReleaseRequested;
        bool mDelayedRequestIsExclusive;

        OIS::Mouse *mMouse;
        OIS::Keyboard *mKeyboard;

        std::list<Input::Code> mCodesPressed;
        bool mHasMouseMoved;
        /// Move since last known position, last known position.
        Ogre::Vector2 mMouseMove, mMousePos, mLastMouseMove;

        Ogre::Vector2 mMousePosAtLastMousePressed;

        /// used by push/popMouseState. Store mouse position only.
        std::list<Ogre::Vector2> mMouseStateStack;
        
        /// Called back about input events.
        std::set<InputEventListener *> mListeners;
        
        /// Register of InputEvent happening -> Actions to fire.
        std::map<std::pair<Input::Code, Input::Type>, std::list<Signal>> mActionsRegister;
        
        InputBuffer mInputBuffer;
        
    };

}

#endif /* STEEL_INPUTMANAGER_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
