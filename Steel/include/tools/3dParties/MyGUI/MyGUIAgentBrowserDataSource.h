#ifndef STEEL_MYGUIAGENTBROWSERDATASOURCE_H
#define STEEL_MYGUIAGENTBROWSERDATASOURCE_H

#include "steeltypes.h"
#include "tools/File.h"
#include "MyGUIFileTreeDataSource.h"
#include "SignalListener.h"

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
        
        union
        {
            ModelId modelId;
            AgentId agentId;
        };
    };
    
    class MyGUIAgentBrowserDataSource: public MyGUITreeControlDataSource, public SignalListener
    {
    public:
        typedef MyGUIAgentBrowserNodeData ControlNodeDataType;
        
        MyGUIAgentBrowserDataSource();
        MyGUIAgentBrowserDataSource(const MyGUIAgentBrowserDataSource &o);
        ~MyGUIAgentBrowserDataSource();
        MyGUIAgentBrowserDataSource &operator=(const MyGUIAgentBrowserDataSource &o);
        
        void init(MyGUI::TreeControl *const control, Engine const *const engine);
        virtual void shutdown() override;
        virtual void notifyTreeNodePrepare(MyGUI::TreeControl *pTreeControl, MyGUI::TreeControlNode *pNode) override;
        
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
        Engine const* mEngine;
        Level const* mLevel;
        
        // owned
        struct Signals
        {
            Signals():
            levelSet(INVALID_SIGNAL),
            levelLoaded(INVALID_SIGNAL),
            levelUnset(INVALID_SIGNAL),
            needRefresh(INVALID_SIGNAL)
            {}
            
            Signal levelSet;
            Signal levelLoaded;
            Signal levelUnset;
            Signal needRefresh;
        };
        Signals mSignals;
    };
}

#endif // STEEL_MYGUIAGENTBROWSERDATASOURCE_H
