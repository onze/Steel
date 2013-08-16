
#include "SignalListener.h"
#include "SignalManager.h"
#include "SignalEmitter.h"

namespace Steel
{

    SignalListener::SignalListener()
        : mRegisteredSignals(std::set<Signal>())
#ifdef DEBUG
        , mRegisteredSignalStrings(std::set<Ogre::String>())
#endif
    {
    }

    SignalListener::~SignalListener()
    {
        unregisterAllSignals();
    }
    
    void SignalListener::registerSignal(const Ogre::String& signal)
    {
        registerSignal(SignalManager::instance().toSignal(signal));
    }

    void SignalListener::registerSignal(const Signal signal)
    {
        SignalManager::instance().registerListener(signal, this);
        mRegisteredSignals.insert(signal);
#ifdef DEBUG
        mRegisteredSignalStrings.insert(SignalManager::instance().fromSignal(signal));
#endif
    }

    void SignalListener::unregisterSignal(Signal signal)
    {
        int found=mRegisteredSignals.erase(signal);
        if(found)
        {
            mRegisteredSignalStrings.erase(SignalManager::instance().fromSignal(signal));
            SignalManager::instance().unregisterListener(signal,this);
        }
    }
    
    void SignalListener::unregisterAllSignals()
    {
        while(mRegisteredSignals.size())
            unregisterSignal(*(mRegisteredSignals.begin()));
    }

    void SignalListener::_onSignal(Signal signal, SignalEmitter* src)
    {
        //TODO: call bound methods
        this->onSignal(signal, src);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
