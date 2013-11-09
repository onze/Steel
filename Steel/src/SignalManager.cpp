
#include "Debug.h"
#include "SignalManager.h"
#include "SignalListener.h"
#include "SignalEmitter.h"

namespace Steel
{
    SignalManager *SignalManager::sInstance = nullptr;

    SignalManager::SignalManager(): mNextSignal(0L), mSignalsMap(std::map<Ogre::String, Signal>())
#ifdef DEBUG
        , mInverseSignalsMap(std::map<Signal, Ogre::String>())
#endif
        , mListeners(std::map<Signal, std::set<SignalListener *>>()),
        mEmittedSignals(std::set<std::pair<Signal, SignalEmitter *>>())
    {
    }

    SignalManager::~SignalManager()
    {

    }

    void SignalManager::emit(Signal signal, SignalEmitter *src)
    {
        mEmittedSignals.insert(std::pair<Signal, SignalEmitter *>(signal, src));
    }

    void SignalManager::fireEmittedSignals()
    {
        if(mEmittedSignals.size())
        {
            decltype(mEmittedSignals) copy(mEmittedSignals.begin(), mEmittedSignals.end());
            mEmittedSignals.clear();

            while(copy.size())
            {
                auto kv = copy.begin();
                fire(kv->first, kv->second);
                copy.erase(kv);
            }
        }
    }

    void SignalManager::fire(Signal signal, SignalEmitter *src)
    {
        std::map<Signal, std::set<SignalListener *>>::iterator it = mListeners.find(signal);

        if(mListeners.end() == it)
            return;

        std::set<SignalListener *> listeners(it->second);

        for(auto & listener : listeners)
        {
            listener->_onSignal(signal, src);
        }
    }

    Signal SignalManager::toSignal(const Ogre::String &signal)
    {
        Signal returnedValue = INVALID_SIGNAL;

        auto it = mSignalsMap.find(signal);

        if(mSignalsMap.end() == it)
        {
            mSignalsMap[signal] = returnedValue = mNextSignal;
            mInverseSignalsMap[mNextSignal] = signal;
            ++mNextSignal;
        }
        else
        {
            returnedValue = it->second;
        }

        return returnedValue;
    }

    Ogre::String SignalManager::fromSignal(const Signal signal)
    {
        if(INVALID_SIGNAL == signal)
        {
            return "<INVALID_SIGNAL>";
        }

        auto it = mInverseSignalsMap.find(signal);

        if(mInverseSignalsMap.end() != it)
        {
            return it->second;
        }

        return "unknown signal";
    }

    void SignalManager::registerListener(const Signal signal, SignalListener *listener)
    {
        if(nullptr == listener)
        {
            Debug::warning("in SignalManager::registerListener(): trying to insert a nullptr listener for signal ");
            Debug::warning(signal)(" / ")(fromSignal(signal)).endl();
            return;
        }

        mListeners.emplace(signal, std::set<SignalListener *>()).first->second.insert(listener);
    }

    void SignalManager::unregisterListener(const Signal signal, SignalListener *listener)
    {
        if(nullptr == listener)
            return;

        auto signal_it = mListeners.find(signal);

        if(mListeners.end() != signal_it)
        {
            mListeners[signal].erase(listener);
        }
    }

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
