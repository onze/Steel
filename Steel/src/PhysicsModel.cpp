#include "PhysicsModel.h"

#include <btBulletCollisionCommon.h>
#include <OgreEntity.h>

#include "steeltypes.h"
#include <Agent.h>
#include <OgreModel.h>
#include <tools/StringUtils.h>
#include <tools/JsonUtils.h>
#include <BtOgreGP.h>
#include <BtOgrePG.h>

namespace Steel
{
    const Ogre::String PhysicsModel::MASS_ATTRIBUTE="mass";
    const Ogre::String PhysicsModel::BBOX_SHAPE_ATTRIBUTE="shape";
    const Ogre::String PhysicsModel::GHOST_ATTRIBUTE="ghost";

    const Ogre::String PhysicsModel::BBOX_SHAPE_NAME_BOX="box";
    const Ogre::String PhysicsModel::BBOX_SHAPE_NAME_CONVEXHULL="convexHull";
    const Ogre::String PhysicsModel::BBOX_SHAPE_NAME_SPHERE="sphere";
    const Ogre::String PhysicsModel::BBOX_SHAPE_NAME_TRIMESH="trimesh";

    PhysicsModel::PhysicsModel(): Model(),
        mWorld(NULL), mBody(NULL),
        mMass(.0f), mIsKinematics(false), mStates(std::stack<bool>()), mShape(BS_SPHERE), mIsGhost(false)
    {

    }

    void PhysicsModel::init(btDynamicsWorld* world, Steel::OgreModel* omodel)
    {
        mWorld = world;
        Ogre::String intro = "PhysicsModel::init(): ";
        if (NULL == world)
        {
            Debug::error(intro)(" was given a NULL world. Aborted.").endl();
            return;
        }
        if (NULL == omodel)
        {
            Debug::error(intro)(" was given a NULL omodel. Aborted.").endl();
            return;
        }
        // Create shape
        BtOgre::StaticMeshToShapeConverter converter(omodel->entity());

        btCollisionShape *shape=NULL;
        switch(mShape)
        {
            case BS_BOX:
                shape=converter.createBox();
                break;
            case BS_CONVEXHULL:
                shape=converter.createConvex();
                break;
            case BS_SPHERE:
                shape=converter.createSphere();
                break;
            case BS_TRIMESH:
                shape=converter.createTrimesh();
        }

        //Calculate inertia.
        btVector3 inertia;
        shape->calculateLocalInertia(mMass, inertia);

        //Create BtOgre MotionState (connects Ogre and Bullet).
        BtOgre::RigidBodyState *state = new BtOgre::RigidBodyState(omodel->sceneNode());

        //Create the Body.
        mBody = new btRigidBody(mMass, state, shape, inertia);
        if(mIsGhost)
            mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
        world->addRigidBody(mBody);
    }

    PhysicsModel::PhysicsModel(const PhysicsModel& o)
    {
        operator=(o);
    }

    PhysicsModel &PhysicsModel::operator=(const PhysicsModel& o)
    {
        Model::operator=(o);
        mWorld = o.mWorld;
        mBody = o.mBody;
        mMass = o.mMass;
        mIsKinematics = o.mIsKinematics;
        mStates = o.mStates;
        mShape = o.mShape;
        mIsGhost = o.mIsGhost;
        return *this;
    }

    PhysicsModel::~PhysicsModel()
    {
        if (!isFree())
            cleanup();
    }

    void PhysicsModel::cleanup()
    {
        if (NULL != mWorld && mBody != NULL)
        {
            if(NULL!=mGhostObject)
            {
                mWorld->removeCollisionObject(mGhostObject);
                delete mGhostObject;
                mGhostObject = NULL;
            }
            
            mWorld->removeRigidBody(mBody);
            delete mBody;
            mBody = NULL;
            
            mWorld = NULL;
        }
    }

    void PhysicsModel::toJson(Json::Value &root)
    {
        root[PhysicsModel::MASS_ATTRIBUTE] = JsonUtils::toJson(mMass);
        root[PhysicsModel::BBOX_SHAPE_ATTRIBUTE] = JsonUtils::toJson(mShape);
        root[PhysicsModel::GHOST_ATTRIBUTE] = JsonUtils::toJson(mIsGhost);
        root[PhysicsModel::EMIT_ON_ANY_TAG_ATTRIBUTE] = JsonUtils::toJson(mEmitOnAnyTag);
    }

    BoundingShape PhysicsModel::BBoxShapeFromString(Ogre::String &shape)
    {
        if(shape==PhysicsModel::BBOX_SHAPE_NAME_BOX)
            return BS_BOX;
        if(shape==PhysicsModel::BBOX_SHAPE_NAME_CONVEXHULL)
            return BS_CONVEXHULL;
        if(shape==PhysicsModel::BBOX_SHAPE_NAME_SPHERE)
            return BS_SPHERE;
        if(shape==PhysicsModel::BBOX_SHAPE_NAME_TRIMESH)
            return BS_TRIMESH;
        Debug::error("in PhysicsModel::BBoxShapeFromString(): unknown value ").quotes(shape)
        (". Defaulting to ")(PhysicsModel::BBOX_SHAPE_NAME_SPHERE).endl();
        return BS_SPHERE;
    }

    bool PhysicsModel::fromJson(Json::Value &root)
    {
        Json::Value value;
        bool allWasFine = true;

        // MASS
        value = root[PhysicsModel::MASS_ATTRIBUTE];
        if ((value.isNull() || !value.isString()) && !(allWasFine = false))
            Debug::error("in PhysicsModel::fromJson(): invalid field ").quotes(PhysicsModel::MASS_ATTRIBUTE)(" (skipped).").endl();
        else
            mMass = Ogre::StringConverter::parseReal(value.asString(), 0.f);

        // SHAPE
        value = root[PhysicsModel::BBOX_SHAPE_ATTRIBUTE];
        if ((!value.isNull() && !value.isString()) && !(allWasFine = false))
            Debug::error("in PhysicsModel::fromJson(): invalid field  ").quotes(PhysicsModel::BBOX_SHAPE_ATTRIBUTE)("  (skipped).").endl();
        else
        {
            Ogre::String svalue=value.asString();
            mShape = BBoxShapeFromString(svalue);
        }

        // GHOST
        value = root[PhysicsModel::GHOST_ATTRIBUTE];
        if ((!value.isNull() && !value.isString()) && !(allWasFine = false))
            Debug::error("in PhysicsModel::fromJson(): invalid field  ").quotes(PhysicsModel::GHOST_ATTRIBUTE)("  (skipped).").endl();
        else
            mIsGhost = Ogre::StringConverter::parseBool(value.asString(), false);

        // final check
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
        if (!mStates.empty())
        {
            if (mStates.top() != mIsKinematics)
            {
                mIsKinematics ? toRigidBody() : toKinematics();
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
        mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        mBody->setActivationState(DISABLE_DEACTIVATION);
        auto zero = btVector3(0.f, .0f, .0f);
        mBody->setLinearVelocity(zero);
        mBody->setAngularVelocity(zero);
        mIsKinematics = true;
        mWorld->addRigidBody(mBody);
    }

    void PhysicsModel::toRigidBody()
    {
        mWorld->removeRigidBody(mBody);
        mBody->forceActivationState(ACTIVE_TAG);
        mBody->setCollisionFlags(mBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
        mIsKinematics = false;
        mWorld->addRigidBody(mBody);
    }

    void PhysicsModel::move(const Ogre::Vector3 &dpos)
    {
        bool switchBack = false;
        if (!mIsKinematics && (switchBack = true))
            toKinematics();

        btTransform ts = mBody->getWorldTransform();
        ts.setOrigin(ts.getOrigin() + BtOgre::Convert::toBullet(dpos));
        mBody->getMotionState()->setWorldTransform(ts);
        mBody->setCenterOfMassTransform(ts);

        if (switchBack)
            toRigidBody();
    }

    void PhysicsModel::setPosition(const Ogre::Vector3 &pos)
    {
        bool switchBack = false;
        if (!mIsKinematics && (switchBack = true))
            toKinematics();

        btTransform ts = mBody->getWorldTransform();
        ts.setOrigin(BtOgre::Convert::toBullet(pos));
        mBody->getMotionState()->setWorldTransform(ts);
        mBody->setCenterOfMassTransform(ts);

        if (switchBack)
            toRigidBody();
    }

    void PhysicsModel::rescale(const Ogre::Vector3 &sca)
    {
        mBody->getCollisionShape()->setLocalScaling(
            mBody->getCollisionShape()->getLocalScaling() * BtOgre::Convert::toBullet(sca));
    }

    void PhysicsModel::setScale(const Ogre::Vector3 &sca)
    {
        mBody->getCollisionShape()->setLocalScaling(BtOgre::Convert::toBullet(sca));
    }

    void PhysicsModel::setSelected(bool selected)
    {
        if (selected)
        {
            pushState();
            toKinematics();
        }
        else
        {
            popState();
        }
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
