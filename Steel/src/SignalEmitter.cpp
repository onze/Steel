
#include "SignalEmitter.h"
#include "Debug.h"
#include "SignalManager.h"

namespace Steel
{
    void SignalEmitter::emit(const Signal signal, bool anonymous/* = false*/)
    {
        if(INVALID_SIGNAL == signal)
            Debug::log("SignalEmitter::emit(")(signal)("/").quotes(SignalManager::instance().fromSignal(signal))(") is invalid !").endl().breakHere();
        else
            SignalManager::instance().emit(signal, anonymous ? nullptr : this);
    }

    void SignalEmitter::emit(const Ogre::String &signal, bool anonymous/* = false*/)
    {
        emit(SignalManager::instance().toSignal(signal), anonymous);
    }

    void SignalEmitter::fire(const Signal signal, bool anonymous/* = false*/)
    {
        if(INVALID_SIGNAL == signal)
            Debug::log("SignalEmitter::fire(")(signal)("/").quotes(SignalManager::instance().fromSignal(signal))(") is invalid !").endl().breakHere();
        else
            SignalManager::instance().fire(signal, anonymous ? nullptr : this);
    }

    void SignalEmitter::fire(const Ogre::String &signal, bool anonymous/* = false*/)
    {
        fire(SignalManager::instance().toSignal(signal), anonymous);
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
