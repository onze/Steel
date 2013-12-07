
#include "InputEventListener.h"

namespace Steel
{
    bool InputEventListener::onInputEvent(Input::Event const &evt)
    {
        switch(evt.device)
        {
            case Input::Device::KEYBOARD:
                switch(evt.type)
                {
                    case Input::Type::DOWN:
                        return this->keyPressed(evt.code, evt);

                    case Input::Type::UP:
                        return this->keyReleased(evt.code, evt);

                    default: return true;
                }

            case Input::Device::MOUSE:
                switch(evt.type)
                {
                    case Input::Type::DOWN:
                        return this->mousePressed(evt.code, evt);

                    case Input::Type::UP:
                        return this->mouseReleased(evt.code, evt);

                    case Input::Type::MOVE:
                        return this->mouseMoved(evt.position, evt);
                        
                    case Input::Type::NONE:
                        if(evt.code == Input::Code::MC_WHEEL)
                            return this->mouseWheeled(evt.delta, evt);

                    default: return true;
                }

            default: return true;
        }
    }
}
