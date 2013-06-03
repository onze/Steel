#include "BT/BTNode.h"
#include <tools/File.h>


namespace Steel
{
    const Ogre::String BTNode::AGENT_SPEC="agentSpec";

    BTNode::BTNode(BTShapeToken const &token):
        mState(READY),mToken(token)
    {

    }

    BTNode::BTNode(const BTNode& other)
    {

    }

    BTNode::~BTNode()
    {

    }

    BTNode& BTNode::operator=(const BTNode& other)
    {
        return *this;
    }

    bool BTNode::operator==(const BTNode& other) const
    {
        return false;
    }

    bool BTNode::reset()
    {
        Ogre::String intro="BTNode::reset(): ";

        File contentFile(mToken.contentFile);
        if(!contentFile.exists())
        {
            Debug::error(intro)("token.contentFile \"")(contentFile)("\"does not exists. ");
            Debug::error("Aborting reset.").endl();
            return false;
        }
        Ogre::String content=contentFile.read(true);

        if(content.length()>0)
        {
            Json::Value root;
            if(!Json::Reader().parse(content, root, false))
            {
                Debug::error(intro)("content of file ")(contentFile);
                Debug::error(" is not valid json, aborting reset. Content was:").endl()(contentFile.read()).endl();
                return false;
            }
            if(!parseNodeContent(root))
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

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
