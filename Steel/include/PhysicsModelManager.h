#ifndef PHYSICSMODELMANAGER_H
#define PHYSICSMODELMANAGER_H

#include <btBulletDynamicsCommon.h>

#include "steeltypes.h"
#include "_ModelManager.h"
#include "PhysicsModel.h"

namespace Steel
{
    class PhysicsModelManager:public _ModelManager<PhysicsModel>
    {

        public:
            PhysicsModelManager(Level *level);
            virtual ~PhysicsModelManager();

            ///
            void init();

            /**
             * batched call to fromSingleJson.
             */
            std::vector<ModelId> fromJson(Json::Value &models);

            /**
             * initialize a new PhysicsModel according to data in the json serialization.
             */
            ModelId fromSingleJson(Json::Value &model);

            /// Initialize a new PhysicsModel and returns its identifier.
            ModelId newModel();

            virtual ModelType modelType()
            {
                return MT_PHYSICS;
            };
            
            /// Main loop iteration
            void update(float timestep);
            
        protected:
            virtual bool onAgentLinked(AgentId aid, ModelId mid);
            // not owned
            //owned
            btDynamicsWorld *mWorld;
            btSequentialImpulseConstraintSolver *mSolver;
            btCollisionDispatcher *mDispatcher;
            btDefaultCollisionConfiguration *mCollisionConfig;
            btAxisSweep3 *mBroadphase;
    };
}
#endif // PHYSICSMODELMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
