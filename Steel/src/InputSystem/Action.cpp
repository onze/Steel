#include "InputSystem/Action.h"
#include <SignalManager.h>
#include <Debug.h>

namespace Steel
{
    Duration Action::sTimeFrameAnd = 25;

    Action::Action(): mType(Action::Type::COMPOSITE), mSubActions()
    {
        mSignal = INVALID_SIGNAL;
    }

    Action::Action(const Ogre::String &signal): mType(Action::Type::SIMPLE),
        mSignal(SignalManager::instance().toSignal(signal)), mSubActions()
    {
    }

    Action::Action(Signal const &signal): mType(Action::Type::SIMPLE),
        mSignal(signal), mSubActions()
    {
    }

    Action::~Action()
    {
    }

    bool Action::operator==(const Steel::Action &o) const
    {
        return o.mType == mType && o.mSignal == mSignal && o.mSubActions == mSubActions;
    }

    bool Action::IsOfType(Action::Type type)
    {
        return 0 != (static_cast<unsigned int>(type) & static_cast<unsigned int>(mType));
    }

    Action &Action::pushSubAction(Action const &subAction)
    {
        if(!IsOfType(Action::Type::COMPOSITE))
        {
            Debug::error("Can't pushSubAction on a non-composite Action. Skipped.").endl();
            assert(false);
        }
        else
        {
            mSubActions.push_back(subAction);
        }

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

    bool Action::resolve(std::list< std::pair< Signal, TimeStamp > >::const_iterator &it_signal,
                         std::list< std::pair< Signal, TimeStamp > > const &signalsBuffer) const
    {

        switch(mType)
        {
            case Action::Type::SIMPLE:
                while(it_signal->first != mSignal && signalsBuffer.end() != it_signal)
                    ++it_signal;

                if(signalsBuffer.end() != it_signal)
                {
                    ++it_signal;
                    return true;
                }

                break;

            case Action::Type::AND:
            {
                // easy bailout: input is not big enough
                if(signalsBuffer.size() < mSubActions.size())
                    return false;

                // validates if all actions resolve
                bool allFound = true;
                std::list< std::pair< Signal, TimeStamp > >::const_iterator it_first = signalsBuffer.cend();
                std::list< std::pair< Signal, TimeStamp > >::const_iterator it_last = signalsBuffer.cbegin();

                for(auto const & sub : mSubActions)
                {
                    // all actions should resolve from the same starting position
                    auto it = it_signal;
                    allFound &= sub.resolve(it, signalsBuffer);

                    // action was found was valid
                    if(allFound)
                    {
                        // update first/last input
                        if(it->second < it_first->second)
                            it_first = it;

                        if(it->second > it_last->second)
                            it_last = it;
                    }
                    else
                    {
                        return false;
                    }
                }

                // merge back to it_signal
                it_signal = it_last;

                // validate timeFrame
                if(allFound && (it_last->second - it_first->second) < static_cast<TimeStamp>(Action::sTimeFrameAnd))
                    return true;

                break;
            }

            case Action::Type::OR:
                Debug::error(" not implemented").endl().breakHere();
                break;

            case Action::Type::INVALID_TYPE:
            default:
                break;
        }

        return false;
    }
}
