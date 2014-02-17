#ifndef STEEL_ACTIONCOMBO_H
#define STEEL_ACTIONCOMBO_H

#include <initializer_list>
#include <vector>
#include <set>

#include <steeltypes.h>
#include "Action.h"

namespace Steel
{
    class ActionCombo
    {
    private:
        ActionCombo() {};
    public:
        /// Max allowed duration between 2 inputs of a combo for it to be valid. In millisecond.
        static Duration sMaxInputInterval;
        
        ActionCombo(Ogre::String const &signal);
        ActionCombo(Signal signal);
        ActionCombo(ActionCombo const &o);
        ~ActionCombo();
        ActionCombo &operator=(ActionCombo const &o);
        bool operator==(ActionCombo const &o) const;

        /// Adds an action to the combo sequence. Returns a ref to the combo, for chained building it.
        ActionCombo &push_back(Action const &action);
        unsigned int size() const;
        /// Returns a set of all signals used by this combo's actions.
        std::set<Signal> signalsInvolved() const;

        bool evaluate(std::list<std::pair<Signal, TimeStamp>> const &mSignalsBuffer);
        Signal signal() const {return mSignal;}

    private:
        Signal mSignal;
        std::vector<Action> mActions;
    };
}
#endif // ACTIONCOMBO_H
