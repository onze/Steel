
#include "InputEventListener.h"

namespace Steel
{
    void InputEventListener::onInputEvent(Input::Event const &evt)
    {
        switch(evt.device)
        {
            case Input::Device::KEYBOARD:
                switch(evt.type)
                {
                    case Input::Type::DOWN:
                        this->keyPressed(evt.code, evt);
                        break;

                    case Input::Type::UP:
                        this->keyReleased(evt.code, evt);
                        break;

                    default: break;
                }

                break;

            case Input::Device::MOUSE:
                switch(evt.type)
                {
                    case Input::Type::DOWN:
                        this->mousePressed(evt.code, evt);
                        break;

                    case Input::Type::UP:
                        this->mouseReleased(evt.code, evt);
                        break;

                    case Input::Type::MOVE:
                        this->mouseMoved(evt.position, evt);
                        break;

                    default: break;
                }

                break;

            default: break;
        }
    }
}
