
#include "BTModel.h"

namespace Steel
{

    BTModel::BTModel()
    {
    }

    BTModel::BTModel ( const BTModel &m )
    {
    }

    BTModel::~BTModel()
    {
    }

    BTModel &BTModel::operator= ( const BTModel &m )
    {
        return *this;
    }

    bool BTModel::init(Steel::BTShapeStream* shapeStream)
    {
        switchShapeTo(shapeStream);
        return true;
    }

    bool BTModel::switchShapeTo(BTShapeStream* shapeStream)
    {
        mStateStream.clear();
        if(!mStateStream.init(shapeStream))
        {
            Debug::error("BTModel::switchShapeTo(): can't init stateStream from shapeStream: ")(shapeStream).endl();
            return false;
        }
        return true;
    }

    ModelType BTModel::modelType()
    {
        return MT_BT;
    }

    bool BTModel::fromJson ( Json::Value &node )
    {
        return true;
    }
    
    void BTModel::toJson ( Json::Value &node )
    {

    }

    void BTModel::cleanup()
    {
        mStateStream.clear();
    }
    
    void BTModel::update(float timestep)
    {
        
    }

} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
