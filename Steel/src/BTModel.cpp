
#include <OgreString.h>

#include "BTModel.h"
#include "BT/BTNode.h"

namespace Steel
{

    BTModel::BTModel():mStateStream(),mCurrentStateIndex(0),mStatesStack(std::stack<BTStateIndex>())
    {
    }

    BTModel::BTModel (const BTModel &o)
    {
        mStateStream=o.mStateStream;
        mCurrentStateIndex=o.mCurrentStateIndex;
        mStatesStack=o.mStatesStack;
    }

    BTModel::~BTModel()
    {
    }

    BTModel &BTModel::operator=(const BTModel &o)
    {
        mStateStream=o.mStateStream;
        mCurrentStateIndex=o.mCurrentStateIndex;
        mStatesStack=o.mStatesStack;
        return *this;
    }

    bool BTModel::init(Steel::BTShapeStream* shapeStream)
    {
        return switchShapeTo(shapeStream);
    }

    bool BTModel::switchShapeTo(BTShapeStream* shapeStream)
    {
        mStateStream.clear();
        mCurrentStateIndex=0;
        mStatesStack=std::stack<BTStateIndex>();
        if(!mStateStream.init(shapeStream))
        {
            Debug::error("BTModel::switchShapeTo(): can't init stateStream from shapeStream: ")(shapeStream).endl();
            return false;
        }
        return true;
    }

    ModelType BTModel::modelType()
    {
        return MT_BT;
    }

    bool BTModel::fromJson(Json::Value &node)
    {
        return true;
    }

    void BTModel::toJson(Json::Value &node)
    {

    }

    void BTModel::cleanup()
    {
        mStateStream.clear();
    }

    void BTModel::update(float timestep)
    {
        static const Ogre::String intro="in BTModel::update(): ";
        BTStateIndex prevStateIndex=mCurrentStateIndex;
        BTStateIndex newIndex=mCurrentStateIndex;
        BTNode* node=NULL;
        BTNode* parent=NULL;
        BTState nodeState=SUCCESS;
        BTShapeToken token;
        while(true)
        {
            if(nodeState!=READY)
            {
                prevStateIndex=mCurrentStateIndex;
                token=mStateStream.tokenAt(mCurrentStateIndex);
                if(BTUnknownToken==token.type)
                {
                    Debug::error(intro)("node #")(prevStateIndex)(" with token ")(mStateStream.tokenAt(prevStateIndex))
                    ("yielded an unknown token type as index ")(mCurrentStateIndex)
                    (". Node:")(node).endl()("Aborting BTNode update.").endl();
                    return;
                }

                node=mStateStream.stateAt(mCurrentStateIndex);
            }

            nodeState=node->state();
            switch(nodeState)
            {
                case READY:
                    node->run(timestep);
                    continue;
                case SKIPT_TO:
                    newIndex=node->nodeSkippedTo();
                    
                    if(node->begin()==newIndex)
                    {
                        Debug::error(intro)("node #")(mCurrentStateIndex)(" with token ")(token)
                        (" has SKIPT_TO state pointing to itself. To run again, it should keep the READY state.").endl()
                        ("Aborting tree run.");
                        mStatesStack=std::stack<BTStateIndex>();
                        mCurrentStateIndex=0;
                        return;
                    }
                    // visiting a child or a sibling
                    if(newIndex<node->end())
                        mStatesStack.push(mCurrentStateIndex);
                    else if(newIndex==node->end())
                        mStatesStack.pop();

                    mCurrentStateIndex=newIndex;
                    continue;
                case FAILURE:
                case SUCCESS:

                    // root's success
                    if(0==mStatesStack.size())
                    {
                        mCurrentStateIndex=0;
                        node->onParentNotified();
                        return;
                    }

                    parent=mStateStream.stateAt(mStatesStack.top());
                    parent->childReturned(nodeState);
                    node->onParentNotified();
                    // give control back to parent
                    mCurrentStateIndex=mStatesStack.top();
                    mStatesStack.pop();
                    continue;
                case RUNNING:
                    // stop execution for this frame.
                    return;
                default:
                    Debug::error(intro)("node #")(mCurrentStateIndex)(" with token ")(token)
                    ("yielded an unknown state.")(". Node:")(node).endl()("Aborting BTNode update.").endl();
                    return;
            }
        }
    }

} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
