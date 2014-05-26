#include "UI/PropertyGrid/PropertyGridAgentAdapter.h"
#include <models/AgentManager.h>
#include <models/Agent.h>
#include <Debug.h>

namespace Steel
{

    PropertyGridAgentAdapter::PropertyGridAgentAdapter(AgentManager *agentMan, AgentId aid): PropertyGridAdapter(),
        mAgentMan(agentMan), mAid(aid)
    {
    }

    PropertyGridAgentAdapter::~PropertyGridAgentAdapter()
    {
        mAgentMan = nullptr;
        mAid = INVALID_ID;
    }

    void PropertyGridAgentAdapter::buildProperties()
    {
        Agent *agent = mAgentMan->getAgent(mAid);

        if(nullptr == agent)
        {
            PropertyGridProperty *prop_id = new PropertyGridProperty("AgentId " + Ogre::StringConverter::toString(mAid) + " (invalid)");
            mProperties.push_back(prop_id);
            Debug::error(STEEL_METH_INTRO, "could not find agent ", mAid).endl();
            return;
        }

        /// persistence toggle
        PropertyGridProperty *prop = new PropertyGridProperty("Persistent");
        {
            PropertyGridProperty::BoolReadCallback readCB([this]()->bool
            {
                Agent *agent = mAgentMan->getAgent(mAid);
                return agent->isPersistent();
            });
            PropertyGridProperty::BoolWriteCallback writeCB([this](bool flag)
            {
                Agent *agent = mAgentMan->getAgent(mAid);
                agent->setPersistent(flag);
            });
            prop->setCallbacks(readCB, writeCB);
        }
        mProperties.push_back(prop);
    }
}
