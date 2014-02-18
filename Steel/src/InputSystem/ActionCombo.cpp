
#include "InputSystem/ActionCombo.h"
#include <SignalManager.h>
#include <Debug.h>
#include <tools/JsonUtils.h>
#include <tests/UnitTestManager.h>

namespace Steel
{
    const char *ActionCombo::SEQUENCE_ATTRIBUTE = "sequence";
    const char *ActionCombo::SIGNAL_ATTRIBUTE = "signal";

    Duration ActionCombo::sMaxInputInterval = 100;

    ActionCombo::ActionCombo(Ogre::String const &signal): mSignal(SignalManager::instance().toSignal(signal)), mActions()
    {
    }

    ActionCombo::ActionCombo(Signal const signal): mSignal(signal), mActions()
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

    bool ActionCombo::operator!=(ActionCombo const &o) const
    {
        return !(o == *this);
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

    bool ActionCombo::evaluate(std::list< std::pair< Signal, TimeStamp > > const &mSignalsBuffer)
    {
        auto it_prev = mSignalsBuffer.cbegin();
        auto it_current = it_prev;

        for(Action const & action : mActions)
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


    void ActionCombo::toJson(Json::Value &value) const
    {
        static const Ogre::String intro = "ActionCombo::toJson(): ";
        Json::Value seq = value[ActionCombo::SEQUENCE_ATTRIBUTE] = Json::arrayValue;

        // actions
        Json::Value actionValue;

        for(Action const & action : mActions)
        {
            action.toJson(actionValue);
            seq.append(actionValue);
        }

        value[ActionCombo::SIGNAL_ATTRIBUTE] = JsonUtils::toJson(mSignal);
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // UNIT TESTS (registration done in the UnitTestManager)

    bool utest_ActionCombo(UnitTestExecutionContext const *context)
    {
#define INIT ActionCombo ac;
#define CLEANUP void;

        {
            ActionCombo a("ActionCombo_output"), b("ActionCombo_output");

            if(a != b)
            {
                Debug::error("[UT001] failed at comparison").endl().breakHere();
                return false;
            }

            a.push_back("A");

            if(a == b)
            {
                Debug::error("[UT002] failed at comparison").endl().breakHere();
                return false;
            }

            b.push_back("A");

            if(a != b || !(a == b))
            {
                Debug::error("[UT003] failed at comparison").endl().breakHere();
                return false;
            }

            a.push_back("B").push_back("C");
            b.push_back("B").push_back("C");

            if(a != b || !(a == b))
            {
                Debug::error("[UT004] failed at comparison").endl().breakHere();
                return false;
            }
        }

        {
            ActionCombo a = ActionCombo("A").push_back("B");
        }
        return true;
    }

}
