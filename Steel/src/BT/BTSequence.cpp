/*
 * BTSequence.cpp
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */
#include <iostream>
#include "BT/BTSequence.h"
#include <Debug.h>
#include <assert.h>

namespace Steel
{
    const char *BTSequence::MAX_LOOPS_ATTRIBUTE="maxLoops";

    BTSequence::BTSequence(const Steel::BTShapeToken& token):BTNode(token),
        mCurrentChildNodeIndex(0),mNLoops(0),mMaxLoops(0)
    {
        mCurrentChildNodeIndex=firstChildIndex();
        mState=SKIPT_TO;
    }

    BTSequence::~BTSequence()
    {
    }

    bool BTSequence::parseNodeContent(Json::Value &root)
    {
        static Ogre::String intro="in BTSequence::parseNodeContent(): ";
        if(root.isMember(MAX_LOOPS_ATTRIBUTE))
        {
            Json::Value value=root[MAX_LOOPS_ATTRIBUTE];
            if(value.isUInt() || value.isInt())
            {
                mMaxLoops=value.asUInt();
                mNLoops=0;
                mCurrentChildNodeIndex=firstChildIndex();
                mState=SKIPT_TO;
            }
            else
            {
                Debug::error(intro)("attribute ").quotes(MAX_LOOPS_ATTRIBUTE)("is not parsable as unsigned int: ")(value)(". Aborting.");
            }
        }
        return true;
    }

    BTStateIndex BTSequence::switchToNextChild(BTNode const * const child)
    {
        mCurrentChildNodeIndex=child->token().end;
        if(mCurrentChildNodeIndex==mToken.end)
            mCurrentChildNodeIndex=firstChildIndex();
        return mCurrentChildNodeIndex;
    }

    BTStateIndex BTSequence::nodeSkippedTo()
    {
        return mCurrentChildNodeIndex;
    }

    void BTSequence::childReturned(BTNode const * const node, Steel::BTState state)
    {
        // child
        switch(state)
        {
            case SUCCESS:
                switchToNextChild(node);
                // back to first child -> overall success
                if(firstChildIndex()==mCurrentChildNodeIndex)
                {
                    mState = SUCCESS;
                }
                else
                {
                    mState = SKIPT_TO;
                }
                break;
            case FAILURE:
                mCurrentChildNodeIndex=firstChildIndex();
                mState=FAILURE;
                break;
            default:
                Debug::error("in BTSequence::childReturned(): ")("child returned state ")(state)
                ("(")(BTStateAsString[state])(")").endl();
                break;
        }
    }

    void BTSequence::onParentNotified()
    {
        ++mNLoops;
        // limited number of loops
        if(mMaxLoops>0)
        {
            // max reached
            if(mMaxLoops<=mNLoops)
                return;
            mState=SKIPT_TO;
        }
        else
        {
            // simple case
            mState=SKIPT_TO;
        }
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

