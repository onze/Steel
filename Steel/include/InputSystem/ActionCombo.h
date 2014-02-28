#ifndef STEEL_ACTIONCOMBO_H
#define STEEL_ACTIONCOMBO_H

#include <initializer_list>

#include "steeltypes.h"
#include "Action.h"

namespace Steel
{
    class UnitTestExecutionContext;
    class SignalBufferEntry;

    class ActionCombo
    {
    public:
        static const char *SEQUENCE_ATTRIBUTE;
        static const char *SIGNAL_ATTRIBUTE;

        /// Max allowed duration between 2 inputs of a combo for it to be valid. In millisecond.
        static Duration sDefaultTimeGap;
        ActionCombo();
        ActionCombo(Ogre::String const &signal);
        ActionCombo(Signal const signal);
        ActionCombo(ActionCombo const &o);
        ~ActionCombo();
        ActionCombo &operator=(ActionCombo const &o);
        bool operator==(ActionCombo const &o) const;
        bool operator!=(ActionCombo const &o) const;
        Hash hash() const;

        /// Adds an action to the combo sequence. Returns a ref to the combo, for chained building it.
        ActionCombo &push_back(Action const &action);
        unsigned int size() const;
        /// Returns a set of all signals used by this combo's actions.
        std::set<Signal> signalsInvolved() const;

        bool evaluate(std::list<SignalBufferEntry> const &signalsBuffer, TimeStamp const now_tt = 0) const;

        // Serialization
        /// Reads itself from a Json::Value
        void toJson(Json::Value &value) const;

        // getters
        Signal signal() const {return mSignal;}
        std::vector<Action> const &actions() const {return mActions;}
    private:
        Signal mSignal;
        std::vector<Action> mActions;
    };

    bool utest_ActionCombo(UnitTestExecutionContext const *context);
}


namespace std
{
    template<>
    struct hash<Steel::ActionCombo>
    {
        typedef Steel::ActionCombo argument_type;
        typedef std::size_t value_type;

        value_type operator()(argument_type const &combo) const
        {
            value_type h = std::hash<Steel::Signal>()(combo.signal());

            for(Steel::Action const & action : combo.actions())
                Steel::StdUtils::hash_combine(h, action);

            return h;
        }
    };
}
#endif // ACTIONCOMBO_H
