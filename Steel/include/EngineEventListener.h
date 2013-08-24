#ifndef STEEL_ENGINEEVENTLISTENER_H
#define STEEL_ENGINEEVENTLISTENER_H

namespace Steel
{
    class Level;

    class EngineEventListener
    {
        public:

            /// called when a new level becomes the current level.
            virtual void onLevelSet(Level *level)=0;

            /// called right before a level is unset (becomes not current anymore).
            virtual void onLevelUnset(Level *level)=0;
    };
}

#endif // STEEL_ENGINEEVENTLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
