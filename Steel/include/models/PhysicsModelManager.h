#ifndef STEEL_PHYSICSMODELMANAGER_H
#define STEEL_PHYSICSMODELMANAGER_H

class btCollisionObjectWrapper;
class btDynamicsWorld;
class btGhostPairCallback;

#include "steeltypes.h"
#include "_ModelManager.h"
#include "PhysicsModel.h"

namespace Steel
{
    class PhysicsModelManager: public _ModelManager<PhysicsModel>
    {

        public:
            PhysicsModelManager(Level *level, btDynamicsWorld *world);
            virtual ~PhysicsModelManager();

            inline ModelType modelType() {return ModelType::PHYSICS;}

            /// Initialize a new PhysicsModel and returns its identifier.
            ModelId newModel();

            /// Callback used to sync a PhysicsModel to its OgreModel upon linkage.
            bool onAgentLinkedToModel(Agent *agent, ModelId mid);

            void update(float timestep);
        protected:
            // not owned
            btDynamicsWorld *mWorld;
            //owned
            btGhostPairCallback* mbulletGhostPairCallback;
    };
}
#endif // STEEL_PHYSICSMODELMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
