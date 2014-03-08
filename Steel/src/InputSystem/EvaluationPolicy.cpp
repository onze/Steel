
#include "InputSystem/EvaluationPolicy.h"

namespace Steel
{
    
    //const char *EvaluationPolicy::RECOVERDURATION_ATTRIBUTE = "recoverDuration";
    
    EvaluationPolicy::EvaluationPolicy():
        mType(PolicyType::None), mRecoverDuration(DURATION_MIN),
        mComboEvaluates(false), mLastEvaluationTimestamp()
    {

    }

    EvaluationPolicy::EvaluationPolicy(PolicyType type):
        mType(type), mRecoverDuration(DURATION_MIN),
        mComboEvaluates(false), mLastEvaluationTimestamp()
    {

    }

    EvaluationPolicy::EvaluationPolicy(const EvaluationPolicy &o):
        mType(o.mType), mRecoverDuration(o.mRecoverDuration),
        mComboEvaluates(o.mComboEvaluates), mLastEvaluationTimestamp(o.mLastEvaluationTimestamp)
    {

    }

    EvaluationPolicy::~EvaluationPolicy()
    {

    }

    EvaluationPolicy &EvaluationPolicy::operator=(const EvaluationPolicy &o)
    {
        if(this != &o)
        {
            mType = o.mType;
            mRecoverDuration = o.mRecoverDuration;
            mComboEvaluates = o.mComboEvaluates;
            mLastEvaluationTimestamp = o.mLastEvaluationTimestamp;
        }

        return *this;
    }

    bool EvaluationPolicy::operator==(const EvaluationPolicy &o) const
    {
        return o.mType == mType &&
               o.mRecoverDuration == mRecoverDuration &&
               o.mComboEvaluates == mComboEvaluates &&
               o.mLastEvaluationTimestamp == mLastEvaluationTimestamp;
    }

    bool EvaluationPolicy::evaluate(TimeStamp now_tt, bool comboEvaluates) const
    {
        bool shouldEmit = comboEvaluates;

        if(comboEvaluates)
        {
            if(EvaluationPolicy::PolicyType::None != mType)
            {
                // early cancel based on evaluation policy
                if(has(mType, PolicyType::RecoverDuration))
                    shouldEmit &= (!mComboEvaluates) || static_cast<Duration>(now_tt - mLastEvaluationTimestamp) > mRecoverDuration;

                if(has(mType, PolicyType::Flip))
                    shouldEmit &= !mComboEvaluates;

                mLastEvaluationTimestamp = now_tt;
            }
        }
        else
        {
            shouldEmit = false;
        }

        mComboEvaluates = comboEvaluates;
        return shouldEmit;
    }

    EvaluationPolicy &EvaluationPolicy::type(EvaluationPolicy::PolicyType policyType)
    {
        mType = policyType;
        return *this;
    }

    EvaluationPolicy &EvaluationPolicy::recoverDuration(Duration duration)
    {
        mType |= PolicyType::RecoverDuration; // useful in case it was forgotten
        mRecoverDuration = duration;
        return *this;
    }
}
