#ifndef STEEL_SIGNALEMITTER_H
#define STEEL_SIGNALEMITTER_H

#include "steeltypes.h"

namespace Steel
{
    class SignalEmitter
    {
    public:
        void emit(const Signal signal, bool anonymous = false);
        void emit(const Ogre::String &signal, bool anonymous = false);
    };
}

#endif // STEEL_SIGNALEMITTER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
