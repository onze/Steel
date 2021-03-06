#ifndef STEEL_INPUTBUFFER_H
#define STEEL_INPUTBUFFER_H

#include <GLX/OgreTimerImp.h>

#include "SignalListener.h"
#include "SignalEmitter.h"
#include "ActionCombo.h"

namespace Steel
{
    class Level;
    class UnitTestExecutionContext;
    class File;

    struct SignalBufferEntry
    {
        Signal signal;
        TimeStamp timestamp;
    };

    /**
     * Handles Combos, that is more or less complex filters on combinations of actions (generated by the inputManager)
     * to emit other actions.
     * It can be viewed a generic predicate runner over steel signals. It can completely be abused for stuff outside of controller input,
     * like missions/objectives/whatnot.
     * A combo "evaluates" when the actions it contains all resolve on the current input. A combo's signal is emitted whenever the combo
     * switches from its "non-evaluating" state to its "evaluating" state.
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
        
        /// Make the inputBuffer receive the given signal. Used to register the input map.
        void registerAction(Signal signal);
        void unregisterAction(Signal signal);

        /// Registers the combo for evaluation upon input. Returns the signal emitted when the combo evaluates.
        Signal registerActionCombo(ActionCombo const &combo);
        void unregisterActionCombo(ActionCombo const &combo);

        // SignalListener interface
        virtual void onSignal(Signal signal, SignalEmitter *const src = nullptr);
        
        bool isEvaluating(ActionCombo const& combo) const;
        
        // getters
        Duration inputLifeDuration() const {return mInputLifeDuration;}

    private:
        // not owned

        // owned
        Ogre::Timer mTimer;

        /// Signals are batched during a frame and process on update
        std::vector<SignalBufferEntry> mSignalsBatch;

        /// Signals against which combos are evaluated each frame. FIFO.
        std::list<SignalBufferEntry> mSignalsBuffer;

        /// Time an input is alive in the buffer, in milliseconds
        Duration mInputLifeDuration;

        class ActionComboEntry
        {
        public:
            // helper
            ActionComboEntry(ActionCombo const &_combo, unsigned _refCount);
            
            /// Combo.
            ActionCombo combo;
            
            /// How many of such combo are being registered
            unsigned refCount;
        };
        std::map<Hash, ActionComboEntry> mCombos;


        /// Signals listened to.
        std::map<Signal, RefCount> mSignalsListened;

    };

    bool utest_InputBufferMain(UnitTestExecutionContext const *context);
}

#endif // STEEL_INPUTBUFFER_H
