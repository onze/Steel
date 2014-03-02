#include <BtOgreGP.h>
#include <BtOgrePG.h>
#include <BtOgreExtras.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <BulletCollision/BroadphaseCollision/btBroadphaseProxy.h>

#include "TerrainPhysicsManager.h"
#include "Debug.h"
#include "TerrainManager.h"
#include "Level.h"
#include "OgreTerrain.h"


namespace Steel
{
    TerrainPhysicsManager::TerrainPhysicsManager(TerrainManager *terrainMan):
        TerrainManagerEventListener(), Ogre::FrameListener(),
        mTerrainMan(nullptr), mTerrains(std::map<Ogre::Terrain *, TerrainPhysics *>()),
        mWorld(nullptr), mSolver(nullptr), mDispatcher(nullptr), mCollisionConfig(nullptr), mBroadphase(nullptr),
        mDebugDrawer(nullptr)
    {
        mTerrainMan = terrainMan;
        // http://www.bulletphysics.org/Bullet/phpBB3/viewtopic.php?p=16731#p16731
        // Use the btDbvtBroadphase instead of btAxisSweep3 to increase performance of adding/removing to the world
        mBroadphase = new btAxisSweep3(btVector3(-10000, -10000, -10000), btVector3(10000, 10000, 10000), 1024);
//         mBroadphase=new btDbvtBroadphase();
        mCollisionConfig = new btDefaultCollisionConfiguration();
        mDispatcher = new btCollisionDispatcher(mCollisionConfig);
        mSolver = new btSequentialImpulseConstraintSolver();


        mWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
        mWorld->setGravity(BtOgre::Convert::toBullet(mTerrainMan->level()->gravity()));

        terrainMan->addTerrainManagerEventListener(this);

        mDebugDrawer = new BtOgre::DebugDrawer(terrainMan->sceneManager()->getRootSceneNode(), mWorld);
        mDebugDrawer->setDebugMode(false);
        mWorld->setDebugDrawer(mDebugDrawer);
    }

    void TerrainPhysicsManager::setWorldGravity(Ogre::Vector3 const &gravity)
    {
        if(nullptr != mWorld)
            mWorld->setGravity(BtOgre::Convert::toBullet(gravity));
    }

    TerrainPhysicsManager::TerrainPhysicsManager(const TerrainPhysicsManager &o)
    {
        Debug::error("not implemented").endl().breakHere();
    }

    TerrainPhysicsManager::~TerrainPhysicsManager()
    {
        if(nullptr != mWorld)
        {
            if(getDebugDraw())
                setDebugDraw(false);

            if(nullptr != mDebugDrawer)
            {
                mWorld->setDebugDrawer(nullptr);

                if(!Ogre::ResourceGroupManager::getSingleton().resourceGroupExists("BtOgre"))
                    Ogre::ResourceGroupManager::getSingleton().createResourceGroup("BtOgre");

                delete mDebugDrawer;
                mDebugDrawer = nullptr;
            }

            while(mTerrains.size() > 0)
                removeTerrainFor((*mTerrains.begin()).first);

            //remove the rigidbodies from the dynamics world and delete them
            for(int i = mWorld->getNumCollisionObjects() - 1; i >= 0 ; i--)
            {
                btCollisionObject *obj = mWorld->getCollisionObjectArray()[i];
                btRigidBody *body = btRigidBody::upcast(obj);

                if(body && body->getMotionState())
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
            mWorld = nullptr;
            mSolver = nullptr;
            mDispatcher = nullptr;
            mBroadphase = nullptr;
            mCollisionConfig = nullptr;
        }

        mTerrainMan = nullptr;
    }

    TerrainPhysicsManager &TerrainPhysicsManager::operator=(const TerrainPhysicsManager &o)
    {
        return *this;
    }

    bool TerrainPhysicsManager::operator==(const TerrainPhysicsManager &o) const
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
    }

    void TerrainPhysicsManager::onTerrainEvent(TerrainManager::LoadingState state)
    {
        switch(state)
        {
            case TerrainManager::LoadingState::READY:
                break;

            default:
                break;
        }
    }

    TerrainPhysicsManager::TerrainPhysics *TerrainPhysicsManager::getTerrainFor(Ogre::Terrain *ogreTerrain) const
    {
        auto it = mTerrains.find(ogreTerrain);

        if(it == mTerrains.end())
            return nullptr;
        else
            return (*it).second;
    }

    bool TerrainPhysicsManager::createTerrainFor(Ogre::Terrain *ogreTerrain)
    {
        TerrainPhysics *terrain = getTerrainFor(ogreTerrain);

        if(nullptr != terrain)
        {
            Debug::error("TerrainPhysicsManager::createTerrain(): TerrainPhysics already exists ");
            Debug::error("at position ")(ogreTerrain->getPosition())(". Aborted.").endl();
            return false;
        }

        // some const
        int side = ogreTerrain->getSize();
        auto minHeight = ogreTerrain->getMinHeight(), maxHeight = ogreTerrain->getMaxHeight();
        float metersBetweenVertices = ogreTerrain->getWorldSize() / (ogreTerrain->getSize() - 1);

        // create and save the new terrain
        terrain = new TerrainPhysics();
        mTerrains[ogreTerrain] = terrain;

        // give it a bullet representation
        terrain->mHeightfieldData = new float[side * side];
        terrain->mTerrainShape = new btHeightfieldTerrainShape(side, side,
                terrain->mHeightfieldData,
                1.f,
                minHeight, maxHeight,
                1, PHY_FLOAT, false);
        terrain->mTerrainShape->setUseDiamondSubdivision(true);
        terrain->mTerrainShape->setLocalScaling(btVector3(metersBetweenVertices, 1.f, metersBetweenVertices));

        btTransform transform = getOgreTerrainTransform(ogreTerrain);
        terrain->mMotionState = new btDefaultMotionState(transform);


        btScalar mass(0.);
        btVector3 localInertia(0, 0, 0);

        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,
                terrain->mMotionState,
                terrain->mTerrainShape,
                localInertia);
        terrain->mBody = new btRigidBody(rbInfo);
        {
            auto flags = btCollisionObject::CF_STATIC_OBJECT | btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT;
            terrain->mBody->setCollisionFlags(terrain->mBody->getCollisionFlags() | flags);
        }

        updateHeightmap(ogreTerrain, terrain);

        //add the body to the dynamics world
        mWorld->addRigidBody(terrain->mBody);
        return true;
    }

    btTransform TerrainPhysicsManager::getOgreTerrainTransform(Ogre::Terrain *oterrain)
    {
        auto minHeight = oterrain->getMinHeight(), maxHeight = oterrain->getMaxHeight();
        btTransform transform;
        transform.setIdentity();
        // set origin to middle of heightfield
        Ogre::Vector3 pos = oterrain->getPosition();
        pos.y = (maxHeight + minHeight) / 2.f;
        transform.setOrigin(BtOgre::Convert::toBullet(pos));
        transform.setRotation(BtOgre::Convert::toBullet(Ogre::Quaternion::IDENTITY));
        return transform;
    }

    void TerrainPhysicsManager::updateHeightmap(Ogre::Terrain *oterrain)
    {
        TerrainPhysics *pterrain = getTerrainFor(oterrain);

        if(nullptr != pterrain)
            updateHeightmap(oterrain, pterrain);
    }

    void TerrainPhysicsManager::updateHeightmap(Ogre::Terrain *oterrain, TerrainPhysics *pterrain)
    {
        // We need to mirror the ogre-height-data along the z axis
        // This is related to how Ogre and Bullet differ in heighmap storing

        auto minHeight = oterrain->getMinHeight(), maxHeight = oterrain->getMaxHeight();
        int side = oterrain->getSize();

        float *physicsData = pterrain->mHeightfieldData;
        float *pTerrainHeightData = oterrain->getHeightData();

        for(int i = 0; i < side; ++i)
        {
            memcpy(physicsData + side * i,
                   pTerrainHeightData + side * (side - i - 1),
                   sizeof(float) * (side));
        }

        // set origin to middle of heightfield
        btTransform transform;
        transform.setIdentity();
        Ogre::Vector3 pos = oterrain->getPosition();
        pos.y = (maxHeight + minHeight) / 2.f;
        transform.setOrigin(BtOgre::Convert::toBullet(pos));
        transform.setRotation(BtOgre::Convert::toBullet(Ogre::Quaternion::IDENTITY));
        pterrain->mMotionState->setWorldTransform(transform);

        // wake up rigbodies in the area
        pterrain->mBody->activate(true);
        auto objects = mWorld->getCollisionObjectArray();

        for(auto i = 0; i < objects.size(); ++i)
            objects[i]->activate(true);
    }
    bool TerrainPhysicsManager::removeTerrainFor(Ogre::Terrain *ogreTerrain)
    {
        auto it = mTerrains.find(ogreTerrain);

        if(it == mTerrains.end())
        {
            Debug::error("TerrainPhysicsManager::removeTerrainFor(): no such TerrainPhysics. Aborted.").endl();
            return false;
        }

        TerrainPhysics *terrain = (*it).second;

        if(nullptr != terrain->mBody)
        {
            mWorld->removeRigidBody(terrain->mBody);

            if(nullptr != terrain->mBody->getMotionState())
                delete terrain->mBody->getMotionState();

            delete terrain->mBody;
        }

        delete terrain;
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
        if(nullptr != mWorld)
            mWorld->stepSimulation(timestep, 10);
    }

    bool TerrainPhysicsManager::frameRenderingQueued(const Ogre::FrameEvent &evt)
    {
        if(nullptr != mDebugDrawer)
        {
            mDebugDrawer->step();

            if(!mDebugDrawer->getDebugMode())
            {
                Ogre::Root::getSingletonPtr()->removeFrameListener(this);
                Debug::log("TerrainPhysicsManager::setDebugDraw(false)").endl();
            }
        }

        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
