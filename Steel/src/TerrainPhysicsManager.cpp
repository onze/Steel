
#include <BtOgreGP.h>
#include <BtOgrePG.h>
#include <BtOgreExtras.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include "TerrainPhysicsManager.h"
#include <Debug.h>
#include <TerrainManager.h>
#include <OgreTerrain.h>


namespace Steel
{

    TerrainPhysicsManager::TerrainPhysicsManager(TerrainManager *terrainMan):
        TerrainManagerEventListener(),Ogre::FrameListener(),
        mTerrainMan(NULL),mTerrains(std::map<Ogre::Terrain *,TerrainPhysics *>()),
        mWorld(NULL),mSolver(NULL),mDispatcher(NULL),mCollisionConfig(NULL),mBroadphase(NULL),
        mDebugDrawer(NULL)
    {
        mTerrainMan=terrainMan;
        mBroadphase = new btAxisSweep3(btVector3(-10000,-10000,-10000), btVector3(10000,10000,10000), 1024);
        mCollisionConfig = new btDefaultCollisionConfiguration();
        mDispatcher = new btCollisionDispatcher(mCollisionConfig);
        mSolver = new btSequentialImpulseConstraintSolver();

        mWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
        mWorld->setGravity(btVector3(0,-9.8,0));

        terrainMan->addTerrainManagerEventListener(this);

        mDebugDrawer = new BtOgre::DebugDrawer(terrainMan->sceneManager()->getRootSceneNode(), mWorld);
        mDebugDrawer->setDebugMode(false);
        mWorld->setDebugDrawer(mDebugDrawer);
    }

    TerrainPhysicsManager::TerrainPhysicsManager(const TerrainPhysicsManager& o)
    {
    }

    TerrainPhysicsManager::~TerrainPhysicsManager()
    {
        if(NULL!=mWorld)
        {
            if(getDebugDraw())
                setDebugDraw(false);

            if(NULL!=mDebugDrawer)
            {
                mWorld->setDebugDrawer(NULL);
                delete mDebugDrawer;
                mDebugDrawer=NULL;
            }
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

    bool TerrainPhysicsManager::getDebugDraw()
    {
        return mDebugDrawer->getDebugMode();
    }

    void TerrainPhysicsManager::setDebugDraw(bool flag)
    {
        mDebugDrawer->setDebugMode(flag);
        if(flag)
        {
            Ogre::Root::getSingletonPtr()->addFrameListener(this);
            Debug::log("TerrainPhysicsManager::setDebugDraw(true)").endl();
        }
        else
        {
            Ogre::Root::getSingletonPtr()->removeFrameListener(this);
            Debug::log("TerrainPhysicsManager::setDebugDraw(false)").endl();
        }
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

    TerrainPhysicsManager::TerrainPhysics *TerrainPhysicsManager::getTerrainFor(Ogre::Terrain *ogreTerrain) const
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
        // create it
        auto minHeight=ogreTerrain->getMinHeight(),maxHeight=ogreTerrain->getMaxHeight();
        terrain=new TerrainPhysics();

        // >>> We need to mirror the ogre-height-data along the z axis first!
        // This is related to how Ogre and Bullet differ in heighmap storing
        float *pTerrainHeightData = ogreTerrain->getHeightData();
        float pTerrainHeightDataConvert[side * side];
        for(int i = 0; i < side; ++i)
        {
            memcpy(pTerrainHeightDataConvert + side* i,
                   pTerrainHeightData + side* (side- i - 1),
                   sizeof(float)*(side));
        }
        // <<< End of conversion

        terrain->mTerrainShape=new btHeightfieldTerrainShape(side,side,
                pTerrainHeightDataConvert,
                1.f,
                minHeight,maxHeight,
                1, PHY_FLOAT, false);
        terrain->mTerrainShape->setUseDiamondSubdivision(true);
        float metersBetweenVertices = ogreTerrain->getWorldSize()/(ogreTerrain->getSize()-1);
        terrain->mTerrainShape->setLocalScaling(btVector3(metersBetweenVertices,1.f,metersBetweenVertices));

        // save it
        mTerrains[ogreTerrain]=terrain;
        // set origin to middle of heightfield
        btTransform transform;
        transform.setIdentity();
        Ogre::Vector3 pos=ogreTerrain->getPosition();
        pos.y=(maxHeight+minHeight)/2.f;
        transform.setOrigin(BtOgre::Convert::toBullet(pos));
        transform.setRotation(BtOgre::Convert::toBullet(Ogre::Quaternion::IDENTITY));
        btScalar mass(0.);
        btVector3 localInertia(0,0,0);

        terrain->mMotionState = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,
                terrain->mMotionState,
                terrain->mTerrainShape,
                localInertia);
        terrain->mBody= new btRigidBody(rbInfo);
        terrain->mBody->setCollisionFlags(terrain->mBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

        //add the body to the dynamics world
        mWorld->addRigidBody(terrain->mBody);
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
        if(NULL!=mWorld)
            mWorld->stepSimulation(timestep,7);
    }

    bool TerrainPhysicsManager::frameRenderingQueued(const Ogre::FrameEvent &evt)
    {
        if(NULL!=mDebugDrawer)
            mDebugDrawer->step();
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
