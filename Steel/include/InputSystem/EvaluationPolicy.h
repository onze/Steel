#ifndef STEEL_EVALUATIONPOLICY_H
#define STEEL_EVALUATIONPOLICY_H
#include <steeltypes.h>
#include <tools/StdUtils.h>

namespace Steel
{
    /**
     * ActionCombo decorator operating on top of the combo evaluation.
     * Serves as a modulator of the combo evaluation state: some combos we want them with a recover delay,
     * some others with a switch-like behavior, some others we want to negate.
     * From a higfher perspective, this is a predicate over a bool input.
     */
    class EvaluationPolicy
    {
    public:
        //static const char *RECOVERDURATION_ATTRIBUTE;
        
        enum class PolicyType : unsigned
        {
            /// Evaluates just like its input
            None = STEEL_BIT(0),
            /// Evaluates iff its input does and it was not already doing so
            Flip = STEEL_BIT(1),
            /// Evaluates iff the input does and at least recoverDuration() has elapsed.
            RecoverDuration = STEEL_BIT(2)
        };

        EvaluationPolicy();
        EvaluationPolicy(PolicyType type);
        EvaluationPolicy(const EvaluationPolicy &o);
        ~EvaluationPolicy();
        EvaluationPolicy &operator=(const EvaluationPolicy &o);
        bool operator==(const EvaluationPolicy &o) const;

        bool evaluate(TimeStamp now_tt, bool comboEvaluates) const;

        //getters
        ///
        EvaluationPolicy::PolicyType getType() const {return mType;};
        TimeStamp const recoverDuration() const {return mRecoverDuration;}
        
        bool isEvaluating() const {return mComboEvaluates;}
        //setters

        /// Sets the policy type
        EvaluationPolicy &type(EvaluationPolicy::PolicyType policyType);
        /// Milliseconds. Sets the minimum duration required since last evaluation for the combo to be able to evaluate.
        EvaluationPolicy &recoverDuration(Duration duration);

    private:
        PolicyType mType;
        /// Milliseconds. Minimum delay since last positive evaluation for the input to be considered.
        Duration mRecoverDuration;

        /// Transient
        mutable bool mComboEvaluates;
        /// Last time the combo evaluated. Transient.
        mutable TimeStamp mLastEvaluationTimestamp;
    };
}


namespace std
{

    /// hash an EvaluationPolicy::PolicyType enum value
    template <>
    struct hash<Steel::EvaluationPolicy::PolicyType>
    {
        typedef Steel::EvaluationPolicy::PolicyType argument_type;
        typedef std::size_t value_type;
        value_type operator()(argument_type value) const
        {
            return std::hash<std::underlying_type<argument_type>::type>()(static_cast<std::underlying_type<argument_type>::type>(value));
        }
    };

    /// hash an EvaluationPolicy instance
    template<>
    struct hash<Steel::EvaluationPolicy>
    {
        typedef Steel::EvaluationPolicy argument_type;
        typedef std::size_t value_type;

        value_type operator()(argument_type const &policy) const
        {
            value_type h = std::hash<Steel::EvaluationPolicy::PolicyType>()(policy.getType());
            Steel::StdUtils::hash_combine(h, policy.recoverDuration());
            return h;
        }
    };
}


//http://stackoverflow.com/questions/12059774/c11-standard-conformant-bitmasks-using-enum-class

inline constexpr Steel::EvaluationPolicy::PolicyType
operator|(Steel::EvaluationPolicy::PolicyType __x, Steel::EvaluationPolicy::PolicyType __y)
{
    typedef Steel::EvaluationPolicy::PolicyType argument_type;
    typedef std::underlying_type<argument_type>::type underlying_type;
    return static_cast<argument_type>(static_cast<underlying_type>(__x) | static_cast<underlying_type>(__y));
}

inline constexpr Steel::EvaluationPolicy::PolicyType
operator&(Steel::EvaluationPolicy::PolicyType __x, Steel::EvaluationPolicy::PolicyType __y)
{
    typedef Steel::EvaluationPolicy::PolicyType argument_type;
    typedef std::underlying_type<Steel::EvaluationPolicy::PolicyType>::type underlying_type;
    return static_cast<argument_type>(static_cast<underlying_type>(__x) & static_cast<underlying_type>(__y));
}

inline Steel::EvaluationPolicy::PolicyType &
operator&=(Steel::EvaluationPolicy::PolicyType &__x, Steel::EvaluationPolicy::PolicyType __y)
{
    __x = __x & __y;
    return __x;
}

inline Steel::EvaluationPolicy::PolicyType &
operator|=(Steel::EvaluationPolicy::PolicyType &__x, Steel::EvaluationPolicy::PolicyType __y)
{
    __x = __x | __y;
    return __x;
}

namespace Steel
{
    inline constexpr bool has(EvaluationPolicy::PolicyType __x, EvaluationPolicy::PolicyType __y)
    {
        return EvaluationPolicy::PolicyType::None != ::operator&(__x , __y);
    }
}

#endif // STEEL_EVALUATIONPOLICY_H
