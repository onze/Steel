#ifndef STEEL_BTNODE_H
#define STEEL_BTNODE_H

#include <json/json.h>

#include <BT/btnodetypes.h>
#include <AgentSpec.h>

namespace Steel
{
class BTNode
{

public:
    BTNode(const Steel::BTShapeToken& token);
    BTNode(const BTNode& other);
    virtual ~BTNode();
    virtual BTNode& operator=(const BTNode& other);
    virtual bool operator==(const BTNode& other) const;

    /// Sets the node to its original content file settings.
    virtual bool reset();

    // getters
    inline unsigned begin()
    {
        return mToken.begin;
    }

    inline unsigned end()
    {
        return mToken.end;
    }

protected:
    static const Ogre::String AGENT_SPEC;

    /// Actual parsing. Meant to be overloaded by subclasses.
    virtual bool parseNodeContent(Json::Value &root);

    // owned
    BTState mState;
    BTShapeToken mToken;
};
}
#endif

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
