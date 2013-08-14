#ifndef STEEL_PHYSICSMODELMANAGER_H
#define STEEL_PHYSICSMODELMANAGER_H

class btCollisionObjectWrapper;
class btDynamicsWorld;

#include "steeltypes.h"
#include "_ModelManager.h"
#include "PhysicsModel.h"
#include <BulletCollision/NarrowPhaseCollision/btManifoldPoint.h>

namespace Steel
{
    class PhysicsModelManager: public _ModelManager<PhysicsModel>
    {

        public:
            PhysicsModelManager(Level *level, btDynamicsWorld *world);
            virtual ~PhysicsModelManager();

            inline ModelType modelType()
            {
                return MT_PHYSICS;
            }

            /// Initialize a new PhysicsModel according to data in the json serialization.
            bool fromSingleJson(Json::Value &model, ModelId &id);

            /// Initialize a new PhysicsModel and returns its identifier.
            ModelId newModel();

            /// Callback used to sync a PhysicsModel to its OgreModel upon linkage.
            bool onAgentLinkedToModel(Agent *agent, ModelId mid);
        protected:
            // not owned
            btDynamicsWorld *mWorld;
            //owned
    };
}
#endif // STEEL_PHYSICSMODELMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
