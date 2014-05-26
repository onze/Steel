#include "models/PhysicsModel.h"


#include <bullet/BulletCollision/btBulletCollisionCommon.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/CollisionShapes/btCollisionShape.h>
#include <bullet/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet/LinearMath/btVector3.h>
#include <OgreEntity.h>
#include <BtOgreGP.h>
#include <BtOgrePG.h>

#include "steeltypes.h"
#include "models/Agent.h"
#include "models/OgreModel.h"
#include "tools/StringUtils.h"
#include "tools/JsonUtils.h"
#include "TagManager.h"
#include "SignalManager.h"
#include <models/PhysicsModelManager.h>
#include <Level.h>
#include <models/AgentManager.h>
#include <tools/RigidBodyStateWrapper.h>
#include <SignalListener.h>
#include <Debug.h>

namespace Steel
{
    const Ogre::String PhysicsModel::MASS_ATTRIBUTE = "mass";
    const float PhysicsModel::DEFAULT_MODEL_MASS = .0f;

    const Ogre::String PhysicsModel::FRICTION_ATTRIBUTE = "friction";
    const float PhysicsModel::DEFAULT_MODEL_FRICTION = 0.5f;

    const Ogre::String PhysicsModel::DAMPING_ATTRIBUTE = "damping";
    const float PhysicsModel::DEFAULT_MODEL_DAMPING = .0f;

    const Ogre::String PhysicsModel::LEVITATE_ATTRIBUTE = "levitate";
    const bool PhysicsModel::DEFAULT_MODEL_LEVITATE = false;

    const Ogre::String PhysicsModel::ROTATION_FACTOR_ATTRIBUTE = "rotationFactor";
    const float PhysicsModel::DEFAULT_MODEL_ROTATION_FACTOR = 1.0f;

    const Ogre::String PhysicsModel::KEEP_VERTICAL_FACTOR_ATTRIBUTE = "keepVerticalFactor";
    const float PhysicsModel::DEFAULT_MODEL_KEEP_VERTICAL_FACTOR = 0.0f;

    const Ogre::String PhysicsModel::BBOX_SHAPE_ATTRIBUTE = "shape";
    const Ogre::String PhysicsModel::GHOST_ATTRIBUTE = "ghost";
    const Ogre::String PhysicsModel::EMIT_ON_TAG_ATTRIBUTE = "emitOnHitIfTagged";

    const Ogre::String PhysicsModel::BBOX_SHAPE_NAME_BOX = "box";
    const Ogre::String PhysicsModel::BBOX_SHAPE_NAME_CONVEXHULL = "convexHull";
    const Ogre::String PhysicsModel::BBOX_SHAPE_NAME_SPHERE = "sphere";
    const Ogre::String PhysicsModel::BBOX_SHAPE_NAME_TRIMESH = "trimesh";

    PhysicsModel::PhysicsModel(): Model(), SignalEmitter(),
        mWorld(nullptr), mBody(nullptr),
        mMass(PhysicsModel::DEFAULT_MODEL_MASS), mFriction(PhysicsModel::DEFAULT_MODEL_FRICTION),
        mDamping(PhysicsModel::DEFAULT_MODEL_DAMPING), mIsKinematics(false),
        mRotationFactor(PhysicsModel::DEFAULT_MODEL_ROTATION_FACTOR), mKeepVerticalFactor(PhysicsModel::DEFAULT_MODEL_KEEP_VERTICAL_FACTOR),
        mStates(), mShape(BoundingShape::SPHERE), mIsSelected(false),
        mIsGhost(false), mGhostObject(nullptr), mEmitOnTag(), mCollidingAgents(),
        mLevitate(PhysicsModel::DEFAULT_MODEL_LEVITATE), mEventSignals()
    {
    }

    PhysicsModel::PhysicsModel(PhysicsModel const &o): Model(o), SignalEmitter(),
        mWorld(o.mWorld),
        mBody(o.mBody),
        mMass(o.mMass),
        mFriction(o.mFriction),
        mDamping(o.mDamping),
        mIsKinematics(o.mIsKinematics),
        mRotationFactor(o.mRotationFactor),
        mKeepVerticalFactor(o.mKeepVerticalFactor),
        mStates(o.mStates),
        mShape(o.mShape),
        mIsGhost(o.mIsGhost),
        mGhostObject(o.mGhostObject),
        mEmitOnTag(o.mEmitOnTag),
        mCollidingAgents(o.mCollidingAgents),
        mLevitate(o.mLevitate),
        mEventSignals(o.mEventSignals.begin(), o.mEventSignals.end())
    {
    }

    PhysicsModel &PhysicsModel::operator=(const PhysicsModel &o)
    {
        if(&o != this)
        {
            Model::operator=(o);
            mWorld = o.mWorld;
            mBody = o.mBody;
            mMass = o.mMass;
            mIsKinematics = o.mIsKinematics;
            mStates = o.mStates;
            mShape = o.mShape;
            mIsGhost = o.mIsGhost;
            mGhostObject = o.mGhostObject;
            mEmitOnTag = o.mEmitOnTag;
            mCollidingAgents = o.mCollidingAgents;
            mEventSignals.clear();
            mEventSignals.insert(o.mEventSignals.begin(), o.mEventSignals.end());
        }

        return *this;
    }

    PhysicsModel::~PhysicsModel()
    {
    }

    void PhysicsModel::cleanup()
    {

        if(nullptr != mWorld && mBody != nullptr)
        {
            if(nullptr != mGhostObject)
            {
                mWorld->removeCollisionObject(mGhostObject);
                delete mGhostObject;
                mGhostObject = nullptr;
            }

            destroyRigidBody();
            mWorld = nullptr;
        }

        mEventSignals.clear();
        mEmitOnTag.clear();
        Model::cleanup();
    }

    void PhysicsModel::init(btDynamicsWorld *world, Steel::OgreModel *omodel)
    {
        mWorld = world;
        Ogre::String intro = "PhysicsModel::init(): ";

        if(nullptr == world)
        {
            Debug::error(intro)(" was given a nullptr world. Aborted.").endl();
            return;
        }

        if(nullptr == omodel)
        {
            Debug::error(intro)(" was given a nullptr omodel. Aborted.").endl();
            return;
        }

        createRigidBody(omodel);

        // default
//         mBody->setCollisionFlags(
//             btCollisionObject::CF_ANISOTROPIC_FRICTION |
//             //btCollisionObject::CF_ANISOTROPIC_FRICTION_DISABLED
//             btCollisionObject::CF_ANISOTROPIC_ROLLING_FRICTION |
//             btCollisionObject::CF_CHARACTER_OBJECT |
//             btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK |
//             btCollisionObject::CF_DISABLE_SPU_COLLISION_PROCESSING |
//             btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT |
//             btCollisionObject::CF_KINEMATIC_OBJECT |
//             //btCollisionObject::CF_NO_CONTACT_RESPONSE
//             btCollisionObject::CF_STATIC_OBJECT
//         );
        addToWorld();

        // ghost setup if needed
        setGhost(mIsGhost);

        setUserPointer(nullptr);
    }

    void PhysicsModel::createRigidBody(Steel::OgreModel *const omodel)
    {
        // Create shape
        btVector3 inertia;
        btCollisionShape *shape = createShapeForEntity(omodel, mShape, inertia);

        //Create BtOgre MotionState (connects Ogre and Bullet).
        RigidBodyStateWrapper *state = new RigidBodyStateWrapper(omodel->sceneNode());

        //Create the Body.
        mBody = new btRigidBody(mMass, state, shape, inertia);
        mBody->setFriction(mFriction);

        setDamping(mDamping);
    }

    void PhysicsModel::destroyRigidBody()
    {
        mWorld->removeRigidBody(mBody);

        if(nullptr != mBody->getMotionState())
            delete mBody->getMotionState();

        STEEL_DELETE(mBody);
    }

    void PhysicsModel::enableWorldInteractions(bool flag)
    {
        if(nullptr == mBody)
            return;

        // visual representation should not affect the world
        if(flag)
            mBody->setCollisionFlags(mBody->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
        else
            mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }

    void PhysicsModel::setGhost(bool flag)
    {
        if(nullptr != mBody)
        {
            if(flag)
            {
                enableWorldInteractions(false);

                // bullet hitbox
                if(nullptr == mGhostObject)
                    mGhostObject = new btPairCachingGhostObject();

                mGhostObject->setWorldTransform(mBody->getWorldTransform());
                mGhostObject->setCollisionShape(mBody->getCollisionShape());
                mGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE |
                                                btCollisionObject::CF_CHARACTER_OBJECT |
                                                btCollisionObject::CF_STATIC_OBJECT |
                                                btCollisionObject::CF_KINEMATIC_OBJECT
                                               );

                mWorld->addCollisionObject(mGhostObject);
                mIsGhost = true;
            }
            else
            {
                if(nullptr != mGhostObject)
                    mWorld->removeCollisionObject(mGhostObject);

                mIsGhost = false;
            }
        }

    }

    void PhysicsModel::setKeepVerticalFactor(float value)
    {
        mKeepVerticalFactor = value;
    }

    float PhysicsModel::keepVerticalFactor()
    {
        return mKeepVerticalFactor;
    }

    void PhysicsModel::setDamping(float value)
    {
        if(nullptr != mBody)
            mBody->setDamping(value, value);
    }

    float PhysicsModel::linearDamping()
    {
        return nullptr == mBody?.0f : (float) mBody->getLinearDamping();
    }

    Signal PhysicsModel::registerEvent(EventType event, SignalListener *const listener)
    {
        auto it = mEventSignals.find(event);
        Signal signal = mEventSignals.end() == it ? mEventSignals.emplace(event, SignalManager::instance().anonymousSignal()).first->second : it->second;
        listener->registerSignal(signal);
        return signal;
    }

    bool PhysicsModel::fromJson(Json::Value const &root)
    {
        Json::Value value;
        bool allWasFine = true;

        mMass = JsonUtils::asFloat(root[PhysicsModel::MASS_ATTRIBUTE], PhysicsModel::DEFAULT_MODEL_MASS);
        mFriction = JsonUtils::asFloat(root[PhysicsModel::FRICTION_ATTRIBUTE], PhysicsModel::DEFAULT_MODEL_FRICTION);
        mDamping = JsonUtils::asFloat(root[PhysicsModel::DAMPING_ATTRIBUTE], PhysicsModel::DEFAULT_MODEL_DAMPING);
        mRotationFactor = JsonUtils::asFloat(root[PhysicsModel::ROTATION_FACTOR_ATTRIBUTE], PhysicsModel::DEFAULT_MODEL_ROTATION_FACTOR);
        mKeepVerticalFactor = JsonUtils::asFloat(root[PhysicsModel::KEEP_VERTICAL_FACTOR_ATTRIBUTE], PhysicsModel::DEFAULT_MODEL_KEEP_VERTICAL_FACTOR);

        auto shape = JsonUtils::asString(root[PhysicsModel::BBOX_SHAPE_ATTRIBUTE], Ogre::String(BBOX_SHAPE_NAME_SPHERE));
        mShape = toBoundingShape(shape);

        mIsGhost = JsonUtils::asBool(root[PhysicsModel::GHOST_ATTRIBUTE], false);

        // EMIT_ON_ANY_TAG_ATTRIBUTE
        value = root[PhysicsModel::EMIT_ON_TAG_ATTRIBUTE];

        if(!mIsGhost && !value.isNull())
            Debug::error(STEEL_METH_INTRO, "unexpected field  ").quotes(PhysicsModel::EMIT_ON_TAG_ATTRIBUTE)(". Skipped.").endl();
        else if(mIsGhost)
        {
            if((!value.isNull() && !value.isObject()) && !(allWasFine = false))
                Debug::error(STEEL_METH_INTRO, "invalid field  ").quotes(PhysicsModel::EMIT_ON_TAG_ATTRIBUTE)(". Aborting.").endl();
            else
            {
                bool aborted = false;

                for(auto const & tagName : value.getMemberNames())
                {
                    if(aborted)
                        break;

                    // get the tag
                    Tag tag = TagManager::instance().toTag(tagName);

                    if(INVALID_TAG == tag && !(allWasFine = false))
                    {
                        Debug::error(STEEL_METH_INTRO, "invalid tag ")(tag)(" / ").quotes(tagName).endl();
                        break;
                    }

                    // get matching signals
                    Json::Value signals = value[tagName];

                    if(!signals.isArray() && !(allWasFine = false))
                    {
                        Debug::error(STEEL_METH_INTRO, "invalid signals for tag ").quotes(tagName).endl();
                        break;
                    }

                    mEmitOnTag.emplace(tag, std::set<Tag>());

                    for(Json::ValueIterator it = signals.begin(); it != signals.end(); ++it)
                    {
                        Json::Value signal_value = *it;

                        if(signal_value.isNull() || !signal_value.isString())
                        {
                            aborted = true;
                            Debug::error(STEEL_METH_INTRO, "invalid signal ").quotes(signal_value.asString())(" for tag ").quotes(tagName).endl();
                            break;
                        }

                        Signal signal = SignalManager::instance().toSignal(signal_value.asString());
                        mEmitOnTag[tag].insert(signal);
                    }
                }
            }
        }

        mLevitate = JsonUtils::asBool(root[PhysicsModel::LEVITATE_ATTRIBUTE], PhysicsModel::DEFAULT_MODEL_LEVITATE);

        // agentTags
        allWasFine &= deserializeTags(root);

        // final check
        if(!allWasFine)
        {
            Debug::error("json was:").endl()(root.toStyledString()).endl()("deserialisation aborted.").endl();
            return false;
        }

        return true;
    }

    void PhysicsModel::addToWorld()
    {
        mWorld->addRigidBody(mBody);
        mBody->setActivationState(ACTIVE_TAG);
    }

    void PhysicsModel::removeFromWorld()
    {
        mWorld->removeRigidBody(mBody);
    }

    void PhysicsModel::toKinematics()
    {
        if(!mIsKinematics)
        {
            removeFromWorld();
            // http://www.oogtech.org/content/2011/09/07/bullet-survival-kit-4-the-motion-state/
            // This flag tells the engine that the object is kinematic –
            // so it shouldn’t move under the influence of gravity or because of colliding with other objects
            mBody->setCollisionFlags(mBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            mBody->setActivationState(DISABLE_DEACTIVATION);
            auto zero = btVector3(0.f, .0f, .0f);
            mBody->setLinearVelocity(zero);
            mBody->setAngularVelocity(zero);
            mIsKinematics = true;
            addToWorld();
        }
    }

    void PhysicsModel::toRigidBody()
    {
        if(mIsKinematics)
        {
            removeFromWorld();
            //         mBody->forceActivationState(ACTIVE_TAG);
            mBody->setActivationState(ACTIVE_TAG);
            mBody->setCollisionFlags(mBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
            mIsKinematics = false;
            addToWorld();
        }
    }

    void PhysicsModel::update(float timestep, PhysicsModelManager *manager)
    {
        if(mIsGhost && mEmitOnTag.size())
        {
            collisionCheck(manager);
        }

        if(static_cast<RigidBodyStateWrapper *>(mBody->getMotionState())->poolTransform())
        {
            auto const it = mEventSignals.find(EventType::CHANGE);

            if(mEventSignals.end() != it)
                emit(it->second);
        }

        if(PhysicsModel::DEFAULT_MODEL_KEEP_VERTICAL_FACTOR != mKeepVerticalFactor)
        {
            Ogre::Vector3 t = (rotation() * Ogre::Vector3::UNIT_Y).crossProduct(Ogre::Vector3::UNIT_Y);
            applyTorqueImpulse(t * timestep * mKeepVerticalFactor);
        }

        if(mLevitate)
            applyCentralForce(Ogre::Vector3::UNIT_Y * mMass);
    }

    void PhysicsModel::setUserPointer(Agent *agent)
    {
        if(nullptr != mBody)
        {
            mBody->setUserPointer(agent);
        }

        if(nullptr != mGhostObject)
        {
//             mGhostObject->setUserPointer(agent);
        }
    }

    void PhysicsModel::collisionCheck(PhysicsModelManager *manager)
    {
        std::set<AgentId> currentlyCollidingAgents;

        btManifoldArray manifoldArray;
        btBroadphasePairArray &pairArray = mGhostObject->getOverlappingPairCache()->getOverlappingPairArray();
        int numPairs = pairArray.size();

        for(int i = 0; i < numPairs; i++)
        {
            manifoldArray.clear();

            const btBroadphasePair &pair = pairArray[i];

            //unless we manually perform collision detection on this pair, the contacts are in the dynamics world paircache:
            btBroadphasePair *collisionPair = mWorld->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);

            if(!collisionPair)
                continue;

            if(collisionPair->m_algorithm)
                collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

            for(int j = 0; j < manifoldArray.size(); j++)
            {
                btPersistentManifold *manifold = manifoldArray[j];

//                 btScalar directionSign = manifold->getBody0() == mGhostObject ? btScalar(-1.0) : btScalar(1.0);
                for(int p = 0; p < manifold->getNumContacts(); p++)
                {
                    const btManifoldPoint &pt = manifold->getContactPoint(p);

                    if(pt.getDistance() < 0.f)
                    {
                        const btCollisionObject *otherBody = manifold->getBody0();

                        if(otherBody == mGhostObject || otherBody == mBody)
                            otherBody = manifold->getBody1();


                        Agent *otherAgent = static_cast<Agent *>(otherBody->getUserPointer());

                        if(nullptr != otherAgent && otherAgent->physicsModel() != this)
                        {
                            if(0)
                                Debug::log("agent ")(((Agent *)mGhostObject->getUserPointer())->id())
                                (" with physics model ")(((Agent *)mGhostObject->getUserPointer())->physicsModelId())
                                (" collides with agent ")(otherAgent->id())
                                (" with physics model ")(otherAgent->physicsModelId()).endl();

                            currentlyCollidingAgents.insert(otherAgent->id());
                        }

//                         const btVector3& ptA = pt.getPositionWorldOnA();
//                         const btVector3& ptB = pt.getPositionWorldOnB();
//                         const btVector3& normalOnB = pt.m_normalWorldOnB;
                    }
                }
            }
        }

        if(currentlyCollidingAgents.size())
        {
            // determine newly colliding agents
            std::set<AgentId> newlyColliding;

            if(mCollidingAgents.size())
            {
                std::set_difference(currentlyCollidingAgents.begin(), currentlyCollidingAgents.end(),
                                    mCollidingAgents.begin(), mCollidingAgents.end(),
                                    std::inserter(newlyColliding, newlyColliding.end()));
            }
            else
            {
                newlyColliding.insert(currentlyCollidingAgents.begin(), currentlyCollidingAgents.end());
            }

            if(newlyColliding.size())
            {
                // collect tags
                std::set<Tag> tagsMet;

                for(auto const & aid : newlyColliding)
                {
                    Agent *agent = manager->level()->agentMan()->getAgent(aid);

                    if(nullptr == agent)
                        continue;

                    auto _tags = agent->tags();
                    tagsMet.insert(_tags.begin(), _tags.end());
                }

                // collect signals
                std::set<Signal> signals;

                for(auto const & tag : tagsMet)
                {
                    auto toEmit_it = mEmitOnTag.find(tag);

//                     Tag _tag=toEmit_it->first;
//                     std::set<Tag> _signals=toEmit_it->second;
//                     Debug::log(_tag).endl()(_signals).endl()(mEmitOnTag).endl();
                    if(mEmitOnTag.end() != toEmit_it)
                    {
                        signals.insert(toEmit_it->second.begin(), toEmit_it->second.end());
                    }
                }

                // emit signals
                for(auto const & signal : signals)
                    emit(signal);

                // register newly colliding as currently colliding
                mCollidingAgents.clear();
                mCollidingAgents.insert(currentlyCollidingAgents.begin(), currentlyCollidingAgents.end());
            }
        }
        else
        {
            mCollidingAgents.clear();
        }
    }

    void PhysicsModel::toJson(Json::Value &node)
    {
        if(PhysicsModel::DEFAULT_MODEL_MASS != mMass)
            node[PhysicsModel::MASS_ATTRIBUTE] = JsonUtils::toJson(mMass);

        if(PhysicsModel::DEFAULT_MODEL_FRICTION != mFriction)
            node[PhysicsModel::FRICTION_ATTRIBUTE] = JsonUtils::toJson(mFriction);

        if(PhysicsModel::DEFAULT_MODEL_DAMPING != mDamping)
            node[PhysicsModel::DAMPING_ATTRIBUTE] = JsonUtils::toJson(mDamping);

        if(PhysicsModel::DEFAULT_MODEL_LEVITATE != mLevitate)
            node[PhysicsModel::LEVITATE_ATTRIBUTE] = JsonUtils::toJson(mLevitate);

        if(PhysicsModel::DEFAULT_MODEL_ROTATION_FACTOR != mRotationFactor)
            node[PhysicsModel::ROTATION_FACTOR_ATTRIBUTE] = JsonUtils::toJson(mRotationFactor);

        if(PhysicsModel::DEFAULT_MODEL_KEEP_VERTICAL_FACTOR != mKeepVerticalFactor)
            node[PhysicsModel::KEEP_VERTICAL_FACTOR_ATTRIBUTE] = JsonUtils::toJson(mKeepVerticalFactor);

        node[PhysicsModel::BBOX_SHAPE_ATTRIBUTE] = JsonUtils::toJson(toString(mShape));

        if(mIsGhost)
            node[PhysicsModel::GHOST_ATTRIBUTE] = JsonUtils::toJson(mIsGhost);

        if(mEmitOnTag.size())
        {
            // tags and signals need to be remapped to their string values
            Json::Value mapValue;

            for(auto const & item : mEmitOnTag)
            {
                if(item.second.size())
                {
                    Json::Value signalsValue;

                    for(Signal const & signal : item.second)
                        signalsValue.append(SignalManager::instance().fromSignal(signal).c_str());

                    mapValue[TagManager::instance().fromTag(item.first).c_str()] = signalsValue;
                }
            }

            if(mapValue.size())
                node[PhysicsModel::EMIT_ON_TAG_ATTRIBUTE] = mapValue;
        }

        serializeTags(node);
    }

    void PhysicsModel::setSelected(bool selected)
    {
        if(selected)
        {
            pushState();
            toKinematics();
        }
        else
        {
            popState();
        }

        mIsSelected = selected;
    }

    bool PhysicsModel::popState()
    {
        if(!mStates.empty())
        {
            State state = mStates.top();

            if(state.isKinematics != mIsKinematics)
                mIsKinematics ? toRigidBody() : toKinematics();

            mBody->setCollisionFlags(state.bodyCollisionFlags);
            mStates.pop();
        }

        return mIsKinematics;
    }

    void PhysicsModel::pushState()
    {
        mStates.push( {mIsKinematics, mBody->getCollisionFlags(), mIsSelected});
    }

    void PhysicsModel::setPosition(const Ogre::Vector3 &pos)
    {
        bool switchBack = false;

        if(!mIsKinematics && (switchBack = true))
            toKinematics();

        btTransform ts = mBody->getWorldTransform();
        ts.setOrigin(BtOgre::Convert::toBullet(pos));

        mBody->getMotionState()->setWorldTransform(ts);
        mBody->setCenterOfMassTransform(ts);

        if(nullptr != mGhostObject)
            mGhostObject->setWorldTransform(ts);

        if(switchBack)
            toRigidBody();
    }

    void PhysicsModel::move(const Ogre::Vector3 &dpos)
    {
        bool switchBack = false;

        if(!mIsKinematics && (switchBack = true))
            toKinematics();

        btTransform ts = mBody->getWorldTransform();
        ts.setOrigin(ts.getOrigin() + BtOgre::Convert::toBullet(dpos));

        mBody->getMotionState()->setWorldTransform(ts);
        mBody->setCenterOfMassTransform(ts);

        if(nullptr != mGhostObject)
            mGhostObject->setWorldTransform(ts);

        if(switchBack)
            toRigidBody();
    }

    Ogre::Quaternion PhysicsModel::rotation()
    {
        if(nullptr == mBody)
            return Ogre::Quaternion::ZERO;

        btTransform tr;
        mBody->getMotionState()->getWorldTransform(tr);
        return BtOgre::Convert::toOgre(tr.getRotation());
    }

    void PhysicsModel::rotate(Ogre::Quaternion const &q)
    {
        if(nullptr == mBody || q.isNaN() || q == Ogre::Quaternion::ZERO)
            return;

        bool switchBack = false;

        if(!mIsKinematics && (switchBack = true))
            toKinematics();

        if(1)
        {
            btTransform ts = mBody->getWorldTransform(), tr;
            tr.setIdentity();
            tr.setRotation(BtOgre::Convert::toBullet(q));
            ts *= tr;

            mBody->getMotionState()->setWorldTransform(ts);
            mBody->setCenterOfMassTransform(ts);

            if(nullptr != mGhostObject)
                mGhostObject->setWorldTransform(ts);
        }
        else
        {
            btTransform tr;
            tr.setIdentity();
            tr.setRotation(BtOgre::Convert::toBullet(q));
            mBody->getWorldTransform().operator *= (tr);
        }

        if(switchBack)
            toRigidBody();
    }

    void PhysicsModel::setRotation(Ogre::Quaternion const &q)
    {
        if(nullptr == mBody || q.isNaN() || q == Ogre::Quaternion::ZERO)
            return;

        bool switchBack = false;

        if(!mIsKinematics && (switchBack = true))
            toKinematics();

        btTransform tr;
        mBody->getMotionState()->getWorldTransform(tr);
        tr.setRotation(BtOgre::Convert::toBullet(q));
        mBody->getMotionState()->setWorldTransform(tr);
        mBody->setCenterOfMassTransform(tr);

        if(nullptr != mGhostObject)
            mGhostObject->setWorldTransform(tr);

        if(switchBack)
            toRigidBody();
    }

    void PhysicsModel::applyTorque(Ogre::Vector3 const &tq)
    {
        if(nullptr == mBody)
            return;

        mBody->applyTorque(BtOgre::Convert::toBullet(tq)*mRotationFactor);
    }

    void PhysicsModel::applyTorqueImpulse(Ogre::Vector3 const &tq)
    {
        if(nullptr == mBody)
            return;

        mBody->applyTorqueImpulse(BtOgre::Convert::toBullet(tq)*mRotationFactor);
    }

    void PhysicsModel::applyCentralImpulse(Ogre::Vector3 const &f)
    {
        if(nullptr == mBody)
            return;

        mBody->applyCentralImpulse(BtOgre::Convert::toBullet(f));
    }

    void PhysicsModel::applyCentralForce(Ogre::Vector3 const &f)
    {
        if(nullptr == mBody)
            return;

        mBody->applyCentralForce(BtOgre::Convert::toBullet(f));
    }

    Ogre::Vector3 PhysicsModel::angularVelocity() const
    {
        if(nullptr == mBody)
            return Ogre::Vector3::ZERO;

        return BtOgre::Convert::toOgre(mBody->getAngularVelocity());
    }

    void PhysicsModel::rescale(Ogre::Vector3 const &sca)
    {
        mBody->getCollisionShape()->setLocalScaling(mBody->getCollisionShape()->getLocalScaling() * BtOgre::Convert::toBullet(sca));
    }

    void PhysicsModel::setScale(Ogre::Vector3 const &sca)
    {
        mBody->getCollisionShape()->setLocalScaling(BtOgre::Convert::toBullet(sca));
    }

    Ogre::Vector3 PhysicsModel::velocity()
    {
        return BtOgre::Convert::toOgre(mBody->getLinearVelocity());
    }

    void PhysicsModel::enableDeactivation()
    {
        if(nullptr == mBody)
            return;

        mBody->setActivationState(WANTS_DEACTIVATION);
    }

    void PhysicsModel::disableDeactivation()
    {
        if(nullptr == mBody)
            return;

        mBody->setActivationState(DISABLE_DEACTIVATION);
    }

    btCollisionShape *PhysicsModel::createShapeForEntity(OgreModel *const omodel, BoundingShape requestedShape, btVector3 &inertia)
    {
        if(nullptr == omodel)
        {
            Debug::error(STEEL_METH_INTRO, "cannot create shape for null ogre model. Returning nullptr.").endl();
            return nullptr;
        }

        if(nullptr == omodel->entity())
        {
            Debug::error(STEEL_METH_INTRO, "cannot create shape for null ogre model entity. Returning nullptr.").endl();
            return nullptr;
        }

        BtOgre::StaticMeshToShapeConverter converter(omodel->entity());
        btCollisionShape *shape = nullptr;

        switch(requestedShape)
        {
            case BoundingShape::BOX:
                shape = converter.createBox();
                break;

            case BoundingShape::CONVEXHULL:
                shape = converter.createConvex();
                break;

            case BoundingShape::SPHERE:
                shape = converter.createSphere();
                break;

            case BoundingShape::TRIMESH:
                shape = converter.createTrimesh();
                break;
        }

        if(nullptr != shape)
            shape->calculateLocalInertia(mMass, inertia);

        return shape;
    }

    void PhysicsModel::setShape(OgreModel *const omodel, BoundingShape requestedShape)
    {
        bool isGhostSave = mIsGhost;
        auto collisionFlagsSave = mBody->getCollisionFlags();
        bool isKinematicsSave = mIsKinematics;

        setGhost(false);

        destroyRigidBody();
        mShape = requestedShape;
        createRigidBody(omodel);
        addToWorld();


        isKinematicsSave ? toRigidBody() : toKinematics();
        mBody->setCollisionFlags(collisionFlagsSave);

        if(isGhostSave)
            setGhost(true);
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

