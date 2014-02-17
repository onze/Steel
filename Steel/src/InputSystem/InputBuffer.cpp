
#include "InputSystem/InputBuffer.h"

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
            ++(mSignalsListened.emplace(signal, 0).first->second);

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
                        mSignalsListened.erase(it_signal);
                    else
                        --(it_signal->second);
                }
            }

            mCombos.erase(it_combo);
        }
    }

    void InputBuffer::onSignal(Signal signal, SignalEmitter */*src*/)
    {
        // split per input controller here
        mSignalsBatch.push_back(std::make_pair(signal, mTimer.getMilliseconds()));
    }

    void InputBuffer::update()
    {
        //double delta = ((double)mTimer.getMicroseconds())/1000.;
        // dispatch actions
        if(mSignalsBatch.size())
        {
            // remove old input
            const long unsigned int thresholdTimestamp = mTimer.getMilliseconds() - mInputLifeDuration;
            auto it = mSignalsBuffer.begin();

            while(it->second > thresholdTimestamp)
                ++it;

            mSignalsBuffer.erase(mSignalsBuffer.begin(), it);

            // add new input
            mSignalsBuffer.insert(mSignalsBuffer.end(), mSignalsBatch.begin(), mSignalsBatch.end());
        }

        for(auto & combo : mCombos)
            if(combo.evaluate(mSignalsBuffer))
                SignalManager::instance().emit(combo.signal());
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // UNIT TESTS (registration done in the UnitTestManager)

    bool utest_InputBufferMain(UnitTestExecutionContext const* context)
    {
        // init
        InputBuffer ib;
        ib.init();
        
        
        ib.registerActionCombo(Steel::ActionCombo("__COMBO_fireHook")
        .push_back(Steel::Action("AttackBegin"))
        .push_back(Steel::Action("$startTimer"))
        .push_back(Steel::Action("AttackEnd"))
        );
        return true;
    }

}
