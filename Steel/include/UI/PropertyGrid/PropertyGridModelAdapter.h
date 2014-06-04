#ifndef STEEL_PROPERTYGRIDMODELADAPTER_H
#define STEEL_PROPERTYGRIDMODELADAPTER_H

#include "steeltypes.h"
#include "PropertyGridAdapter.h"
#include "SignalListener.h"
#include "SignalEmitter.h"

namespace Steel
{

    class Level;

    class PropertyGridModelAdapter : public PropertyGridAdapter
    {
        PropertyGridModelAdapter() = delete;
    public:
        PropertyGridModelAdapter(Level *const level, AgentId aid, ModelId mid);
        virtual ~PropertyGridModelAdapter();

        virtual void buildProperties() override;
        Signal getSignal(PropertyGridAdapter::PublicSignal signal) const override {return PropertyGridAdapter::getSignal(signal);}

    protected:
        // not owned
        Level *mLevel = nullptr;
        AgentId mAid = INVALID_ID;
        ModelId mMid = INVALID_ID;
    };

    class PropertyGridPhysicsModelAdapter: public PropertyGridModelAdapter
    {
    public:
        PropertyGridPhysicsModelAdapter(Level *const level, AgentId aid, ModelId mid): PropertyGridModelAdapter(level, aid, mid) {};
        ~PropertyGridPhysicsModelAdapter() {};

        void buildProperties() override;
        
    protected:
        StringVector const &boundingShapeValues();
    };
    
    class PropertyGridBTModelAdapter: public PropertyGridModelAdapter
    {
    public:
        PropertyGridBTModelAdapter(Level *const level, AgentId aid, ModelId mid): PropertyGridModelAdapter(level, aid, mid) {};
        ~PropertyGridBTModelAdapter() {};
        
        void buildProperties() override;
    };
    
    class PropertyGridBlackboardModelAdapter: public PropertyGridModelAdapter
    {
    public:
        PropertyGridBlackboardModelAdapter(Level *const level, AgentId aid, ModelId mid): PropertyGridModelAdapter(level, aid, mid) {};
        ~PropertyGridBlackboardModelAdapter() {};
        
        void buildProperties() override;
        
        Signal getSignal(PropertyGridAdapter::PublicSignal signal) const override;
        void onSignal(Signal signal, SignalEmitter *const source) override;
    };
}

#endif // STEEL_PROPERTYGRIDMODELADAPTER_H
