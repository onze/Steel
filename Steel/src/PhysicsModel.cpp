#include "PhysicsModel.h"

#include <btBulletCollisionCommon.h>
#include <OgreEntity.h>

#include "steeltypes.h"
#include <Agent.h>
#include <OgreModel.h>
#include <BtOgreGP.h>
#include <BtOgrePG.h>

namespace Steel
{
    PhysicsModel::PhysicsModel():Model(),
        mWorld(NULL),mBody(NULL)
    {

    }

    void PhysicsModel::init(btDynamicsWorld *world,OgreModel *omodel)
    {
        mWorld=world;
        Ogre::String intro="PhysicsModel::init(): ";
        if(NULL==world)
        {
            Debug::error(intro)(" was given a NULL world. Aborted.").endl();
            return;
        }
        if(NULL==omodel)
        {
            Debug::error(intro)(" was given a NULL omodel. Aborted.").endl();
            return;
        }
        // Create shape
        BtOgre::StaticMeshToShapeConverter converter(omodel->entity());
        // You can also just use btSphereShape(1.2) or something.
        btSphereShape *mShape = converter.createSphere();

        //Calculate inertia.
        btScalar mass = 5;
        btVector3 inertia;
        mShape->calculateLocalInertia(mass, inertia);

        //Create BtOgre MotionState (connects Ogre and Bullet).
        BtOgre::RigidBodyState *state = new BtOgre::RigidBodyState(omodel->sceneNode());

        //Create the Body.
        mBody = new btRigidBody(mass, state, mShape, inertia);
        world->addRigidBody(mBody);
    }

    PhysicsModel::PhysicsModel(const PhysicsModel& o)
    {
        operator=(o);
    }
    
    PhysicsModel &PhysicsModel::operator=(const PhysicsModel& o)
    {
        Model::operator=(o);
        mWorld=o.mWorld;
        mBody=o.mBody;
        return *this;
    }

    PhysicsModel::~PhysicsModel()
    {
        if(!isFree())
            cleanup();
    }

    void PhysicsModel::cleanup()
    {
        if(NULL!=mWorld)
        {
            mWorld->removeRigidBody(mBody);
            mWorld=NULL;
        }
    }

    ModelType PhysicsModel::modelType()
    {
        return MT_PHYSICS;
    }

    void PhysicsModel::toJson(Json::Value &object)
    {

    }

    bool PhysicsModel::fromJson(Json::Value &object)
    {
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
