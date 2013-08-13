
#include "SignalListener.h"
#include "SignalManager.h"
#include "SignalEmitter.h"

namespace Steel
{

    SignalListener::SignalListener()
        : mRegisteredSignals(std::vector<Signal>())
#ifdef DEBUG
        , mRegisteredSignalStrings(std::vector<Ogre::String>())
#endif
    {
    }

    SignalListener::~SignalListener()
    {
    }
    
    void SignalListener::registerSignal(const Ogre::String& signal)
    {
        registerSignal(SignalManager::instance()->toSignal(signal));
    }

    void SignalListener::registerSignal(const Signal signal)
    {
        SignalManager::instance()->registerListener(signal, this);
        mRegisteredSignals.push_back(signal);
#ifdef DEBUG
        mRegisteredSignalStrings.push_back(SignalManager::instance()->fromSignal(signal));
#endif
    }

    void SignalListener::unregisterSignal(Signal signal)
    {

    }

    void SignalListener::_onSignal(Signal signal, SignalEmitter* src)
    {
        //TODO: call bound methods
        this->onSignal(signal, src);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
