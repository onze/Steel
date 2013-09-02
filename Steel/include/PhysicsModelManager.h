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

            /// Initialize a new PhysicsModel and returns its identifier.
            ModelId newModel();

            /// Callback used to sync a PhysicsModel to its OgreModel upon linkage.
            bool onAgentLinkedToModel(Agent *agent, ModelId mid);

            void update(float timestep);

            inline Level *level()
            {
                return mLevel;
            }
        protected:
            // not owned
            Level *mLevel;
            btDynamicsWorld *mWorld;
            //owned
    };
}
#endif // STEEL_PHYSICSMODELMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
