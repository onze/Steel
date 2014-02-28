#include "json/json.h"
#include <type_traits>

#include "InputSystem/Action.h"


#include <InputSystem/InputBuffer.h>
#include <SignalManager.h>
#include <Debug.h>
#include <tests/UnitTestManager.h>
#include <tools/JsonUtils.h>

namespace Steel
{
    const Action Action::ANY(Action::Type::ANY);

    char const *const Action::AND_ATTRIBUTE = "$and";
    char const *const Action::OR_ATTRIBUTE = "$or";

    char const *const Action::MAX_DELAY_ATTRIBUTE = "$maxDelay";
    char const *const Action::MIN_DELAY_ATTRIBUTE = "$minDelay";

    char const *const Action::ANY_ATTRIBUTE = "$*";

    Duration Action::sDefaultTimeWindowAnd = 25;

    Action::Action(): mType(Action::Type::COMPOSITE),
        mSignal(INVALID_SIGNAL),
        mMaxDelay(DURATION_MAX), mMinDelay(DURATION_MIN),
        mSubActions()
    {
    }

    Action::Action(const Ogre::String &signal): mType(Action::Type::SIMPLE),
        mSignal(INVALID_SIGNAL),
        mMaxDelay(DURATION_MAX), mMinDelay(DURATION_MIN),
        mSubActions()
    {
        parseSignalString(signal);
    }

    Action::Action(char const *const signal): mType(Action::Type::INVALID_TYPE),
        mSignal(INVALID_SIGNAL),
        mMaxDelay(DURATION_MAX), mMinDelay(DURATION_MIN),
        mSubActions()
    {
        parseSignalString(signal);
    }

    void Action::parseSignalString(Ogre::String const &signal)
    {
        if(Ogre::StringUtil::startsWith(signal, "$"))
        {
            Ogre::String signal_(signal);

            if(signal == Action::AND_ATTRIBUTE)
                mType = Action::Type::AND;
            else if(signal == Action::OR_ATTRIBUTE)
                mType = Action::Type::OR;
            else if(signal == Action::ANY_ATTRIBUTE)
                mType = Action::Type::ANY;
        }
        else
        {
            mType = Action::Type::SIMPLE;
            mSignal = SignalManager::instance().toSignal(signal);
        }
    }

    Action::Action(Signal const &signal): mType(Action::Type::SIMPLE),
        mSignal(signal),
        mMaxDelay(DURATION_MAX), mMinDelay(DURATION_MIN),
        mSubActions()
    {
    }

    Action::Action(Action::Type type): mType(type),
        mSignal(INVALID_SIGNAL),
        mMaxDelay(DURATION_MAX), mMinDelay(DURATION_MIN),
        mSubActions()
    {
    }

    Action::~Action()
    {
    }

    bool Action::operator==(const Steel::Action &o) const
    {
        return o.mType == mType && o.mSignal == mSignal && o.mSubActions == mSubActions;
    }

    bool Action::operator!=(const Steel::Action &o) const
    {
        return !(o == *this);
    }

    bool Action::IsOfType(Action::Type const type) const
    {
        return 0 != (static_cast<const unsigned int>(type) & static_cast<const unsigned int>(mType));
    }

    Action &Action::pushSubAction(Action const &subAction)
    {
        if(!IsOfType(Action::Type::COMPOSITE))
            Debug::error("Action::pushSubAction(): Can't pushSubAction on a non-composite Action. Skipped.").endl().breakHere();
        else
            mSubActions.push_back(subAction);

        return *this;
    }

    Action &Action::maxDelay(Duration maxDelay)
    {
        mMaxDelay = maxDelay;
        return (*this);
    }

    Action &Action::minDelay(Duration minDelay)
    {
        mMinDelay = minDelay;
        return (*this);
    }

    std::set<Signal> Action::signals() const
    {
        static const Ogre::String intro = "Action::signals(): ";
        std::set<Signal> actionSignals;

        switch(mType)
        {
            case Action::Type::SIMPLE:
                actionSignals.insert(mSignal);
                break;

            case Action::Type::AND:
                for(Action const & sub : mSubActions)
                {
                    auto subSignals = sub.signals();
                    actionSignals.insert(subSignals.begin(), subSignals.end());
                }

                break;

            case Action::Type::META:
            case Action::Type::ANY:
                // none or all ?...
                break;

            case Action::Type::COMPOSITE:
            default:
                Debug::error(intro)("unknown action type ")((unsigned)mType).endl();
                break;
        }

        return actionSignals;
    }

    bool Action::resolve(std::list<SignalBufferEntry>::const_iterator const &it_cbegin,
                         std::list<SignalBufferEntry>::const_iterator &it_signal,
                         std::list<SignalBufferEntry>::const_iterator const &it_cend,
                         TimeStamp const now_tt
                        ) const
    {
        //         Debug::log(*this)(" resolves ").asSignalBuffer(signalsBuffer)(" from ").asSignalBufferEntry(*it_signal).endl();
        switch(mType)
        {
            case Action::Type::SIMPLE:
                while(it_signal->signal != mSignal && it_cend != it_signal)
                    ++it_signal;

                if(it_cend != it_signal)
                    return true;

                break;

            case Action::Type::AND:
            {
                // validates if all actions resolve
                bool allFound = true;
                std::list<SignalBufferEntry>::const_iterator it_first = it_cend;
                std::list<SignalBufferEntry>::const_iterator it_last = it_signal;

                for(auto const & sub : mSubActions)
                {
                    // all actions should resolve from the same starting position
                    auto it(it_signal);
                    allFound &= sub.resolve(it_signal, it, it_cend);

                    // action was found was valid
                    if(allFound)
                    {
                        //                         Debug::log("resolved sub ")(sub).endl();
                        // update first/last input
                        if(it->timestamp <= it_first->timestamp)
                            it_first = it;

                        if(it->timestamp >= it_last->timestamp)
                            it_last = it;

                        ++it;
                    }
                    else
                    {
                        //                         Debug::log("with false").endl();
                        return false;
                    }
                }


                // validate timeFrame
                Duration timeFrame = it_last->timestamp - it_first->timestamp;

                if(allFound && timeFrame < static_cast<Duration>(Action::sDefaultTimeWindowAnd))
                {
                    // merge back to it_signal
                    it_signal = it_last;
                    //                     Debug::log("with true").endl();
                    return true;
                }

                break;
            }

            case Action::Type::OR:

                // validates if any action resolve
                for(auto const & sub : mSubActions)
                {
                    // all actions should resolve from the same starting position
                    auto it(it_signal);

                    if(sub.resolve(it_signal, it, it_cend))
                    {
                        it_signal = it;
                        return true;
                    }
                }

                break;

            case Action::Type::META:
            {
                    // checking min delay
                    auto it_previous(it_signal);
                    --it_previous;
                    // if we have a next input, we check the delay against it. Otherwise, we take now.
                    Duration delay;

                    if(it_cend == it_signal)
                        delay = static_cast<Duration>(now_tt - it_previous->timestamp);
                    else
                        delay = static_cast<Duration>(it_signal->timestamp - it_previous->timestamp);

                    if(delay < mMinDelay)
                        return false;

                    // checking max delay: has it not been too long since last input to resolve ?
                    // easy case: there was no input before, so of course it has not been too long to pass to next input.
                    if(it_cbegin == it_signal)
                        return true;

                    // if the is no input after, compare with now.
                    if(it_cend == it_signal)
                        delay = static_cast<Duration>(now_tt - it_previous->timestamp);
                    else
                        delay = static_cast<Duration>(it_signal->timestamp - it_previous->timestamp);

                    if(delay > mMaxDelay)
                        return false;
                    
                    // META Actions don't consume input
                    it_signal = it_previous;
                    return true;
            }
            break;

            case Action::Type::ANY:
                if(it_cend != it_signal)
                    return true;

            case Action::Type::INVALID_TYPE:
            default:
                break;
        }

        //         Debug::log("with false").endl();
        return false;
    }

    void Action::toJson(Json::Value &value) const
    {
        value = Json::nullValue;

        switch(mType)
        {
            case Action::Type::SIMPLE:
                value = SignalManager::instance().fromSignal(mSignal).c_str();
                break;

            case Action::Type::OR:
            case Action::Type::AND:
            {
                Json::Value seq = Json::arrayValue;
                Json::Value subActionValue;

                for(Action const & subAction : mSubActions)
                {
                    subAction.toJson(subActionValue);
                    seq.append(subActionValue);
                }

                value[mType == Action::Type::OR ? Action::OR_ATTRIBUTE : Action::AND_ATTRIBUTE] = seq;
            }

            case Action::Type::COMPOSITE:
                break;

            case Action::Type::META:
                if(DURATION_MAX != mMaxDelay)
                    value[Action::MAX_DELAY_ATTRIBUTE] = JsonUtils::toJson(mMaxDelay);

                if(DURATION_MIN != mMinDelay)
                    value[Action::MIN_DELAY_ATTRIBUTE] = JsonUtils::toJson(mMinDelay);

                break;

            case Action::Type::ANY:
                value = Action::ANY_ATTRIBUTE;
                break;

            case Action::Type::INVALID_TYPE:
                value = Json::nullValue;
                break;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////
    // UNIT TESTS (registration done in the UnitTestManager)

    bool utest_Action(UnitTestExecutionContext const *context)
    {
#define INIT_SIGNALS Signal sA = SignalManager::instance().toSignal("A"); \
                             Signal sB = SignalManager::instance().toSignal("B"); \
                             Signal sC = SignalManager::instance().toSignal("C"); \
                             Signal sD = SignalManager::instance().toSignal("D"); \
                             Signal sE = SignalManager::instance().toSignal("E"); \
                             std::list<SignalBufferEntry > signalsBuffer; \
                             auto it_cbegin = signalsBuffer.cbegin(); \
                             auto it_cend = signalsBuffer.cend();

        // also avoids "unused variable" warnings
#define CLEANUP_SIGNALS STEEL_UNUSED(sA); \
                             STEEL_UNUSED(sB); \
                             STEEL_UNUSED(sC); \
                             STEEL_UNUSED(sD); \
                             STEEL_UNUSED(sE); \
                             STEEL_UNUSED(signalsBuffer);


        {
            char const *const value = "A";
            Signal const signal = SignalManager::instance().toSignal(value);
            Action a(value);
            Action b(signal);
            STEEL_UT_ASSERT(a == b && !(a != b), "[UT001] failed at signal translation");
        }

        {
            Action const and_ = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            Action const and_identical(and_);
            STEEL_UT_ASSERT(and_ == and_identical && !(and_ != and_identical), "[UT002] failed at copy ctor");
        }

        {
            Action const and_ = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            Action and_identical;
            and_identical = and_;
            STEEL_UT_ASSERT(and_ == and_identical && !(and_ != and_identical), "[UT003] failed at assignation");
        }

        {
            Action const and_ = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            Action const and_identical = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            Action const and_different = Action(Action::Type::AND).pushSubAction(Action("B")).pushSubAction(Action("B"));
            Action const or_ = Action(Action::Type::OR).pushSubAction(Action("A")).pushSubAction(Action("B"));
            STEEL_UT_ASSERT(and_ == and_identical && !(and_ != and_identical), "[UT004] failed recognizing equal $and Actions");
            STEEL_UT_ASSERT(and_ != and_different && !(and_ == and_different), "[UT005] failed differentiating dfferent $and Actions");
            STEEL_UT_ASSERT(and_ != or_ && !(and_ == or_), "[UT006] failed differentiating dfferent $and and $or Actions");
        }

        {
            Action a = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            Action b = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            STEEL_UT_ASSERT(a.signals() == b.signals(), "[UT007] Action::signals failed");
        }

        {
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a("A");
            STEEL_UT_ASSERT(a.resolve(it_cbegin, it_signal, it_cend), "[UT008] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a("A");
            STEEL_UT_ASSERT(!a.resolve(it_cbegin, it_signal, it_cend), "[UT009] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            ++it_signal; // points to sB
            Action a("A");
            STEEL_UT_ASSERT(!a.resolve(it_cbegin, it_signal, it_cend), "[UT010] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a("A");
            STEEL_UT_ASSERT(a.resolve(it_cbegin, it_signal, it_cend), "[UT011] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // valid use case
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::AND).pushSubAction(sA).pushSubAction(sB);
            STEEL_UT_ASSERT(a.resolve(it_cbegin, it_signal, it_cend), "[UT012] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // valid input with delay
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sC, TimeStamp() + (Action::sDefaultTimeWindowAnd / Duration(2))});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::AND).pushSubAction(sA).pushSubAction(sC);
            STEEL_UT_ASSERT(a.resolve(it_cbegin, it_signal, it_cend), "[UT013] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // invalid input: delay too long
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, 0});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp() + Action::sDefaultTimeWindowAnd});
            signalsBuffer.push_back(SignalBufferEntry{sC, TimeStamp() + (Action::sDefaultTimeWindowAnd * Duration(2))});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::AND).pushSubAction(sA).pushSubAction(sC);
            STEEL_UT_ASSERT(!a.resolve(it_cbegin, it_signal, it_cend), "[UT014] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // valid input, $or action
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp() + (Action::sDefaultTimeWindowAnd / Duration(2))});
            signalsBuffer.push_back(SignalBufferEntry{sC, TimeStamp() + Action::sDefaultTimeWindowAnd});
            signalsBuffer.push_back(SignalBufferEntry{sD, TimeStamp() + (Action::sDefaultTimeWindowAnd * Duration(2))});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::OR).pushSubAction(sA).pushSubAction(sD);
            STEEL_UT_ASSERT(a.resolve(it_cbegin, it_signal, it_cend), "[UT015] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // invalid input, $or action
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp() + (Action::sDefaultTimeWindowAnd / Duration(2))});
            signalsBuffer.push_back(SignalBufferEntry{sD, TimeStamp() + (Action::sDefaultTimeWindowAnd * Duration(2))});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::OR).pushSubAction(sC).pushSubAction(sE);
            STEEL_UT_ASSERT(!a.resolve(it_cbegin, it_signal, it_cend), "[UT016] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // invalid input, embedded $or action
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sC, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            //  {or:[{and:[A,B]}, {and:[C,D]}]}
            Action a = Action(Action::Type::OR).pushSubAction(Action(Action::Type::AND).pushSubAction(sA).pushSubAction(sB)).pushSubAction(Action(Action::Type::AND).pushSubAction(sC).pushSubAction(sD));
            STEEL_UT_ASSERT(!a.resolve(it_cbegin, it_signal, it_cend), "[UT017] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            // ok
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp()});
            STEEL_UT_ASSERT(a.resolve(it_cbegin, it_signal, it_cend), "[UT018] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            // too late
            signalsBuffer.push_back(SignalBufferEntry{sD, TimeStamp() + (Action::sDefaultTimeWindowAnd * Duration(2))});
            STEEL_UT_ASSERT(!a.resolve(it_cbegin, it_signal, it_cend), "[UT019] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

        {
            // emtpy input, $or action
            INIT_SIGNALS;
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::OR).pushSubAction(sA).pushSubAction(sD);
            STEEL_UT_ASSERT(!a.resolve(it_cbegin, it_signal, it_cend), "[UT020] Action::resolve() failed with action ", a, " and input ", signalsBuffer);

            CLEANUP_SIGNALS;
        }

#undef INIT_SIGNALS
#undef CLEANUP_SIGNALS
        return true;
    }
}
