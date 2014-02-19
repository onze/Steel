
#include "InputSystem/InputBuffer.h"
#include <InputSystem/ActionCombo.h>
#include <InputSystem/Action.h>
#include <tests/UnitTestManager.h>
#include <tools/File.h>
#include <Debug.h>
#include <SignalManager.h>

namespace Steel
{
    InputBuffer::InputBuffer(): SignalListener(), SignalEmitter(),
        mTimer(), mSignalsBatch(), mSignalsBuffer(), mInputLifeDuration(1000), mCombos(), mSignalsListened()
    {
    }

    InputBuffer::~InputBuffer()
    {
    }

    void InputBuffer::init()
    {
        // TODO: read from config
        mInputLifeDuration = 1000;
        mTimer.reset();
        mSignalsBatch.clear();
        mSignalsBuffer.clear();
        mCombos.clear();
        mSignalsListened.clear();
    }

    void InputBuffer::shutdown()
    {
    }

    bool InputBuffer::loadCombosFile(File /*file*/)
    {
        //TODO: write me !
        Debug::error("not implemented").breakHere();
        return true;
    }

    void InputBuffer::registerActionCombo(ActionCombo const &combo)
    {
        for(auto const signal : combo.signalsInvolved())
        {
            ++(mSignalsListened.emplace(signal, 0).first->second);
            registerSignal(signal);
        }

        mCombos.push_back(combo);
    }

    void InputBuffer::unregisterActionCombo(ActionCombo const &combo)
    {
        auto it_combo = std::find(mCombos.begin(), mCombos.end(), combo);

        if(mCombos.end() != it_combo)
        {
            for(auto const & signal : combo.signalsInvolved())
            {
                auto it_signal = mSignalsListened.find(signal);

                if(mSignalsListened.end() != it_signal)
                {
                    if(1 == it_signal->second)
                    {
                        mSignalsListened.erase(it_signal);
                        unregisterSignal(signal);
                    }
                    else
                        --(it_signal->second);
                }
            }

            mCombos.erase(it_combo);
        }
    }

    void InputBuffer::onSignal(Signal signal, SignalEmitter *const /*src*/)
    {
        // split per input controller here
        mSignalsBatch.push_back(SignalBufferEntry{signal, mTimer.getMilliseconds()});
    }

    void InputBuffer::update()
    {
        //double delta = ((double)mTimer.getMicroseconds())/1000.;
        // dispatch actions
        if(mSignalsBatch.size())
        {
            // remove old input
            const long unsigned int thresholdTimestamp = mTimer.getMilliseconds() - mInputLifeDuration;

            while(mSignalsBuffer.begin() != mSignalsBuffer.end() && mSignalsBuffer.begin()->timestamp > thresholdTimestamp)
                mSignalsBuffer.erase(mSignalsBuffer.begin());

            // add new input
            mSignalsBuffer.insert(mSignalsBuffer.end(), mSignalsBatch.begin(), mSignalsBatch.end());
        }

        for(auto & combo : mCombos)
            if(combo.evaluate(mSignalsBuffer))
                SignalManager::instance().emit(combo.signal());
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // UNIT TESTS (registration done in the UnitTestManager)

    bool utest_InputBufferMain(UnitTestExecutionContext const *context)
    {
#define INIT InputBuffer ib;\
        ib.init(); \
        SignalListenerMock listener;

#define CLEANUP ib.shutdown(); \
        listener.clearSignals();

        {
            INIT;
            const Signal signal_A = SignalManager::instance().toSignal("signal_A");
            listener.registerSignal(signal_A);
            ib.update();

            if(listener.hasReceived(signal_A))
            {
                Debug::error("[UT001] signal listener received signal it was not listening to.").endl().breakHere();
                return false;
            }

            CLEANUP;
        }

        {
            INIT;
            const Signal signal_A = SignalManager::instance().toSignal("signal_A");
            const Signal combo_output = SignalManager::instance().toSignal("combo_output");
            listener.registerSignal(combo_output);
            ActionCombo combo = ActionCombo(combo_output).push_back(Action(signal_A));
            ib.registerActionCombo(combo);
            SignalManager::instance().fire(signal_A);
            ib.update();
            SignalManager::instance().fireEmittedSignals();

            if(!listener.hasOnlyReceived(combo_output))
            {
                Debug::error("[UT002] failed to resolve/deliver ActionCombo(signal_output).push_back(Action(combo_output)).").endl().breakHere();
                return false;
            }

            CLEANUP;
        }

        {
            INIT;
            const Signal signal_A = SignalManager::instance().toSignal("signal_A");
            const Signal signal_B = SignalManager::instance().toSignal("signal_B");
            const Signal combo_output = SignalManager::instance().toSignal("combo_output");
            listener.registerSignal(combo_output);
            ActionCombo combo = ActionCombo(combo_output).push_back(Action(signal_A)).push_back(Action(signal_B));
            ib.registerActionCombo(combo);
            SignalManager::instance().fire(signal_B).fire(signal_A); // wrong ordered combo: invalid
            ib.update();
            SignalManager::instance().fireEmittedSignals();

            if(listener.hasReceived(combo_output))
            {
                Debug::error("[UT003] resolved incomplete ActionCombo ").quotes(combo).endl().breakHere();
                return false;
            }

            CLEANUP;
        }

        {
            INIT;
            const Signal signal_A = SignalManager::instance().toSignal("signal_A");
            const Signal signal_B = SignalManager::instance().toSignal("signal_B");
            const Signal combo_output = SignalManager::instance().toSignal("combo_output");
            listener.registerSignal(combo_output);
            ActionCombo combo = ActionCombo(combo_output).push_back(Action(signal_A)).push_back(Action(signal_B));
            ib.registerActionCombo(combo);
            SignalManager::instance().fire(signal_A).fire(signal_B); // right ordered combo: valid
            ib.update();
            SignalManager::instance().fireEmittedSignals();

            if(!listener.hasOnlyReceived(combo_output))
            {
                Debug::error("[UT004] unresolved valid ActionCombo ").quotes(combo).endl().breakHere();
                return false;
            }

            CLEANUP;
        }
#undef INIT
#undef CLEANUP
        return true;
    }

}
