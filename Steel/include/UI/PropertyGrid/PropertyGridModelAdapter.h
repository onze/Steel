#ifndef STEEL_PROPERTYGRIDMODELADAPTER_H
#define STEEL_PROPERTYGRIDMODELADAPTER_H

#include "steeltypes.h"
#include "PropertyGridAdapter.h"

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
}

#endif // STEEL_PROPERTYGRIDMODELADAPTER_H
