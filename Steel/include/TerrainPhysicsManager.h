#ifndef TERRAINPHYSICSMANAGER_H
#define TERRAINPHYSICSMANAGER_H

#include <map>

#include <OgreVector2.h>
#include <OgreFrameListener.h>

#include "TerrainManagerEventListener.h"
#include "TerrainManager.h"

namespace Ogre
{
    class Terrain;
}

namespace BtOgre
{
    class DebugDrawer;   
}

class btHeightfieldTerrainShape;
class btDynamicsWorld;
class btSequentialImpulseConstraintSolver;
class btCollisionDispatcher;
class btDefaultCollisionConfiguration;
class btAxisSweep3;
class btDefaultMotionState;
class btRigidBody;

namespace Steel
{
    class TerrainPhysicsManager:public TerrainManagerEventListener,Ogre::FrameListener
    {
        private:
            TerrainPhysicsManager(const TerrainPhysicsManager& other);
            virtual TerrainPhysicsManager& operator=(const TerrainPhysicsManager& other);
            virtual bool operator==(const TerrainPhysicsManager& other) const;
            class TerrainPhysics
            {
                public:
                    btHeightfieldTerrainShape *mTerrainShape;
                    btDefaultMotionState* mMotionState;
                    btRigidBody* mBody;
            };

        public:
            TerrainPhysicsManager(TerrainManager *terrainMan);
            virtual ~TerrainPhysicsManager();

            /// Instanciates a physics terrain. See bullet terrain demo for reference.
            bool createTerrainFor(Ogre::Terrain* ogreTerrain);
            /// Deletes a physics terrain
            bool removeTerrainFor(Ogre::Terrain* ogreTerrain);

            /// Adds a physics terrain's to the simulation
            bool activateTerrainFor(Ogre::Terrain *ogreTerrain);
            /// Removes a physics terrain's from the simulation
            bool deactivateTerrainFor(Ogre::Terrain *ogreTerrain);
            
            /// Called by Ogre once per frame
            bool frameRenderingQueued(const Ogre::FrameEvent &evt);

            /// Main loop iteration
            void update(float timestep);
            
            /// Inherited from TerrainManagerEventListener
            void onTerrainEvent(TerrainManager::LoadingState state);

            // getters
            inline btDynamicsWorld *world()
            {
                return mWorld;
            }

            /// Returns the PhysicsTerrain representing the given terrain.
            TerrainPhysics *getTerrainFor(Ogre::Terrain *ogreTerrain) const;
            
            /// Returns whether debug draw of physic shapes is activated
            bool getDebugDraw();
            
            /// De/activate debug draw of physic shapes
            void setDebugDraw(bool flag);
        protected:

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
            BtOgre::DebugDrawer *mDebugDrawer;

    };
}
#endif // TERRAINPHYSICSMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
