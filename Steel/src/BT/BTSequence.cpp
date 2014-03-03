/*
 * BTSequence.cpp
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */
#include <iostream>
#include <json/json.h>

#include "BT/BTSequence.h"
#include "Debug.h"
#include <tools/JsonUtils.h>

namespace Steel
{
    const char *BTSequence::MAX_LOOPS_ATTRIBUTE = "maxLoops";

    BTSequence::BTSequence(const Steel::BTShapeToken &token): BTNode(token),
        mCurrentChildNodeIndex(0), mNLoops(0), mMaxLoops(0)
    {
        mCurrentChildNodeIndex = firstChildIndex();
        mState = BTNodeState::SKIPT_TO;
    }

    BTSequence::~BTSequence()
    {
    }

    bool BTSequence::parseNodeContent(Json::Value &root)
    {
        static Ogre::String intro = "in BTSequence::parseNodeContent(): ";

        if(root.isMember(MAX_LOOPS_ATTRIBUTE))
        {
            mMaxLoops = JsonUtils::asUnsignedLong(root[MAX_LOOPS_ATTRIBUTE], 0);
            mNLoops = 0;
            mCurrentChildNodeIndex = firstChildIndex();
            mState = BTNodeState::SKIPT_TO;
        }

        return true;
    }

    BTStateIndex BTSequence::switchToNextChild(BTNode const *const child)
    {
        mCurrentChildNodeIndex = child->token().end;

        if(mCurrentChildNodeIndex == mToken.end)
            mCurrentChildNodeIndex = firstChildIndex();

        return mCurrentChildNodeIndex;
    }

    BTStateIndex BTSequence::nodeSkippedTo()
    {
        return mCurrentChildNodeIndex;
    }

    void BTSequence::childReturned(BTNode const *const node, BTNodeState state)
    {
        // child
        switch(state)
        {
            case BTNodeState::SUCCESS:
                switchToNextChild(node);

                // back to first child -> overall success
                if(firstChildIndex() == mCurrentChildNodeIndex)
                {
                    mState = BTNodeState::SUCCESS;
                }
                else
                {
                    mState = BTNodeState::SKIPT_TO;
                }

                break;

            case BTNodeState::FAILURE:
                mCurrentChildNodeIndex = firstChildIndex();
                mState = BTNodeState::FAILURE;
                break;

            default:
                Debug::error("in BTSequence::childReturned(): ")("child returned state ")(state).endl();
                break;
        }
    }

    void BTSequence::onParentNotified()
    {
        ++mNLoops;

        // limited number of loops
        if(mMaxLoops > 0)
        {
            // max reached
            if(mMaxLoops <= mNLoops)
                return;

            mState = BTNodeState::SKIPT_TO;
        }
        else
        {
            // simple case
            mState = BTNodeState::SKIPT_TO;
        }
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

