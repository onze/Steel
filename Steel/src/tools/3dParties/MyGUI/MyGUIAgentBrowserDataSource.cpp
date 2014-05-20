
#include "tools/3dParties/MyGUI/MyGUIAgentBrowserDataSource.h"

#include <MyGUI.h>

#include "steelassert.h"
#include "Debug.h"
#include "tools/3dParties/MyGUI/TreeControl.h"
#include <Engine.h>
#include <Level.h>
#include <models/AgentManager.h>
#include <models/Agent.h>
#include <SignalManager.h>

namespace Steel
{

    MyGUIAgentBrowserDataSource::MyGUIAgentBrowserDataSource() : MyGUITreeControlDataSource(), SignalListener(),
        mEngine(nullptr), mLevel(nullptr),
        mSignals()
    {

    }

    MyGUIAgentBrowserDataSource::MyGUIAgentBrowserDataSource(const MyGUIAgentBrowserDataSource &o): MyGUITreeControlDataSource(o), SignalListener(o),
        mEngine(o.mEngine), mLevel(o.mLevel),
        mSignals(o.mSignals)
    {
    }

    MyGUIAgentBrowserDataSource::~MyGUIAgentBrowserDataSource()
    {
        shutdown();
    }

    MyGUIAgentBrowserDataSource &MyGUIAgentBrowserDataSource::operator=(const MyGUIAgentBrowserDataSource &o)
    {
        if(&o != this)
        {
            mEngine = o.mEngine;
            mLevel = o.mLevel;

            mSignals = o.mSignals;
        }

        return *this;
    }

    void MyGUIAgentBrowserDataSource::init(MyGUI::TreeControl *const control)
    {
        STEEL_WRONG_CODE_PATH(STEEL_METH_INTRO);
    }

    void MyGUIAgentBrowserDataSource::init(MyGUI::TreeControl *const control, Steel::Engine const *const engine)
    {
        MyGUITreeControlDataSource::init(control);
        mEngine = engine;
        mLevel = engine->level();
        STEEL_ASSERT(nullptr != mEngine);

        mSignals.levelSet = mEngine->getSignal(Engine::PublicSignal::levelSet);
        registerSignal(mSignals.levelSet);
        mSignals.levelUnset = mEngine->getSignal(Engine::PublicSignal::levelUnset);
        registerSignal(mSignals.levelUnset);

        mSignals.needRefresh = getSignal(PublicSignal::needRefresh);
        registerSignal(mSignals.needRefresh);

        mLevel->selectionMan()->addListener(this);
    }

    void MyGUIAgentBrowserDataSource::shutdown()
    {
        if(nullptr != mEngine)
        {
            unregisterAllSignals();
            mSignals.levelSet = INVALID_SIGNAL;
            mSignals.levelLoaded = INVALID_SIGNAL;
            mSignals.levelUnset = INVALID_SIGNAL;
            mSignals.needRefresh = INVALID_SIGNAL;

            mEngine = nullptr;
        }
    }

    Signal MyGUIAgentBrowserDataSource::getSignal(PublicSignal signal) const
    {

#define STEEL_AGENTBROWSER_GETSIGNAL_CASE(NAME) case NAME:return SignalManager::instance().toSignal("Steel::MyGUIAgentBrowserDataSource::"#NAME)

        switch(signal)
        {
                STEEL_AGENTBROWSER_GETSIGNAL_CASE(PublicSignal::needRefresh);
        }

#undef STEEL_AGENTBROWSER_GETSIGNAL_CASE
        return INVALID_SIGNAL;
    }

    void MyGUIAgentBrowserDataSource::onSignal(Signal signal, SignalEmitter *const src)
    {
        if(signal == mSignals.levelSet)
        {
            mLevel = mEngine->level();
            unregisterSignal(mSignals.levelLoaded);
            mSignals.levelLoaded = mLevel->getSignal(Level::PublicSignal::loaded);
            registerSignal(mSignals.levelLoaded);
        }
        else if(signal == mSignals.levelLoaded)
        {
            unregisterSignal(mSignals.levelLoaded);
            mSignals.levelLoaded = INVALID_SIGNAL;
            mLevel = mEngine->level();
            mControl->reset();
        }
        else if(signal == mSignals.levelUnset)
        {
            unregisterSignal(mSignals.levelLoaded); // may be unset before being loaded
            mSignals.levelLoaded = INVALID_SIGNAL;
            mLevel = nullptr;
            mControl->reset();
        }
        else if(signal == mSignals.needRefresh)
        {
            mControl->reset();
        }
    }

    void MyGUIAgentBrowserDataSource::notifyTreeNodePrepare(MyGUI::TreeControl *treeControl, MyGUI::TreeControlNode *node)
    {
        if(nullptr == mLevel)
            return;

        node->removeAll();

        AgentManager *agentMan = mLevel->agentMan();

        // populate tree root
        if(treeControl->getRoot() == node)
        {
            std::vector<AgentId> ids = agentMan->getAgentIds();
            std::sort(ids.begin(), ids.end());

            for(AgentId const & aid : ids)
            {
                Agent *agent = agentMan->getAgent(aid);

                if(nullptr == agent)
                {
                    Debug::error(STEEL_METH_INTRO, "agent ", aid, " not found. Skipped.").endl().breakHere();
                    continue;
                }

                Ogre::String nodeName = "Agent " + Ogre::StringConverter::toString(aid);
                MyGUI::TreeControlNode *child = new MyGUI::TreeControlNode(nodeName, "Folder");
                child->setPrepared(agent->modelsIds().size() == 0);
                child->setExpanded(false);

                ControlNodeDataType data;
                data.nodeType = ControlNodeDataType::NodeType::Agent;
                data.agentId = aid;
                child->setData(data);
                node->add(child);
            }
        }
        else
        {
            ControlNodeDataType const &agentData = *(node->getData<ControlNodeDataType>());

            // populate agent node
            if(ControlNodeDataType::NodeType::Agent == agentData.nodeType)
            {
                Agent *agent = agentMan->getAgent(agentData.agentId);

                if(nullptr == agent)
                {
                    Debug::error(STEEL_METH_INTRO, "agent ", agentData.agentId, " not found. Skipped.").endl().breakHere();
                    return;
                }

                for(ModelType modelType = (ModelType)((unsigned long) ModelType::FIRST + 1); modelType != ModelType::LAST; modelType = (ModelType)((unsigned long) modelType + 1))
                {
                    if(agent->hasModel(modelType))
                    {
                        ModelId mid = agent->modelId(modelType);
                        Ogre::String nodeName = toString(modelType) + " id: " + Ogre::StringConverter::toString(mid);
                        MyGUI::TreeControlNode *child = new MyGUI::TreeControlNode(nodeName, "Unknown");
                        child->setPrepared(true);
                        child->setExpanded(false);

                        ControlNodeDataType data;
                        data.nodeType = ControlNodeDataType::NodeType::Model;
                        data.modelId = mid;
                        child->setData(data);
                        node->add(child);
                    }
                }
            }
            else
            {
                // models nodes arent expandable.
            }
        }
    }

    void MyGUIAgentBrowserDataSource::onSelectionChanged(Selection const &selection)
    {
        if(nullptr == mControl)
            return;

        if(selection.size() != 1)
            return;

        AgentId const aid = selection.front();
        
        MyGUI::TreeControlNode *selectedNode = nullptr;

        // find node to be selected
        for(MyGUI::TreeControlNode *node : mControl->getRoot()->getChildren())
        {
            ControlNodeDataType const &data = *(node->getData<ControlNodeDataType>());

            if(data.nodeType != ControlNodeDataType::NodeType::Agent)
                continue;
            
            if(data.agentId != aid)
                continue;
            
            selectedNode = node;
            break;
        }

        if(nullptr != selectedNode)
            mControl->setSelection(selectedNode);
    }

}
