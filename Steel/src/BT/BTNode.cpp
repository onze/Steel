#include "BT/BTNode.h"
#include <tools/File.h>


namespace Steel
{
    const char *BTNode::AGENT_SPEC_ATTRIBUTE = "agentSpec";

    BTNode::BTNode(BTShapeToken const &token):
    mState(BTNodeState::READY), mToken(token)
    {
    }

    BTNode::BTNode(const BTNode &o): mState(o.mState), mToken(o.mToken)
    {
    }

    BTNode::~BTNode()
    {
    }

    BTNode &BTNode::operator=(const BTNode &o)
    {
        if(this == &o)
            return *this;

        mState = o.mState;
        mToken = o.mToken;
        return *this;
    }

    bool BTNode::operator==(const BTNode &o) const
    {
        return mState == o.mState && mToken == o.mToken;
    }

    BTNodeState BTNode::state()
    {
        return mState;
    }

    bool BTNode::reset()
    {
        Ogre::String intro = "BTNode::reset(): ";

        File contentFile(mToken.contentFile);

        if(!contentFile.exists())
        {
            Debug::error(intro)("token.contentFile \"")(contentFile)("\"does not exists. ");
            Debug::error("Aborting reset.").endl();
            return false;
        }

        Ogre::String content = contentFile.read(true);

        if(content.length() > 0)
        {
            Json::Value root;

            if(!Json::Reader().parse(content, root, false))
            {
                Debug::error(intro)("content of file ")(contentFile);
                Debug::error(" is not valid json, aborting reset. Content was:").endl()(contentFile.read()).endl();
                return false;
            }

            if(!this->parseNodeContent(root))
            {
                Debug::warning(intro)("Could not parse content properly.").endl();
                return false;
            }
        }

        return true;
    }

    bool BTNode::parseNodeContent(Json::Value &root)
    {
        return true;
    }

    BTStateIndex BTNode::firstChildIndex()
    {
        return mToken.begin + 1;
    }

    BTStateIndex BTNode::lastChildIndex()
    {
        return mToken.end - 1;
    }

    void BTNode::run(BTModel *btModel, float timestep)
    {
        mState = BTNodeState::SKIPT_TO;
    }

    BTStateIndex BTNode::nodeSkippedTo()
    {
        return firstChildIndex();
    }

    void BTNode::childReturned(BTNode const *const child, BTNodeState state)
    {
        mState = state;
    }

    void BTNode::onParentNotified()
    {
        mState = BTNodeState::READY;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
