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
        mWorld(NULL),mBody(NULL),mMass(.0f),mIsKinematics(false),mStates(std::stack<bool>())
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
        btConvexHullShape *mShape= converter.createConvex();
        // You can also just use btSphereShape(1.2) or something.
//         btSphereShape *mShape = converter.createSphere();1

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
        mMass=o.mMass;
        mIsKinematics=o.mIsKinematics;
        mStates=o.mStates;
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

    bool PhysicsModel::popState()
    {
        if(!mStates.empty())
        {
            if(mStates.top()!=mIsKinematics)
            {
                mIsKinematics?toRigidBody():toKinematics();
            }
            mStates.pop();
        }
        return mIsKinematics;
    }

    void PhysicsModel::pushState()
    {
        mStates.push(mIsKinematics);
    }

    void PhysicsModel::toKinematics()
    {
        mWorld->removeRigidBody(mBody);
        // http://www.oogtech.org/content/2011/09/07/bullet-survival-kit-4-the-motion-state/
        // This flag tells the engine that the object is kinematic –
        // so it shouldn’t move under the influence of gravity or because of colliding with other objects
        mBody->setCollisionFlags(mBody->getCollisionFlags()|btCollisionObject::CF_KINEMATIC_OBJECT);
        mBody->setActivationState(DISABLE_DEACTIVATION);
        auto zero=btVector3(0.f,.0f,.0f);
        mBody->setLinearVelocity(zero);
        mBody->setAngularVelocity(zero);
        mIsKinematics=true;
        mWorld->addRigidBody(mBody);
    }

    void PhysicsModel::toRigidBody()
    {
        mWorld->removeRigidBody(mBody);
        mBody->setActivationState(ACTIVE_TAG);
        mBody->setCollisionFlags(mBody->getCollisionFlags()&~btCollisionObject::CF_KINEMATIC_OBJECT);
        mIsKinematics=false;
        mWorld->addRigidBody(mBody);
    }

    void PhysicsModel::move(const Ogre::Vector3 &dpos)
    {
        bool switchBack=false;
        if(!mIsKinematics && (switchBack=true))toKinematics();

        btTransform ts=mBody->getWorldTransform();
        ts.setOrigin(ts.getOrigin()+BtOgre::Convert::toBullet(dpos));
        mBody->getMotionState()->setWorldTransform(ts);
        mBody->setCenterOfMassTransform(ts);

        if(switchBack)toRigidBody();
    }

    void PhysicsModel::setPosition(const Ogre::Vector3 &pos)
    {
        bool switchBack=false;
        if(!mIsKinematics && (switchBack=true))toKinematics();

        btTransform ts=mBody->getWorldTransform();
        ts.setOrigin(BtOgre::Convert::toBullet(pos));
        mBody->getMotionState()->setWorldTransform(ts);
        mBody->setCenterOfMassTransform(ts);

        if(switchBack)toRigidBody();
    }

    void PhysicsModel::rescale(const Ogre::Vector3 &sca)
    {
        mBody->getCollisionShape()->setLocalScaling(mBody->getCollisionShape()->getLocalScaling()*BtOgre::Convert::toBullet(sca));
    }

    void PhysicsModel::setScale(const Ogre::Vector3 &sca)
    {
        mBody->getCollisionShape()->setLocalScaling(BtOgre::Convert::toBullet(sca));
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
