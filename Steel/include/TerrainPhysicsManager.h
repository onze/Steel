#ifndef STEEL_TERRAINPHYSICSMANAGER_H
#define STEEL_TERRAINPHYSICSMANAGER_H

#include <map>

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

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
                    TerrainPhysics():
                        mTerrainShape(NULL),mMotionState(NULL),mBody(NULL),mHeightfieldData(NULL)
                    {}
                    btHeightfieldTerrainShape *mTerrainShape;
                    btDefaultMotionState* mMotionState;
                    btRigidBody* mBody;
                    float *mHeightfieldData;
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

            btTransform getOgreTerrainTransform(Ogre::Terrain * oterrain);

            /// Called by Ogre once per frame
            bool frameRenderingQueued(const Ogre::FrameEvent &evt);

            /// Inherited from TerrainManagerEventListener
            void onTerrainEvent(TerrainManager::LoadingState state);

            /// Returns whether debug draw of physic shapes is activated
            bool getDebugDraw();

            /// Main loop iteration
            void update(float timestep);

            /// Update height values
            void updateHeightmap(Ogre::Terrain* terrain);

            // getters
            inline btDynamicsWorld *world()
            {
                return mWorld;
            }

            /// Returns the PhysicsTerrain representing the given terrain.
            TerrainPhysics *getTerrainFor(Ogre::Terrain *ogreTerrain) const;

            // setters
            /// De/activate debug draw of physic shapes
            void setDebugDraw(bool flag);

        protected:
            void updateHeightmap(Ogre::Terrain *oterrain, TerrainPhysics* pterrain);
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
#endif // STEEL_TERRAINPHYSICSMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
