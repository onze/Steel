
#include "InputSystem/ActionCombo.h"
#include "InputSystem/InputBuffer.h"
#include "InputSystem/Action.h"
#include "SignalManager.h"
#include "Debug.h"
#include "tools/JsonUtils.h"
#include "tests/UnitTestManager.h"
#include "tools/StdUtils.h"

namespace Steel
{
    const char *ActionCombo::SEQUENCE_ATTRIBUTE = "sequence";
    const char *ActionCombo::SIGNAL_ATTRIBUTE = "signal";

    Duration ActionCombo::sDefaultTimeGap = DURATION_MAX;

    ActionCombo::ActionCombo(): mSignal(INVALID_SIGNAL), mActions(), mPolicy()
    {
    }

    ActionCombo::ActionCombo(Ogre::String const &signal): mSignal(SignalManager::instance().toSignal(signal)), mActions(), mPolicy()
    {
    }

    ActionCombo::ActionCombo(Signal const signal): mSignal(signal), mActions(), mPolicy()
    {
    }

    ActionCombo::ActionCombo(ActionCombo const &o): mSignal(o.mSignal), mActions(o.mActions), mPolicy(o.mPolicy)
    {
    }

    ActionCombo::~ActionCombo()
    {
    }

    Hash ActionCombo::hash() const
    {
        return std::hash<ActionCombo>()(*this);
    }

    ActionCombo &ActionCombo::operator=(ActionCombo const &o)
    {
        if(&o != this)
        {
            mSignal = o.mSignal;
            mActions.clear();
            mActions.insert(mActions.begin(), o.mActions.begin(), o.mActions.end());
            mPolicy = o.mPolicy;
        }

        return *this;
    }

    bool ActionCombo::operator==(ActionCombo const &o) const
    {
        return o.mSignal == mSignal && o.mActions == mActions && o.mPolicy == mPolicy;
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

    void ActionCombo::toJson(Json::Value &value) const
    {
        static const Ogre::String intro = "ActionCombo::toJson(): ";

        if(mActions.size())
        {
            Json::Value seq = Json::arrayValue;

            // actions
            Json::Value actionValue;

            for(Action const & action : mActions)
            {
                action.toJson(actionValue);
                seq.append(actionValue);
            }

            value[ActionCombo::SEQUENCE_ATTRIBUTE] = seq;
        }

        if(INVALID_SIGNAL != mSignal)
            value[ActionCombo::SIGNAL_ATTRIBUTE] = JsonUtils::toJson(SignalManager::instance().fromSignal(mSignal));

//         if(DURATION_MIN != mRecoverDuration)
        //             value[ActionCombo::EVALUATION_POLICY_ATTRIBUTE] = JsonUtils::toJson(mPolicy);
//         if(DURATION_MIN != mRecoverDuration)
//             value[ActionCombo::RECOVERDURATION_ATTRIBUTE] = JsonUtils::toJson(mRecoverDuration);
    }

    bool ActionCombo::evaluate(std::list<SignalBufferEntry> const &signalsBuffer, TimeStamp now_tt) const
    {
        bool allResolved = false;

        // for each signal in the input
        for(std::list<SignalBufferEntry>::const_iterator it = signalsBuffer.cbegin(); signalsBuffer.cend() != it; ++it)
        {
            auto it_prev = it;
            auto it_current = it;
            allResolved = true;

            for(Action const & action : mActions)
            {
                // check max delay between inputs
                if(it != it_current && static_cast<decltype(sDefaultTimeGap)>(it_current->timestamp - it_prev->timestamp) > sDefaultTimeGap)
                {
                    allResolved = false;
                    break;
                }

                if(!action.resolve(it, it_current, signalsBuffer.cend(), now_tt))
                {
                    allResolved = false;
                    break;
                }

                if(signalsBuffer.cend() != it_current)
                    it_prev = it_current++;
            }

            if(allResolved)
                break;
        }

        return mPolicy.evaluate(now_tt, allResolved);
     }

    ActionCombo &ActionCombo::setPolicy(EvaluationPolicy policy)
    {
        mPolicy = policy;
        return *this;
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // UNIT TESTS (registration done in the UnitTestManager)

    bool utest_ActionCombo(UnitTestExecutionContext const *context)
    {
        {
            ActionCombo a("ActionCombo_output"), b("ActionCombo_output");

            if(a != b)
            {
                Debug::error("[UT001] failed at comparison").endl().breakHere();
                return false;
            }

            a.push_back("A");
            STEEL_UT_ASSERT(a != b, "[UT002] failed at comparison");

            b.push_back("A");
            STEEL_UT_ASSERT(a == b && !(a != b), "[UT003] failed at comparison");

            a.push_back("B").push_back("C");
            b.push_back("B").push_back("C");
            STEEL_UT_ASSERT(a == b && !(a != b), "[UT004] failed at comparison");
        }

        {
            STEEL_UT_ASSERT(ActionCombo("combo").signalsInvolved().size() == 0, "[UT004] ActionCombo::signalsInvolved failed.");
            STEEL_UT_ASSERT(ActionCombo("combo").push_back("A").signalsInvolved().size() == 1, "[UT005] ActionCombo::signalsInvolved failed.");
            STEEL_UT_ASSERT(ActionCombo("combo").push_back("A").push_back("B").signalsInvolved().size() == 2, "[UT006] ActionCombo::signalsInvolved failed.");
            STEEL_UT_ASSERT(ActionCombo("combo").push_back("A").push_back("B").push_back("B").signalsInvolved().size() == 2, "[UT007] ActionCombo::signalsInvolved failed.");
            STEEL_UT_ASSERT(ActionCombo("combo").push_back("A").push_back("B").push_back("B").push_back(Action(Action::Type::AND).pushSubAction("and.A").pushSubAction("and.B")).signalsInvolved().size() == 4, "[UT008] ActionCombo::signalsInvolved failed.");
        }

#define INIT_SIGNALS Signal sA = SignalManager::instance().toSignal("A"); \
        Signal sB = SignalManager::instance().toSignal("B"); \
        Signal sC = SignalManager::instance().toSignal("C"); \
        Signal sD = SignalManager::instance().toSignal("D"); \
        Signal sE = SignalManager::instance().toSignal("E"); \
        std::list<SignalBufferEntry > signalsBuffer;

        // also avoids "unused variable" warnings
#define CLEANUP_SIGNALS STEEL_UNUSED(sA); \
        STEEL_UNUSED(sB); \
        STEEL_UNUSED(sC); \
        STEEL_UNUSED(sD); \
        STEEL_UNUSED(sE); \
        STEEL_UNUSED(signalsBuffer);

        {
            // input invalid because of TimeGap too short
            INIT_SIGNALS;
            Duration const delay = 250;
            auto combo = ActionCombo("Attack-hold-small").push_back(sA).push_back(Action(Action::Type::META).minDelay(delay)).push_back(sB);
            signalsBuffer.push_back({sA, TimeStamp(0)});
            signalsBuffer.push_back({sB, TimeStamp(delay / 2)});

            STEEL_UT_ASSERT(!combo.evaluate(signalsBuffer), "[UT009] failed ActionCombo::evaluate(): combo ", combo, ", input ", signalsBuffer);

            signalsBuffer.push_back({sB, TimeStamp(delay * 2)}); // now the minDelay has passed, but there's the first, extra, B in the way
            STEEL_UT_ASSERT(!combo.evaluate(signalsBuffer), "[UT010] failed ActionCombo::evaluate(): combo ", combo, ", input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // input invalid because of TimeGap too short
            INIT_SIGNALS;
            Duration const delay = 250;
            auto combo = ActionCombo("Attack-hold-small").push_back(sA).push_back(Action(Action::Type::META).minDelay(delay)).push_back(sB);
            signalsBuffer.push_back({sA, TimeStamp(0)}); // ok
            signalsBuffer.push_back({sB, TimeStamp(delay / 2)}); // fail
            signalsBuffer.push_back({sA, TimeStamp(0)}); // ok
            signalsBuffer.push_back({sB, TimeStamp(delay * 2)}); // now the minDelay has passed, and the but there's the first, extra, B in the way
            STEEL_UT_ASSERT(combo.evaluate(signalsBuffer), "[UT011] failed ActionCombo::evaluate(): combo ", combo, ", input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // validates a meta cannot be first
            INIT_SIGNALS;
            Duration const delay = 250;
            auto combo = ActionCombo("Attack-hold-small").push_back(Action(Action::Type::META).maxDelay(delay)).push_back(sA);
            signalsBuffer.push_back({sA, TimeStamp(0)});
            STEEL_UT_ASSERT(!combo.evaluate(signalsBuffer), "[UT013] failed ActionCombo::evaluate(): combo ", combo, ", input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // validates a meta can be last
            INIT_SIGNALS;
            Duration const delay = 250;
            auto combo = ActionCombo("Attack-hold-small").push_back(sA).push_back(Action(Action::Type::META).maxDelay(delay));
            signalsBuffer.push_back({sA, TimeStamp(0)});
            STEEL_UT_ASSERT(combo.evaluate(signalsBuffer), "[UT014] failed ActionCombo::evaluate(): combo ", combo, ", input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // validate META/maxDelay filtering invalid input
            INIT_SIGNALS;
            Duration const delay = 250;
            auto combo = ActionCombo("Attack-hold-small").push_back(sA).push_back(Action(Action::Type::META).maxDelay(delay)).push_back(Action::Type::ANY);
            signalsBuffer.push_back({sA, TimeStamp(0)}); // ok
            signalsBuffer.push_back({sB, TimeStamp(delay * 2)}); // too late
            STEEL_UT_ASSERT(!combo.evaluate(signalsBuffer), "[UT015] failed ActionCombo::evaluate(): combo ", combo, ", input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // validate META/maxDelay not filtering valid input
            INIT_SIGNALS;
            Duration const delay = 250;
            auto combo = ActionCombo("Attack-hold-small")
            .push_back(sA)
            .push_back(Action(Action::Type::META).maxDelay(delay))
            .push_back(Action::Type::ANY);
            signalsBuffer.push_back({sA, TimeStamp(0)});
            signalsBuffer.push_back({sA, TimeStamp(delay / 2)});
            STEEL_UT_ASSERT(combo.evaluate(signalsBuffer), "[UT016] failed ActionCombo::evaluate(): combo ", combo, ", input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // validate ANY validates on any input
            INIT_SIGNALS;
            auto combo = ActionCombo("Attack-hold-small").push_back(Action::Type::ANY);
            signalsBuffer.push_back({sA, TimeStamp(0)});
            STEEL_UT_ASSERT(combo.evaluate(signalsBuffer), "[UT017] failed ActionCombo::evaluate(): combo ", combo, ", input ", signalsBuffer);

            combo.push_back(Action::Type::ANY); // double ANY should not evaluate true on single input
            STEEL_UT_ASSERT(!combo.evaluate(signalsBuffer), "[UT018] failed ActionCombo::evaluate(): combo ", combo, ", input ", signalsBuffer);

            signalsBuffer.push_back({sB, TimeStamp(0)}); // but double ANY should evaluate on double input, etc
            STEEL_UT_ASSERT(combo.evaluate(signalsBuffer), "[UT019] failed ActionCombo::evaluate(): combo ", combo, ", input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }
        return true;
    }

}
