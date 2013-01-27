#ifndef PHYSICSMODEL_H
#define PHYSICSMODEL_H

#include <BulletDynamics/Dynamics/btRigidBody.h>

#include "Model.h"

class btDynamicsWorld;

namespace Steel
{
    class Agent;
    class OgreModel;
    class PhysicsModel:public Model
    {

        public:
            PhysicsModel();
            void init(btDynamicsWorld *world,OgreModel *omodel);
            PhysicsModel(const PhysicsModel& other);
            virtual PhysicsModel& operator=(const PhysicsModel& other);
            virtual ~PhysicsModel();
            virtual ModelType modelType();

            ///serialize itself into the given Json object
            virtual void toJson(Json::Value &object);

            ///deserialize itself from the given Json object. return true is successful.
            virtual bool fromJson(Json::Value &object);
        protected:
            virtual void cleanup();
            //not owned
            //owned
            btRigidBody* mBody;
    };
}
#endif // PHYSICSMODEL_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
