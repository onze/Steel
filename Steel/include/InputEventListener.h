#ifndef STEEL_INPUTLISTENER_H_
#define STEEL_INPUTLISTENER_H_

#include "Input.h"

namespace Steel
{
    class InputEventListener
    {
    public:
        virtual void onInputEvent(Input::Event const &evt);
        
        virtual bool keyPressed(Input::Code key, Input::Event const &evt){return true;};
        virtual bool keyReleased(Input::Code key, Input::Event const &evt){return true;};
        virtual bool mousePressed(Input::Code button, Input::Event const &evt){return true;};
        virtual bool mouseReleased(Input::Code button, Input::Event const &evt){return true;};
        virtual bool mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt){return true;};
    };
}

#endif //STEEL_INPUTLISTENER_H_
