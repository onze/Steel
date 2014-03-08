
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
        mTimer(), mSignalsBatch(), mSignalsBuffer(), mInputLifeDuration(3000),
        mCombos(),
        mSignalsListened()
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

    void InputBuffer::registerAction(Signal signal)
    {
        registerSignal(signal);
    }

    void InputBuffer::unregisterAction(Signal signal)
    {
        unregisterSignal(signal);
    }

    Signal InputBuffer::registerActionCombo(ActionCombo const &combo)
    {
        auto it = mCombos.find(combo.hash());

        // if combo is seen for the first time
        if(mCombos.end() == it)
        {
            for(auto const signal : combo.signalsInvolved())
            {
                ++(mSignalsListened.emplace(signal, 0).first->second);
                registerSignal(signal);
            }

            mCombos.insert(std::make_pair(combo.hash(), ActionComboEntry(combo, 1)));
        }
        else
        {
            ++(it->second.refCount);
        }

        return combo.signal();
    }

    void InputBuffer::unregisterActionCombo(ActionCombo const &combo)
    {
        auto it_combo = mCombos.find(combo.hash());

        if(mCombos.end() != it_combo)
        {
            --(it_combo->second.refCount);

            if(0 == it_combo->second.refCount)
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
    }

    void InputBuffer::onSignal(Signal signal, SignalEmitter *const /*src*/)
    {
        // split per input controller here
        SignalBufferEntry entry = {signal, mTimer.getMilliseconds()};
        Debug::log("InputBuffer::onSignal(): ")(entry).endl();
        mSignalsBatch.push_back(entry);
    }

    void InputBuffer::update()
    {
        static const Ogre::String intro = "InputBuffer::update(): ";
        bool debug = true;

        // dispatch actions
        TimeStamp now_tt(mTimer.getMilliseconds());
        const TimeStamp thresholdTimestamp = now_tt - (TimeStamp)mInputLifeDuration;

        while(mSignalsBuffer.size() > 0 && mSignalsBuffer.front().timestamp < thresholdTimestamp)
        {
//             if(debug)
//                 Debug::log("removing outdated (>")(mInputLifeDuration)(") input action ")(mSignalsBuffer.front()).endl();

            mSignalsBuffer.pop_front();
        }

        // add new input
        if(mSignalsBatch.size())
        {
            mSignalsBuffer.insert(mSignalsBuffer.end(), mSignalsBatch.begin(), mSignalsBatch.end());
            mSignalsBatch.clear();
        }

//         if(debug && mSignalsBuffer.size())
//             Debug::log("post mSignalsBuffer: ")(mSignalsBuffer).endl();

        for(auto & entry : mCombos)
        {
            ActionComboEntry &acEntry = entry.second;
            bool flag = acEntry.combo.evaluate(mSignalsBuffer, now_tt);

            if(flag)
            {
                if(debug)
                    Debug::log("emitting combo ")(acEntry.combo).endl();

                SignalManager::instance().emit(acEntry.combo.signal());
            }
        }
    }

    bool InputBuffer::isEvaluating(const ActionCombo &combo) const
    {
        auto it = mCombos.find(combo.hash());
        return mCombos.cend() == it ? false : it->second.combo.policy().isEvaluating();
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

            STEEL_UT_ASSERT(!listener.hasReceived(signal_A), "[UT001] signal listener received signal it was not listening to.");

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

            STEEL_UT_ASSERT(listener.hasOnlyReceived(combo_output), "[UT002] failed to resolve/deliver ActionCombo(signal_output).push_back(Action(combo_output)).");

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

            STEEL_UT_ASSERT(!listener.hasReceived(combo_output), "[UT003] resolved incomplete ActionCombo \"", combo, "\"");

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

            STEEL_UT_ASSERT(listener.hasOnlyReceived(combo_output), "[UT004] unresolved valid ActionCombo \"", combo, "\"");

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

            STEEL_UT_ASSERT(listener.hasOnlyReceived(combo_output), "[UT004] unresolved valid ActionCombo \"", combo, "\"");

            CLEANUP;
        }
#undef INIT
#undef CLEANUP
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////

    InputBuffer::ActionComboEntry::ActionComboEntry(ActionCombo const &_combo, unsigned _refCount)
        : combo(_combo), refCount(_refCount)
    {
    }

}
