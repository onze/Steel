
#include <OgreString.h>

#include "BTModel.h"
#include "BT/BTNode.h"
#include <tools/JsonUtils.h>
#include <BTModelManager.h>
#include <Level.h>
#include <BlackBoardModelManager.h>

namespace Steel
{
    const char *BTModel::SHAPE_NAME_ATTRIBUTE = "rootPath";

    const Ogre::String BTModel::CURRENT_PATH_NAME_VARIABLE = "__currentPath";

    BTModel::BTModel():
        mBlackBoardModelId(INVALID_ID), mLevel(nullptr), mStateStream(), mCurrentStateIndex(0), mStatesStack(std::stack<BTStateIndex>()),
        mPaused(false), mKilled(false)
    {
    }

    BTModel::BTModel(const BTModel &o):
        mBlackBoardModelId(o.mBlackBoardModelId), mLevel(o.mLevel), mStateStream(o.mStateStream), mCurrentStateIndex(o.mCurrentStateIndex), mStatesStack(o.mStatesStack),
        mPaused(o.mPaused), mKilled(o.mKilled)
    {
    }

    BTModel::~BTModel()
    {
    }

    BTModel &BTModel::operator=(const BTModel &o)
    {
        if(this == &o)
            return *this;

        if(!isFree())
            cleanup();

        if(!o.isFree())
        {
            mBlackBoardModelId = o.mBlackBoardModelId;
            mLevel = o.mLevel;
            mStateStream = o.mStateStream;
            mCurrentStateIndex = o.mCurrentStateIndex;
            mStatesStack = o.mStatesStack;
            mPaused = o.mPaused;
            mKilled = o.mKilled;
        }

        return *this;
    }

    bool BTModel::init(Steel::BTModelManager *manager, Steel::BTShapeStream *shapeStream)
    {
        mPaused = mKilled = false;
        mLevel = manager->level();
        mBlackBoardModelId = INVALID_ID;
        return switchShapeTo(shapeStream);
    }

    bool BTModel::switchShapeTo(BTShapeStream *shapeStream)
    {
        mStateStream.clear();
        mCurrentStateIndex = 0;
        mStatesStack = std::stack<BTStateIndex>();

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
        //throw std::runtime_error("BTModel::fromJson is not meant to be called. Use the BTModelManager.");
        return false;
    }

    void BTModel::toJson(Json::Value &node)
    {
        static const Ogre::String intro = "in BTModel::toJson(): ";
        BTShapeStream *st = mStateStream.shapeStream();

        if(nullptr == st)
        {
            Debug::error(intro)("stateStream's has no shapeStream !").endl();
            node[BTModel::SHAPE_NAME_ATTRIBUTE] = Json::Value::null;
        }
        else
            node[BTModel::SHAPE_NAME_ATTRIBUTE] = JsonUtils::toJson(st->mName);
    }

    void BTModel::cleanup()
    {
        mStateStream.clear();
        Model::cleanup();
    }

    void BTModel::update(float timestep)
    {
        bool debug = true;

        static const Ogre::String intro = "in BTModel::update(): ";

        if(mPaused || mKilled)
            return;

        BTStateIndex prevStateIndex = mCurrentStateIndex;
        BTStateIndex newIndex = mCurrentStateIndex;
        BTNode *node = nullptr;
        BTNode *parent = nullptr;
        BTState nodeState = SUCCESS;
        BTShapeToken token;

        while(true)
        {
            if(nodeState != READY)
            {
                prevStateIndex = mCurrentStateIndex;
                token = mStateStream.tokenAt(mCurrentStateIndex);

                if(BTUnknownToken == token.type)
                {
                    Debug::error(intro)("node #")(prevStateIndex)(" with token ")(mStateStream.tokenAt(prevStateIndex))
                    ("yielded an unknown token type as index ")(mCurrentStateIndex)
                    (". Node:")(node).endl()("Aborting BTNode update.").endl();
                    kill();
                    return;
                }

                node = mStateStream.stateAt(mCurrentStateIndex);
            }

            nodeState = node->state();

            switch(nodeState)
            {
                case READY:
                    if(debug)
                        Debug::log("running ")(BTShapeTokenTypeAsString[token.type])(" #")(token.begin).endl();

                    node->run(this, timestep);
                    continue;

                case SKIPT_TO:
                    newIndex = node->nodeSkippedTo();

                    if(node->begin() == newIndex)
                    {
                        Debug::error(intro)("node #")(mCurrentStateIndex)(" with token ")(token)
                        (" has SKIPT_TO state pointing to itself. To run again, it should keep the READY state.").endl()
                        ("Aborting tree run.");
                        mStatesStack = std::stack<BTStateIndex>();
                        mCurrentStateIndex = 0;
                        return;
                    }

                    if(debug)
                        Debug::log("skipping to  #")(newIndex).endl();

                    // visiting a child
                    if(newIndex < node->end())
                    {
                        mStatesStack.push(mCurrentStateIndex);
                        mCurrentStateIndex = newIndex;
                        continue;
                    }
                    else
                    {
                        // node tries to skip to not a child: assume success
                        newIndex = node->end();
                        nodeState = SUCCESS;
                    }

                case FAILURE:
                case SUCCESS:

                    if(debug)
                        Debug::log(nodeState == SUCCESS ? "SUCCESS" : "FAILURE")(" from ")(BTShapeTokenTypeAsString[token.type])(" #")(token.begin).endl();

                    // root's success
                    if(0 == mStatesStack.size())
                    {
                        mCurrentStateIndex = 0;
                        node->onParentNotified();
                        return;
                    }

                    parent = mStateStream.stateAt(mStatesStack.top());
                    parent->childReturned(node, nodeState);
                    node->onParentNotified();
                    // give control back to parent
                    mCurrentStateIndex = mStatesStack.top();
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

    void BTModel::setSelected(bool flag)
    {
        mPaused = flag;
    }

    void BTModel::setBlackboardModelId(ModelId mid)
    {
        mBlackBoardModelId = mid;
    }

    void BTModel::setPath(Ogre::String const &name)
    {
        if(Ogre::StringUtil::BLANK == name)
            return;

        setVariable(BTModel::CURRENT_PATH_NAME_VARIABLE, name);
    }

    Ogre::String BTModel::path()
    {
        return getStringVariable(BTModel::CURRENT_PATH_NAME_VARIABLE, Ogre::StringUtil::BLANK);
    }

    bool BTModel::hasPath()
    {
        return Ogre::StringUtil::BLANK == path();
    }

    void BTModel::setVariable(Ogre::String const &name, Ogre::String const &value)
    {
        BlackBoardModel *bbModel = mLevel->blackBoardModelMan()->at(mBlackBoardModelId);

        if(nullptr == bbModel)
            return;

        bbModel->setVariable(name, value);
    }
    
    void BTModel::setVariable(Ogre::String const &name, AgentId const &value)
    {
        BlackBoardModel *bbModel = mLevel->blackBoardModelMan()->at(mBlackBoardModelId);
        
        if(nullptr == bbModel)
            return;
        
        bbModel->setVariable(name, value);
    }
    
    Ogre::String BTModel::getStringVariable(Ogre::String const &name, Ogre::String const &defaultValue/*=Ogre::StringUtil::BLANK*/)
    {
        BlackBoardModel *bbModel = mLevel->blackBoardModelMan()->at(mBlackBoardModelId);
        
        if(nullptr == bbModel)
            return defaultValue;
        
        return bbModel->getStringVariable(name);
    }
    
    AgentId BTModel::getAgentIdVariable(Ogre::String const &name, AgentId const &defaultValue/*=INVALID_ID*/)
    {
        BlackBoardModel *bbModel = mLevel->blackBoardModelMan()->at(mBlackBoardModelId);
        
        if(nullptr == bbModel)
            return defaultValue;
        
        return bbModel->getAgentIdVariable(name);
    }

} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
