#include "AgentSpec.h"

namespace Steel
{
    const Ogre::String AgentSpec::TAG="tag";

    AgentSpec::AgentSpec():mTag("")
    {

    }

    AgentSpec::AgentSpec(const AgentSpec& o)
    {
        mTag=o.mTag;
    }

    AgentSpec::~AgentSpec()
    {

    }

    AgentSpec& AgentSpec::operator=(const AgentSpec& o)
    {
        mTag=o.mTag;
        return *this;
    }

    bool AgentSpec::operator==(const AgentSpec& o) const
    {
        return mTag==o.mTag;
    }

    bool AgentSpec::parseJson(const Json::Value &root)
    {
        mTag=root.isMember(AgentSpec::TAG)?root[AgentSpec::TAG].asCString():"";
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
