#include "PhysicsModel.h"

#include <btBulletCollisionCommon.h>
#include <OgreEntity.h>

#include "steeltypes.h"
#include <Agent.h>
#include <OgreModel.h>
#include <tools/StringUtils.h>
#include <BtOgreGP.h>
#include <BtOgrePG.h>

namespace Steel
{
    PhysicsModel::PhysicsModel():Model(),
        mWorld(NULL),mBody(NULL),mMass(.0f)
    {

    }

    void PhysicsModel::init(btDynamicsWorld* world, Steel::OgreModel* omodel)
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
        btVector3 inertia;
        mShape->calculateLocalInertia(mMass, inertia);

        //Create BtOgre MotionState (connects Ogre and Bullet).
        BtOgre::RigidBodyState *state = new BtOgre::RigidBodyState(omodel->sceneNode());

        //Create the Body.
        mBody = new btRigidBody(mMass, state, mShape, inertia);
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
        if(NULL!=mWorld && mBody!=NULL)
        {
            mWorld->removeRigidBody(mBody);
            mBody=NULL;
            mWorld=NULL;
        }
    }

    ModelType PhysicsModel::modelType()
    {
        return MT_PHYSICS;
    }

    void PhysicsModel::toJson(Json::Value &root)
    {
        root["mass"]=StringUtils::toJson(mMass);
    }

    bool PhysicsModel::fromJson(Json::Value &root)
    {
        Json::Value value;
        bool allWasFine = true;

        // gather it
        value = root["mass"];
        if (value.isNull() && !(allWasFine = false))
            Debug::error("in PhysicsModel::fromJson(): invalid field 'mass' (skipped).").endl();
        else
            mMass = Ogre::StringConverter::parseReal(value.asString(),0.f);

        if (!allWasFine)
        {
            Debug::error("json was:").endl()(root.toStyledString()).endl();
            Debug::error("deserialisation aborted.").endl();
            return false;
        }
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
