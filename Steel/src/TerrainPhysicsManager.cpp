#include "TerrainPhysicsManager.h"
#include <Debug.h>
#include <TerrainManager.h>

#include <BtOgreGP.h>
#include <BtOgrePG.h>
#include <OgreTerrain.h>

#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

namespace Steel
{

    TerrainPhysicsManager::TerrainPhysicsManager(TerrainManager *terrainMan):TerrainManagerEventListener(),
        mTerrainMan(NULL),mTerrains(std::map<Ogre::Terrain *,TerrainPhysics *>()),
        mWorld(NULL),mSolver(NULL),mDispatcher(NULL),mCollisionConfig(NULL),mBroadphase(NULL)
    {
        mTerrainMan=terrainMan;
        mBroadphase = new btAxisSweep3(btVector3(-10000,-10000,-10000), btVector3(10000,10000,10000), 1024);
        mCollisionConfig = new btDefaultCollisionConfiguration();
        mDispatcher = new btCollisionDispatcher(mCollisionConfig);
        mSolver = new btSequentialImpulseConstraintSolver();

        mWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
        mWorld->setGravity(btVector3(0,-9.8,0));

        terrainMan->addTerrainManagerEventListener(this);
    }

    TerrainPhysicsManager::TerrainPhysicsManager(const TerrainPhysicsManager& o)
    {
    }

    TerrainPhysicsManager::~TerrainPhysicsManager()
    {
        if(NULL!=mWorld)
        {
            //remove the rigidbodies from the dynamics world and delete them
            for (int i=mWorld->getNumCollisionObjects()-1; i>=0 ; i--)
            {
                btCollisionObject* obj = mWorld->getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(obj);
                if (body && body->getMotionState())
                {
                    delete body->getMotionState();
                }
                mWorld->removeCollisionObject(obj);
                delete obj;
            }
            delete mWorld;
            delete mSolver;
            delete mDispatcher;
            delete mBroadphase;
            delete mCollisionConfig;
            mWorld=NULL;
            mSolver=NULL;
            mDispatcher=NULL;
            mBroadphase=NULL;
            mCollisionConfig=NULL;
        }
        mTerrainMan=NULL;
    }

    TerrainPhysicsManager& TerrainPhysicsManager::operator=(const TerrainPhysicsManager& o)
    {
        return *this;
    }

    bool TerrainPhysicsManager::operator==(const TerrainPhysicsManager& o) const
    {
        return false;
    }

    void TerrainPhysicsManager::onTerrainEvent(TerrainManager::LoadingState state)
    {
        switch(state)
        {
            case TerrainManager::READY:
                //let's update the heightmap bullet thingy here

                break;
            default:
                break;
        }
    }

    TerrainPhysicsManager::TerrainPhysics *TerrainPhysicsManager::getTerrainFor(Ogre::Terrain *ogreTerrain)
    {
        auto it=mTerrains.find(ogreTerrain);
        if(it==mTerrains.end())
            return NULL;
        else
            return (*it).second;
    }

    bool TerrainPhysicsManager::createTerrainFor(Ogre::Terrain *ogreTerrain)
    {
        TerrainPhysics *terrain=getTerrainFor(ogreTerrain);
        if(NULL!=terrain)
        {
            Debug::error("TerrainPhysicsManager::createTerrain(): PhysicsTerrain already exists ");
            Debug::error("at position ")(ogreTerrain->getPosition())(". Aborted.").endl();
            return false;
        }

        int side=ogreTerrain->getSize();
        terrain->mTerrainShape=new btHeightfieldTerrainShape(side,side,
                ogreTerrain->getHeightData(),
                1.f,
                ogreTerrain->getMinHeight(),ogreTerrain->getMaxHeight(),
                1, PHY_FLOAT, false);

        mTerrains[ogreTerrain]=terrain;
        return true;
    }

    bool TerrainPhysicsManager::removeTerrainFor(Ogre::Terrain *ogreTerrain)
    {
        auto it=mTerrains.find(ogreTerrain);
        if(it==mTerrains.end())
        {
            Debug::error("TerrainPhysicsManager::removeTerrain(): no such PhysicsTerrain. Aborted.").endl();
            return false;
        }
        mTerrains.erase(it);
        return true;
    }

    bool TerrainPhysicsManager::activateTerrainFor(Ogre::Terrain *ogreTerrain)
    {
        return true;
    }

    bool TerrainPhysicsManager::deactivateTerrainFor(Ogre::Terrain *ogreTerrain)
    {
        return true;
    }

    void TerrainPhysicsManager::update(float timestep)
    {
        if(mWorld)
            mWorld->stepSimulation(timestep,7);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
