#ifndef TERRAINPHYSICSMANAGER_H
#define TERRAINPHYSICSMANAGER_H
#include <map>
#include <OgreVector2.h>

#include "TerrainManagerEventListener.h"
#include "TerrainManager.h"

namespace Ogre
{
    class Terrain;
}

class btHeightfieldTerrainShape;
class btDynamicsWorld;
class btSequentialImpulseConstraintSolver;
class btCollisionDispatcher;
class btDefaultCollisionConfiguration;
class btAxisSweep3;

namespace Steel
{
    class TerrainPhysicsManager:public TerrainManagerEventListener
    {
        private:
            TerrainPhysicsManager(const TerrainPhysicsManager& other);
            virtual TerrainPhysicsManager& operator=(const TerrainPhysicsManager& other);
            virtual bool operator==(const TerrainPhysicsManager& other) const;
            class TerrainPhysics
            {
                public:
                    btHeightfieldTerrainShape *mTerrainShape;
            };

        public:
            TerrainPhysicsManager(TerrainManager *terrainMan);
            virtual ~TerrainPhysicsManager();

            /// Instanciates a physics terrain
            bool createTerrainFor(Ogre::Terrain* ogreTerrain);
            /// Deletes a physics terrain
            bool removeTerrainFor(Ogre::Terrain* ogreTerrain);

            /// Adds a physics terrain's to the simulation
            bool activateTerrainFor(Ogre::Terrain *ogreTerrain);
            /// Removes a physics terrain's from the simulation
            bool deactivateTerrainFor(Ogre::Terrain *ogreTerrain);

            /// Main loop iteration
            void update(float timestep);
            
            /// Inherited from TerrainManagerEventListener
            void onTerrainEvent(TerrainManager::LoadingState state);

            // getters
            inline btDynamicsWorld *world()
            {
                return mWorld;
            }

        protected:

            /// Returns the PhysicsTerrain representing the given terrain.
            TerrainPhysics *getTerrainFor(Ogre::Terrain *ogreTerrain);

            // not owned
            /// owner
            TerrainManager *mTerrainMan;

            // owned
            std::map<Ogre::Terrain *,TerrainPhysics *> mTerrains;

            btDynamicsWorld *mWorld;
            btSequentialImpulseConstraintSolver *mSolver;
            btCollisionDispatcher *mDispatcher;
            btDefaultCollisionConfiguration *mCollisionConfig;
            btAxisSweep3 *mBroadphase;

    };
}
#endif // TERRAINPHYSICSMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
