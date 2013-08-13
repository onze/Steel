#ifndef STEEL_SIGNALEMITTER_H
#define STEEL_SIGNALEMITTER_H

#include <OgreString.h>

#include "steeltypes.h"
#include "SignalManager.h"
namespace Steel
{
    class SignalEmitter
    {
        public:
            inline void emit(const Signal signal, bool anonymous = false)
            {
                SignalManager::instance()->emit(signal, anonymous ? NULL : this);
            }

            inline void emit(const Ogre::String& signal, bool anonymous = false)
            {
                SignalManager::instance()->emit(signal, anonymous ? NULL : this);
            }
    };
}

#endif // STEEL_SIGNALEMITTER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
