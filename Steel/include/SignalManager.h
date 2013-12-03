#ifndef STEEL_SIGNALMANAGER_H
#define STEEL_SIGNALMANAGER_H

#include <map>
#include <set>

#include <OgreString.h>

#include "steeltypes.h"

namespace Steel
{
    class SignalListener;
    class SignalEmitter;

    class SignalManager
    {
    public:
        inline static SignalManager &instance()
        {
            if(nullptr == SignalManager::sInstance)
                SignalManager::sInstance = new SignalManager();

            return *SignalManager::sInstance;
        }

        SignalManager();
        virtual ~SignalManager();

        void registerListener(const Signal signal, SignalListener *listener);
        void unregisterListener(const Signal signal, SignalListener *listener);

        /// Registers the signal to be fired before next frame (recommended).
        void emit(const Signal signal, SignalEmitter *src = nullptr);
        inline void emit(const Ogre::String &signal, SignalEmitter *src = nullptr)
        {
            emit(toSignal(signal), src);
        }

        /// Immediatly calls all registered listeners of the given signal.
        void fire(const Signal signal, SignalEmitter *src = nullptr);
        inline void fire(const Ogre::String &signal, SignalEmitter *src = nullptr)
        {
            fire(toSignal(signal), src);
        }

        /// Fires all emitted signals.
        void fireEmittedSignals();

        Signal toSignal(const Ogre::String &signal);
        Ogre::String fromSignal(const Signal signal);

    private:
        static SignalManager *sInstance;

        /// Value of the next created signal.
        Signal mNextSignal;
        /// Maps string signals to long values, used internally.
        std::map<Ogre::String, Signal> mSignalsMap;
        
        /// mSignalsMap's reverse mapping, for debug purposes.
        std::map<Signal, Ogre::String> mInverseSignalsMap;

        /// Instances to notify of emitted signals.
        std::map<Signal, std::set<SignalListener *>> mListeners;

        /// Emitted signals
        std::set<std::pair<Signal, SignalEmitter *>> mEmittedSignals;
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
