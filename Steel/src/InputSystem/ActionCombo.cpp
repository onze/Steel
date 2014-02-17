
#include "InputSystem/ActionCombo.h"
#include <SignalManager.h>

namespace Steel
{
    Duration ActionCombo::sMaxInputInterval = 100;

    ActionCombo::ActionCombo(Ogre::String const &signal): mSignal(SignalManager::instance().toSignal(signal)), mActions()
    {
    }

    ActionCombo::ActionCombo(Signal signal): mSignal(signal), mActions()
    {
    }

    ActionCombo::ActionCombo(ActionCombo const &o): mSignal(o.mSignal), mActions(o.mActions)
    {
    }

    ActionCombo::~ActionCombo()
    {
    }

    ActionCombo &ActionCombo::operator=(ActionCombo const &o)
    {
        if(&o != this)
        {
            mSignal = o.mSignal;
            mActions.clear();
            mActions.insert(mActions.begin(), o.mActions.begin(), o.mActions.end());
        }

        return *this;
    }

    bool ActionCombo::operator==(ActionCombo const &o) const
    {
        return o.mSignal == mSignal && o.mActions == mActions;
    }

    unsigned int ActionCombo::size() const
    {
        return mActions.size();
    }

    ActionCombo &ActionCombo::push_back(const Action &action)
    {
        mActions.push_back(action);
        return *this;
    }

    std::set<Signal> ActionCombo::signalsInvolved() const
    {
        std::set<Signal> signals;

        for(Action const & action : mActions)
        {
            // most actions use a signal, but compound actions might use more.
            auto const actionSignals = action.signals();
            signals.insert(actionSignals.cbegin(), actionSignals.cend());
        }

        return signals;
    }

    bool ActionCombo::evaluate(std::list< std::pair< Signal, TimeStamp > > const& mSignalsBuffer)
    {
        auto it_prev = mSignalsBuffer.cbegin();
        auto it_current = it_prev;

        for(Action const& action : mActions)
        {
            // check max delay between inputs
            if(mSignalsBuffer.cbegin() != it_current && static_cast<decltype(sMaxInputInterval)>(it_current->second - it_prev->second) > sMaxInputInterval)
                return false;

            if(!action.resolve(it_current, mSignalsBuffer))
                return false;

            it_prev = it_current;
        }

        return true;
    }


}
