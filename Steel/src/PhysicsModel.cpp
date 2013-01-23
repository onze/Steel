#include "PhysicsModel.h"
#include "steeltypes.h"

namespace Steel
{
    PhysicsModel::PhysicsModel()
    {

    }

    void PhysicsModel::init()
    {

    }

    PhysicsModel::PhysicsModel(const PhysicsModel& other)
    {

    }

    PhysicsModel::~PhysicsModel()
    {

    }

    void PhysicsModel::cleanup()
    {

    }

    PhysicsModel& PhysicsModel::operator=(const PhysicsModel& other)
    {
        return *this;
    }
    
    ModelType PhysicsModel::modelType()
    {
        return MT_PHYSICS;
    }
    
    void PhysicsModel::toJson(Json::Value &object)
    {
        
    }
    
    bool PhysicsModel::fromJson(Json::Value &object)
    {
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
