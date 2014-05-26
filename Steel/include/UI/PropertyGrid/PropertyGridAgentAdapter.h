#ifndef STEEL_PROPERTYGRIDAGENTADAPTER_H
#define STEEL_PROPERTYGRIDAGENTADAPTER_H

#include "steeltypes.h"
#include "PropertyGridAdapter.h"

namespace Steel
{

    class AgentManager;

    class PropertyGridAgentAdapter: public PropertyGridAdapter
    {
        PropertyGridAgentAdapter() = delete;
    public:
        PropertyGridAgentAdapter(AgentManager *agentMan, AgentId aid);
        virtual ~PropertyGridAgentAdapter();

        void buildProperties() override;

    private:
        // not owned
        AgentManager const *mAgentMan = nullptr;
        AgentId mAid = INVALID_ID;
    };
}

#endif // STEEL_PROPERTYGRIDAGENTADAPTER_H
