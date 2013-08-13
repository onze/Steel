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

    BTStateIndex BTSequence::switchToNextChild()
    {
        if(mToken.end==++mCurrentChildNodeIndex)
            mCurrentChildNodeIndex=firstChildIndex();
        return mCurrentChildNodeIndex;
    }

    BTStateIndex BTSequence::nodeSkippedTo()
    {
        return mCurrentChildNodeIndex;
    }

    void BTSequence::childReturned(BTState state)
    {
        // child
        switch(state)
        {
            case SUCCESS:
                // it was the last child -> overall success
                if(mToken.end-1==mCurrentChildNodeIndex)
                {
                    mState = SUCCESS;
                }
                else
                {
                    mState=SKIPT_TO;
                }
                switchToNextChild();
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
        if(mMaxLoops==mNLoops)
            return;
        ++mNLoops;
        if(mMaxLoops>0)
        {
            if(mMaxLoops>mNLoops)
                mState=SKIPT_TO;
        }
        else
            mState=SKIPT_TO;
    }

    /*
     BT*Node::BTState BTSequence::run()
     {
         if (mState == READY)
             onStartRunning();
         std::cout << "BTSequence::run()" << std::endl;

         BTState state;
         while (it != mChildren.end())
         {
             state = (*it)->run();
             switch (state)
             {
                 case RUNNING:
                     return RUNNING;
                 case READY:
                 case SUCCESS:
                     ++it;
                     continue;
                 case FAILURE:
                     mState = READY;
                     return FAILURE;
                 case ERROR:
                     std::cout << "ERROR !" << std::endl;
                     break;
         }
         }
         onStopRunning();
         if (it == mChildren.end())
             return SUCCESS;
         std::cout << "in BTSequence::run(): bad code path: not all children have been consumed, "
         << "yet we're not looping on them" << std::endl;
         assert(false);
         return FAILURE;
     }
     */

    /*
    void BTSequence::onStartRunning()
    {
    	std::cout << "BTSequence::onStartRunning()" << std::endl;
    	it = mChildren.begin();
    	mState = RUNNING;
    }

    void BTSequence::onStopRunning()
    {
    	std::cout << "BTSequence::onStopRunning()" << std::endl;
    	mState = READY;
    }
    */


}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

