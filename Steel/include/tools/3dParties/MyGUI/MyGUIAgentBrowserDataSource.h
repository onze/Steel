#ifndef STEEL_MYGUIAGENTBROWSERDATASOURCE_H
#define STEEL_MYGUIAGENTBROWSERDATASOURCE_H

#include "steeltypes.h"
#include "tools/File.h"
#include "MyGUITreeControlDataSource.h"
#include "SignalListener.h"
#include "SelectionManager.h"

namespace MyGUI
{
    class TreeControl;
}

namespace Steel
{

    class Engine;

    struct MyGUIAgentBrowserNodeData
    {
        enum class NodeType
        {
            Agent = 1,
            Model
        };
        NodeType nodeType;

        AgentId agentId;
        /// INVALID_ID for agent nodes
        ModelId modelId;
        /// ModelType::LAST for agent nodes
        ModelType modelType;
    };

    class MyGUIAgentBrowserDataSource: public MyGUITreeControlDataSource, public SignalListener, public SelectionManager::Listener
    {
    public:
        typedef MyGUIAgentBrowserNodeData ControlNodeDataType;

        MyGUIAgentBrowserDataSource();
        MyGUIAgentBrowserDataSource(const MyGUIAgentBrowserDataSource &o);
        ~MyGUIAgentBrowserDataSource();
        MyGUIAgentBrowserDataSource &operator=(const MyGUIAgentBrowserDataSource &o);

        /// MyGUITreeControlDataSource interface
        void init(MyGUI::TreeControl *const control, Engine const *const engine);
        /// MyGUITreeControlDataSource interface
        virtual void shutdown() override;
        /// MyGUITreeControlDataSource interface
        virtual void notifyTreeNodePrepare(MyGUI::TreeControl *pTreeControl, MyGUI::TreeControlNode *pNode) override;

        /// SelectionManager::Listener interface
        virtual void onSelectionChanged(Selection const &selection) override;

        /// SignalListener interface
        virtual void onSignal(Signal signal, SignalEmitter *const src = nullptr);
        enum class PublicSignal : Signal
        {
            /// emit this signal to refresh the browser.
            needRefresh
        };
        Signal getSignal(PublicSignal signal) const;

    private:
        // Not enough parameters. Use the public one.
        virtual void init(MyGUI::TreeControl *const control) override;

        // not owned
        Engine const *mEngine;
        Level const *mLevel;

        // owned
        struct Signals
        {
            Signal levelSet = INVALID_SIGNAL;
            Signal levelLoaded = INVALID_SIGNAL;
            Signal levelUnset = INVALID_SIGNAL;
            Signal needRefresh = INVALID_SIGNAL;
            Signal agentCreated = INVALID_SIGNAL;
        };
        Signals mSignals;
    };
}

#endif // STEEL_MYGUIAGENTBROWSERDATASOURCE_H
