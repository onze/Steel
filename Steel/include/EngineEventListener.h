#ifndef STEEL_ENGINEEVENTLISTENER_H
#define STEEL_ENGINEEVENTLISTENER_H

#include <OISKeyboard.h>
#include <OISMouse.h>

namespace Steel
{
    class Level;

    class EngineEventListener
    {
    public:

        /// Called when a new level becomes the current level.
        virtual void onLevelSet(Level *level) {};

        /// Called right before a level is unset (becomes not current anymore).
        virtual void onLevelUnset(Level *level) {};

        /// Called right before the main level is updated. Right moment to inject additional input.
        virtual void onBeforeLevelUpdate(Level *level, float dt) {};

        /// Called right after the main level is updated. Last step before next frame is the graphic update.
        virtual void onAfterLevelUpdate(Level *level) {};

        /// Called right after the engine enters edit mode.
        virtual void onStartEditMode() {};

        /// Called right after the engine leaves edit mode.
        virtual void onStopEditMode() {};
    };
}

#endif // STEEL_ENGINEEVENTLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
