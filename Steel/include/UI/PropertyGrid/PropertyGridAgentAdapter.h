#ifndef STEEL_PROPERTYGRIDAGENTADAPTER_H
#define STEEL_PROPERTYGRIDAGENTADAPTER_H

#include "PropertyGridAdapter.h"
#include <steeltypes.h>

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
        AgentId mAid;
    };

    class PropertyGridModelAdapter: public PropertyGridAdapter
{};
}

#endif // STEEL_PROPERTYGRIDAGENTADAPTER_H
