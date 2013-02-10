#ifndef STEEL_PHYSICSMODELMANAGER_H
#define STEEL_PHYSICSMODELMANAGER_H

class btDynamicsWorld;

#include "steeltypes.h"
#include "_ModelManager.h"
#include "PhysicsModel.h"

namespace Steel
{
    class PhysicsModelManager:public _ModelManager<PhysicsModel>
    {

        public:
            PhysicsModelManager(Level *level,btDynamicsWorld *world);
            virtual ~PhysicsModelManager();

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
            
            virtual bool onAgentLinkedToModel(AgentId aid, ModelId mid);
        protected:
            // not owned
            btDynamicsWorld *mWorld;
            //owned
    };
}
#endif // STEEL_PHYSICSMODELMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
