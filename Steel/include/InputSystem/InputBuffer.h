#ifndef STEEL_INPUTBUFFER_H
#define STEEL_INPUTBUFFER_H

#include <vector>
#include <list>

#include <GLX/OgreTimerImp.h>

#include "SignalListener.h"
#include "SignalEmitter.h"
#include "tools/File.h"
#include "ActionCombo.h"

namespace Steel
{

    class Level;
    class UnitTestExecutionContext;

    /**
     * Handles Combos, that is more or less complex filters on combinations of actions (generated by the inputManager)
     * to emit other actions.
     * This is a generic predicate runner over steel signals. It can completely be abused for stuff outside of controller input,
     * like missions/objectives/whatnot.
     */
    class InputBuffer : protected SignalListener, public SignalEmitter
    {
    public:
        InputBuffer();
        ~InputBuffer();

        void init();
        void shutdown();

        /// Resolves actions combos from received action signals.
        void update();
        bool loadCombosFile(File file);

        void registerActionCombo(ActionCombo const &combo);
        void unregisterActionCombo(ActionCombo const &combo);

        // SignalListener interface
        virtual void onSignal(Signal signal, SignalEmitter *const src = nullptr);

    private:
        // not owned

        // owned
        Ogre::Timer mTimer;

        /// Signals are batched during a frame and process on update
        std::vector<std::pair<Signal, TimeStamp>> mSignalsBatch;

        /// Signals against which combos are evaluated each frame. FIFO.
        std::list<std::pair<Signal, TimeStamp>> mSignalsBuffer;

        /// Time an input is alive in the buffer, in milliseconds
        long int mInputLifeDuration;

        std::vector<ActionCombo> mCombos;

        /// Signals listened to.
        std::map<Signal, RefCount> mSignalsListened;

    };

    bool utest_InputBufferMain(UnitTestExecutionContext const* context);
}

#endif // STEEL_INPUTBUFFER_H
