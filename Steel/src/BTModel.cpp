
#include <OgreString.h>

#include "BTModel.h"
#include "BT/BTNode.h"
#include <tools/JsonUtils.h>

namespace Steel
{
    const char *BTModel::SHAPE_NAME_ATTRIBUTE="rootPath";

    BTModel::BTModel():mStateStream(),mCurrentStateIndex(0),mStatesStack(std::stack<BTStateIndex>())
    {
    }

    BTModel::BTModel (const BTModel &o)
    {
        this->operator=(o);
    }

    BTModel::~BTModel()
    {
    }

    BTModel &BTModel::operator=(const BTModel &o)
    {
        if(this==&o)
            return *this;
        if(!isFree())
            cleanup();
        if(!o.isFree())
        {
            mStateStream=o.mStateStream;
            mCurrentStateIndex=o.mCurrentStateIndex;
            mStatesStack=o.mStatesStack;
        }
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

    
    bool BTModel::fromJson(Json::Value &node)
    {
        // building a BTModel requires structures that only the manager can access.
        throw std::runtime_error("BTModel::fromJson is not meant to be called. Use the BTModelManager.");
        return true;
    }

    void BTModel::toJson(Json::Value &node)
    {
        static const Ogre::String intro="in BTModel::toJson(): ";
        BTShapeStream *st=mStateStream.shapeStream();
        if(NULL==st)
        {
            Debug::error(intro)("stateStream's has no shapeStream !").endl();
            node[BTModel::SHAPE_NAME_ATTRIBUTE]=Json::Value::null;
        }
        else
            node[BTModel::SHAPE_NAME_ATTRIBUTE]=JsonUtils::toJson(st->mName);
    }

    void BTModel::cleanup()
    {
        mStateStream.clear();
        Model::cleanup();
    }

    void BTModel::update(float timestep)
    {
        bool debug=false;
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
                    if(debug)
                        Debug::log("running ")(BTShapeTokenTypeAsString[token.type])(" #")(token.begin).endl();
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
                    
                    if(debug)
                        Debug::log("skipping to  #")(newIndex).endl();
                    // visiting a child or a sibling
                    if(newIndex<node->end())
                        mStatesStack.push(mCurrentStateIndex);
                    else if(newIndex==node->end())
                        mStatesStack.pop();

                    mCurrentStateIndex=newIndex;
                    continue;
                case FAILURE:
                case SUCCESS:
                    
                    if(debug)
                        Debug::log(nodeState==SUCCESS?"SUCCESS":"FAILURE")(" from ")(BTShapeTokenTypeAsString[token.type])(" #")(token.begin).endl();
                    // root's success
                    if(0==mStatesStack.size())
                    {
                        mCurrentStateIndex=0;
                        node->onParentNotified();
                        return;
                    }

                    parent=mStateStream.stateAt(mStatesStack.top());
                    parent->childReturned(node, nodeState);
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
