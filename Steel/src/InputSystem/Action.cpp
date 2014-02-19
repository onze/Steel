#include "json/json.h"

#include "InputSystem/Action.h"


#include <InputSystem/InputBuffer.h>
#include <SignalManager.h>
#include <Debug.h>
#include <tests/UnitTestManager.h>

namespace Steel
{
    char const *const Action::AND_ATTRIBUTE = "$and";
    char const *const Action::OR_ATTRIBUTE = "$or";

    Duration Action::sTimeFrameAnd = 25;

    Action::Action(): mType(Action::Type::COMPOSITE), mSignal(INVALID_SIGNAL), mSubActions()
    {
    }

    Action::Action(const Ogre::String &signal): mType(Action::Type::SIMPLE),
        mSignal(SignalManager::instance().toSignal(signal)), mSubActions()
    {
    }

    Action::Action(char const *const signal): mType(Action::Type::SIMPLE),
        mSignal(SignalManager::instance().toSignal(signal)), mSubActions()
    {
    }

    Action::Action(Signal const &signal): mType(Action::Type::SIMPLE),
        mSignal(signal), mSubActions()
    {
    }

    Action::Action(Action::Type type): mType(type),
        mSignal(INVALID_SIGNAL), mSubActions()
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

            case Action::Type::COMPOSITE:
            default:
                Debug::error(intro)("unknown composite action.").endl();
                break;
        }

        return actionSignals;
    }

    bool Action::resolve(std::list<SignalBufferEntry>::const_iterator &it_signal,
                         std::list<SignalBufferEntry> const &signalsBuffer) const
    {
//         Debug::log(*this)(" resolves ").asSignalBuffer(signalsBuffer)(" from ").asSignalBufferEntry(*it_signal).endl();
        switch(mType)
        {
            case Action::Type::SIMPLE:
                while(it_signal->signal != mSignal && signalsBuffer.end() != it_signal)
                    ++it_signal;

                if(signalsBuffer.end() != it_signal)
                    return true;

                break;

            case Action::Type::AND:
            {
                // easy bailout: input is not big enough
                if(signalsBuffer.size() < mSubActions.size())
                    return false;

                // validates if all actions resolve
                bool allFound = true;
                std::list<SignalBufferEntry>::const_iterator it_first = signalsBuffer.cend();
                std::list<SignalBufferEntry>::const_iterator it_last = signalsBuffer.cbegin();

                for(auto const & sub : mSubActions)
                {
                    // all actions should resolve from the same starting position
                    auto it(it_signal);
                    allFound &= sub.resolve(it, signalsBuffer);

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

                if(allFound && timeFrame < static_cast<Duration>(Action::sTimeFrameAnd))
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
                    auto it = it_signal;

                    if(sub.resolve(it, signalsBuffer))
                    {
                        it_signal = it;
                        return true;
                    }
                }

                break;

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
            break;

            default:
                Debug::error("Action::toJson(): invalid type.").endl().breakHere();
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
        std::list<SignalBufferEntry > signalsBuffer;

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

            if(a != b || !(a == b))
            {
                Debug::error("[UT001] failed at signal translation").endl().breakHere();
                return false;
            }
        }

        {
            Action const and_ = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            Action const and_identical(and_);

            if(and_ != and_identical || !(and_ == and_identical))
            {
                Debug::error("[UT002] failed at copy ctor").endl().breakHere();
                return false;
            }
        }

        {
            Action const and_ = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            Action and_identical;
            and_identical = and_;

            if(and_ != and_identical || !(and_ == and_identical))
            {
                Debug::error("[UT003] failed at assignation").endl().breakHere();
                return false;
            }
        }

        {
            Action const and_ = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            Action const and_identical = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            Action const and_different = Action(Action::Type::AND).pushSubAction(Action("B")).pushSubAction(Action("B"));
            Action const or_ = Action(Action::Type::OR).pushSubAction(Action("A")).pushSubAction(Action("B"));

            if(and_ != and_identical || !(and_ == and_identical))
            {
                Debug::error("[UT004] failed recognizing equal $and Actions").endl().breakHere();
                return false;
            }

            if(and_ == and_different || !(and_ != and_different))
            {
                Debug::error("[UT005] failed differentiating dfferent $and Actions").endl().breakHere();
                return false;
            }

            if(and_ == or_ || !(and_ != or_))
            {
                Debug::error("[UT006] failed differentiating dfferent $and and $or Actions").endl().breakHere();
                return false;
            }
        }

        {
            Action a = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));
            Action b = Action(Action::Type::AND).pushSubAction(Action("A")).pushSubAction(Action("B"));

            if(a.signals() != b.signals())
            {
                Debug::error("[UT007] Action::signals failed").endl().breakHere();
                return false;
            }
        }

        {
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a("A");

            if(!a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT008] failed Action::resolve").endl().breakHere();
                return false;
            }

            CLEANUP_SIGNALS;
        }

        {
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a("A");

            if(a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT009] failed Action::resolve").endl().breakHere();
                return false;
            }

            CLEANUP_SIGNALS;
        }

        {
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            ++it_signal; // points to sB
            Action a("A");

            if(a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT010] failed Action::resolve").endl().breakHere();
                return false;
            }

            CLEANUP_SIGNALS;
        }

        {
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a("A");

            if(!a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT011] failed Action::resolve").endl().breakHere();
                return false;
            }

            CLEANUP_SIGNALS;
        }

        {
            // valid use case
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp()});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::AND).pushSubAction(sA).pushSubAction(sB);

            if(!a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT012] failed to Action::resolve $and").endl().breakHere();
                return false;
            }

            CLEANUP_SIGNALS;
        }

        {
            // valid input with delay
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sC, TimeStamp() + (Action::sTimeFrameAnd / Duration(2))});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::AND).pushSubAction(sA).pushSubAction(sC);

            if(!a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT013] failed to Action::resolve $and").endl().breakHere();
                return false;
            }

            CLEANUP_SIGNALS;
        }

        {
            // invalid input: delay too long
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, 0});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp() + Action::sTimeFrameAnd});
            signalsBuffer.push_back(SignalBufferEntry{sC, TimeStamp() + (Action::sTimeFrameAnd * Duration(2))});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::AND).pushSubAction(sA).pushSubAction(sC);

            if(a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT014] Action::resolve failed").endl().breakHere();
                return false;
            }

            CLEANUP_SIGNALS;
        }

        {
            // valid input, $or action
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp() + (Action::sTimeFrameAnd / Duration(2))});
            signalsBuffer.push_back(SignalBufferEntry{sC, TimeStamp() + Action::sTimeFrameAnd});
            signalsBuffer.push_back(SignalBufferEntry{sD, TimeStamp() + (Action::sTimeFrameAnd * Duration(2))});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::OR).pushSubAction(sA).pushSubAction(sD);

            if(!a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT015] Action::resolve failed").endl().breakHere();
                return false;
            }

            CLEANUP_SIGNALS;
        }

        {
            // invalid input, $or action
            INIT_SIGNALS;
            signalsBuffer.push_back(SignalBufferEntry{sA, TimeStamp()});
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp() + (Action::sTimeFrameAnd / Duration(2))});
            signalsBuffer.push_back(SignalBufferEntry{sD, TimeStamp() + (Action::sTimeFrameAnd * Duration(2))});
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::OR).pushSubAction(sC).pushSubAction(sE);

            if(a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT016] Action::resolve failed").endl().breakHere();
                return false;
            }

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

            if(a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT017] Action::resolve failed").endl().breakHere();
                return false;
            }

            // ok
            signalsBuffer.push_back(SignalBufferEntry{sB, TimeStamp()});

            if(!a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT018] Action::resolve failed").endl().breakHere();
                return false;
            }

            // too late
            signalsBuffer.push_back(SignalBufferEntry{sD, TimeStamp() + (Action::sTimeFrameAnd * Duration(2))});

            if(a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT019] Action::resolve failed").endl().breakHere();
                return false;
            }

            CLEANUP_SIGNALS;
        }

        {
            // emtpy input, $or action
            INIT_SIGNALS;
            std::list<SignalBufferEntry>::const_iterator it_signal = signalsBuffer.begin();
            Action a = Action(Action::Type::OR).pushSubAction(sA).pushSubAction(sD);

            if(a.resolve(it_signal, signalsBuffer))
            {
                Debug::error("[UT017] Action::resolve failed").endl().breakHere();
                return false;
            }

            CLEANUP_SIGNALS;
        }

#undef INIT_SIGNALS
#undef CLEANUP_SIGNALS
        return true;
    }
}
