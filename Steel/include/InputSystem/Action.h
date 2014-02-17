#ifndef STEEL_ACTION_H
#define STEEL_ACTION_H

#include <OgrePrerequisites.h>
#include <steeltypes.h>

namespace Steel
{

    class ActionFactory
    {
    };

    class Action
    {
    private:

    public:
        /// Time window of a set of inputs for AND actions to resolve positively on it. In millisecond
        static Duration sTimeFrameAnd;
        
        
        /// Actions defined as
        enum class Type : unsigned int
        {
            INVALID_TYPE = 0,
            
            /// resolves if it finds its action value (mSignal) inside the input.
            SIMPLE = BIT(1),
            
            /// resolves if all subActions resolve in no particular order, within the timeframe.
            AND = BIT(2),
            OR = BIT(3),
            
            COMPOSITE = AND | OR
        };
        
        Action();
        Action(Ogre::String const &signal);
        Action(Signal const &signal);
        ~Action();
        bool operator==(const Action &o) const;

        /// Returns all signals used to evaluate the action
        std::set<Signal> signals() const;

        /**
         * Takes a list of inputs (Signal) and an iterator of it.
         * Will consume the iterator until an input validates it and it returns true.
         * After returning, the given iterator points to the next position to consume.
         */
        bool resolve(std::list< std::pair< Signal, TimeStamp >>::const_iterator &it_signal,
                     std::list< std::pair< Signal, TimeStamp > > const &mSignalsBuffer) const;
        
        /// Adds a subaction to a 
        Action &pushSubAction(Action const& subAction);

        bool IsOfType(Action::Type type);
        
    private:
        Type mType;
        Signal mSignal;

        /// Used by composite actions.
        std::vector<Action> mSubActions;
        
        
    };
}

#endif // STEEL_ACTION_H
