#ifndef STEEL_ENGINEEVENTLISTENER_H
#define STEEL_ENGINEEVENTLISTENER_H

namespace Steel
{
    class Level;

    class EngineEventListener
    {
        public:
            EngineEventListener()
            {
            }
            ;
            virtual ~EngineEventListener()
            {
            }
            ;
            /// called when a new level becomes the current level.
            virtual void onLevelSet(Level *level)
            {
            }
            ;
            /// called right before a level is unset (becomes not current anymore).
            virtual void onLevelUnset(Level *level)
            {
            }
            ;
    };
}

#endif // STEEL_ENGINEEVENTLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
