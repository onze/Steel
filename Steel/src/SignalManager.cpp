
#include "Debug.h"
#include "SignalManager.h"
#include "SignalListener.h"
#include "SignalEmitter.h"

namespace Steel
{
    SignalManager *SignalManager::sInstance = nullptr;

    SignalManager::SignalManager(): mNextSignal(0L), mSignalsMap(), mInverseSignalsMap(), mListeners(), mEmittedSignals()
    {
    }

    SignalManager::~SignalManager()
    {

    }

    void SignalManager::emit(Signal signal, SignalEmitter *src)
    {
        mEmittedSignals.insert(std::make_pair(signal, src));
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

    SignalManager &SignalManager::fire(Signal signal, SignalEmitter *const src/* = nullptr*/)
    {
        auto it = mListeners.find(signal);

        if(mListeners.end() == it)
            return *this;

        std::set<SignalListener *> listeners(it->second);

        for(auto & listener : listeners)
            listener->_onSignal(signal, src);

        return *this;
    }

    SignalManager &SignalManager::fire(const Ogre::String &signal, SignalEmitter *const src/* = nullptr*/)
    {
        return fire(toSignal(signal), src);
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

    Signal SignalManager::anonymousSignal()
    {
        return mNextSignal++;
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

        return "<anonymous signal>";
    }

    void SignalManager::registerListener(const Signal signal, SignalListener *listener)
    {
        if(nullptr == listener)
        {
            Debug::warning(STEEL_METH_INTRO, "trying to insert a nullptr listener for signal ");
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

