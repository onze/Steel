#ifndef STEEL_ACTION_H
#define STEEL_ACTION_H

#include <steeltypes.h>
#include <tools/StdUtils.h>

namespace Steel
{

    struct SignalBufferEntry;
    class UnitTestExecutionContext;

    /**
     * Can validate conditions about data passed to ::resolve.
     * Action::Type defines the types of predicate. Can only have one value.
     * Another name could be Predicate.
     */
    class Action
    {
    public:
        static const Action ANY;

        static char const *const AND_ATTRIBUTE;
        static char const *const OR_ATTRIBUTE;
        static char const *const ANY_ATTRIBUTE;

        static char const *const MAX_DELAY_ATTRIBUTE;
        static char const *const MIN_DELAY_ATTRIBUTE;


        /// Time window between first and last inputs of a set for an AND action to resolve positively. In millisecond;
        static Duration sDefaultTimeWindowAnd;

        /// No inheritance, couple switches in the implementation.
        enum class Type : unsigned int
        {
            INVALID_TYPE = 0,

            /// resolves if it finds its action value (mSignal) inside the input.
            SIMPLE = STEEL_BIT(1),

            /// resolves if all subActions resolve in no particular order, within the timeframe.
            AND = STEEL_BIT(2),
            OR = STEEL_BIT(3),

            COMPOSITE = AND | OR,

            /**
             * Conditions over previous actions. Mostly time-based. META Actions don't consume input.
             */
            META = STEEL_BIT(4),

            /// Always resolve positively. Always consumes 1 signal.
            ANY = STEEL_BIT(5)
        };

        Action();
        Action(Ogre::String const &signal);
        Action(char const *const signal);
        Action(Signal const &signal);
        Action(Type const type);
        virtual ~Action();
        bool operator==(const Action &o) const;
        bool operator!=(const Action &o) const;

        /// Returns all signals used to evaluate the action
        std::set<Signal> signals() const;

        /**
         * Takes a list of inputs (Signal) and an iterator of it.
         * Will consume the iterator until an input validates it and it returns true.
         * After returning, the given iterator points to the last consumed position.
         */
        bool resolve(std::list<SignalBufferEntry>::const_iterator const &it_cbegin,
                     std::list<SignalBufferEntry>::const_iterator &it_signal,
                     std::list<SignalBufferEntry>::const_iterator const &it_cend,
                     TimeStamp now_tt = 0
                    ) const;

        /// Adds a subaction to a
        Action &pushSubAction(Action const &subAction);

        bool IsOfType(Action::Type const type) const;

        /// Serializes itself into the given value. All previous value attributes will be ERASED.
        void toJson(Json::Value &value) const;

        //Type::META parameters
        /// In milliseconds. Set the max delay since last action for this one to be able to resolve.
        Action &maxDelay(Duration maxDelay);
        /// In milliseconds. Set the min delay since last action for this one to be able to resolve.
        Action &minDelay(Duration minDelay);

        // getters
        Action::Type type()const {return mType;}
        /// Signal the action resolves on.
        Signal signal()const {return mSignal;}
        /// In milliseconds.
        Duration getMaxDelay()const {return mMaxDelay;}
        /// In milliseconds.
        Duration getMinDelay()const {return mMinDelay;}
        std::vector<Action> const &subActions()const {return mSubActions;}
    private:
        /// Analyses the ctor string signal arg to determine type, signal.
        void parseSignalString(Ogre::String const &signal);

        /// Main symbol of the Enum Polymorphism Pattern.
        Type mType = Type::INVALID_TYPE;
        /// Signal the action resolves on.
        Signal mSignal = INVALID_SIGNAL;

        // meta info
        /// In milliseconds. Max delay since last action for this one to be able to resolve.
        Duration mMaxDelay;
        /// In milliseconds. Min delay since last action for this one to be able to resolve.
        Duration mMinDelay;

        /// Used by composite actions.
        std::vector<Action> mSubActions;
    };

    bool utest_Action(UnitTestExecutionContext const *context);

}

namespace std
{
    /// hash an Action::Type enum value
    template <>
    struct hash<Steel::Action::Type>
    {
        typedef Steel::Action::Type argument_type;
        typedef std::size_t value_type;
        value_type operator()(argument_type value) const
        {
            return std::hash<std::underlying_type<argument_type>::type>()(static_cast<std::underlying_type<argument_type>::type>(value));
        }
    };

    /// hash an Action value
    template<>
    struct hash<Steel::Action>
    {
        typedef Steel::Action argument_type;
        typedef std::size_t value_type;

        value_type operator()(argument_type const &action) const
        {
            value_type h = std::hash<Steel::Action::Type>()(action.type());
            Steel::StdUtils::hash_combine(h, action.signal());

            for(Steel::Action const & sub : action.subActions())
                Steel::StdUtils::hash_combine(h, sub);

            return h;
        }
    };
}


#endif // STEEL_ACTION_H
