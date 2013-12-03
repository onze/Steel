#ifndef STEEL_INPUTLISTENER_H_
#define STEEL_INPUTLISTENER_H_

#include "Input.h"

namespace Steel
{
    class InputEventListener
    {
    public:
        /// Returns false to keep next listeners from receiving the event.
        virtual bool onInputEvent(Input::Event const &evt);
        
        /// Returns false to keep next listeners from receiving the event.
        virtual bool keyPressed(Input::Code key, Input::Event const &evt){return true;};
        
        /// Returns false to keep next listeners from receiving the event.
        virtual bool keyReleased(Input::Code key, Input::Event const &evt){return true;};
        
        /// Returns false to keep next listeners from receiving the event.
        virtual bool mousePressed(Input::Code button, Input::Event const &evt){return true;};
        
        /// Returns false to keep next listeners from receiving the event.
        virtual bool mouseReleased(Input::Code button, Input::Event const &evt){return true;};
        
        /// Returns false to keep next listeners from receiving the event.
        virtual bool mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt){return true;};
    };
}

#endif //STEEL_INPUTLISTENER_H_
