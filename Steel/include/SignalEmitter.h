#ifndef STEEL_SIGNALEMITTER_H
#define STEEL_SIGNALEMITTER_H

#include <OgreString.h>

#include "steeltypes.h"
#include "SignalManager.h"
#include "Debug.h"

namespace Steel
{
    class SignalEmitter
    {
    public:
        inline void emit(const Signal signal, bool anonymous = false)
        {
//             Debug::log("SignalEmitter::emit(")(signal)("/").quotes(SignalManager::instance().fromSignal(signal))(")").endl();
            if(INVALID_SIGNAL == signal)
                Debug::log("SignalEmitter::emit(")(signal)("/").quotes(SignalManager::instance().fromSignal(signal))(") is invalid !").endl().breakHere();
            else
                SignalManager::instance().emit(signal, anonymous ? nullptr : this);
        }

        inline void emit(const Ogre::String &signal, bool anonymous = false)
        {
            emit(SignalManager::instance().toSignal(signal), anonymous);
        }
    };
}

#endif // STEEL_SIGNALEMITTER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
