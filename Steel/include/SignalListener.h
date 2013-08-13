#ifndef STEEL_SIGNALLISTENER_H
#define STEEL_SIGNALLISTENER_H

#include <vector>
#include <OgreString.h>

#include "steeltypes.h"

namespace Steel
{
    class SignalEmitter;

    class SignalListener
    {
        public:
            SignalListener();
            virtual ~SignalListener();

            /// Registers onSignal to be called when the given signal is fired.
            void registerSignal(const Ogre::String &signal);
            /// Registers onSignal to be called when the given signal is fired.
            void registerSignal(const Signal signal);
            
            /// Unregisters calls to onSignal for the given signal.
            void unregisterSignal(Signal signal);

            /// Triggered when the listened signal is fired.
            virtual void onSignal(Signal signal, SignalEmitter* src)=0;
            
            /** 
             * Triggered when the listened signal is fired. Calls onSignal and bound methods.
             * Subclasses most likely want to overwrite onSignal (no leading underscrore).
             */
            void _onSignal(Signal signal, SignalEmitter* src);
        private:
            /// Signals the instance is currently listening to.
            std::vector<Signal> mRegisteredSignals;

#ifdef DEBUG
            mutable std::vector<Ogre::String> mRegisteredSignalStrings;
#endif
    };
}

#endif // STEEL_SIGNALLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
