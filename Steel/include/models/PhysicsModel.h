#ifndef STEEL_PHYSICSMODEL_H
#define STEEL_PHYSICSMODEL_H

#include <stack>

#include "steeltypes.h"
#include "Model.h"
#include "SignalEmitter.h"

class btDynamicsWorld;
class btPairCachingGhostObject;
class btRigidBody;
class btCollisionShape;
class btVector3;

namespace Steel
{
    class SignalListener;
    class PhysicsModelManager;
    class OgreModel;
    class Agent;
    /**
     * Physic representation of an OgreModel (can't work without it as of now)
     */
    class PhysicsModel: public Model, SignalEmitter
    {
        DECLARE_STEEL_MODEL(PhysicsModel, ModelType::PHYSICS);

    public:
        /// The object's mass.
        static const Ogre::String MASS_ATTRIBUTE;
        static const float DEFAULT_MODEL_MASS;

        /// The object's friction. Friction is essentially damping, but it only applies when two bodies are touching instead of all the time like with linear and angular damping
        static const Ogre::String FRICTION_ATTRIBUTE;
        static const float DEFAULT_MODEL_FRICTION;

        /// If true, the model will compensate for gravity each frame.
        static const Ogre::String LEVITATE_ATTRIBUTE;
        static const bool DEFAULT_MODEL_LEVITATE;

        /// The object's damping. Damping affects how the body moves through the world in any given direction. Having linear damping at zero (the default) means objects will keep moving until friction slows them down. At higher values, they would slow down even if they don't touch anything. Angular damping is similar, but applies to angular motion (ie. rotation)
        static const Ogre::String DAMPING_ATTRIBUTE;
        static const float DEFAULT_MODEL_DAMPING;

        /// Rotation angle multiplier
        static const Ogre::String ROTATION_FACTOR_ATTRIBUTE;
        static const float DEFAULT_MODEL_ROTATION_FACTOR;

        /// If non zero, multiplier to the force keeping the model's top upward.
        static const Ogre::String KEEP_VERTICAL_FACTOR_ATTRIBUTE;
        static const float DEFAULT_MODEL_KEEP_VERTICAL_FACTOR;

        /// The shape of the model bounding box. Value should be one of BBOX_SHAPE_NAME_*
        static const Ogre::String BBOX_SHAPE_ATTRIBUTE;
        /// If true, collision with other objects does not affect them (ie hitbox).
        static const Ogre::String GHOST_ATTRIBUTE;
        /// Ghost models will emit their signal(s) iff the colliding agent has one of those
        static const Ogre::String EMIT_ON_TAG_ATTRIBUTE;

        static const Ogre::String BBOX_SHAPE_NAME_BOX;
        static const Ogre::String BBOX_SHAPE_NAME_CONVEXHULL;
        static const Ogre::String BBOX_SHAPE_NAME_SPHERE;
        static const Ogre::String BBOX_SHAPE_NAME_TRIMESH;

        enum class EventType : int
        {
            CHANGE = 1,
        };

        Signal registerEvent(EventType evt, SignalListener *const listener);

        PhysicsModel();
        PhysicsModel(PhysicsModel const &o);
        void init(btDynamicsWorld *world, OgreModel *omodel);
        PhysicsModel &operator=(const PhysicsModel &other);
        virtual ~PhysicsModel();

        ///serialize itself into the given Json object
        void toJson(Json::Value &node);
        ///deserialize itself from the given Json object. return true is successful.
        bool fromJson(Json::Value const &root);

        void pushState();
        /** Set the current state to the top of the states stack. Returns the current
         * state (0: rigidBody, 1: kinematics). If the stack is empty, does nothing an returns th current state.*/
        bool popState();

        void toKinematics();
        void toRigidBody();

        inline float mass() const {return mMass;}

        void rotate(Ogre::Quaternion const &q);
        void setRotation(Ogre::Quaternion const &q);
        Ogre::Quaternion rotation();
        void applyTorque(Ogre::Vector3 const &tq);
        void applyTorqueImpulse(Ogre::Vector3 const &tq);

        void setPosition(Ogre::Vector3 const &pos);
        void move(Ogre::Vector3 const &dpos);
        void applyCentralImpulse(Ogre::Vector3 const &f);
        void applyCentralForce(Ogre::Vector3 const &f);
        Ogre::Vector3 angularVelocity() const;

        void rescale(Ogre::Vector3 const &sca);
        void setScale(Ogre::Vector3 const &sca);

        void setSelected(bool selected);
        void cleanup();

        void update(float timestep, PhysicsModelManager *manager);
        /// used to store a void* within the physics object TODO: store an AgentId
        void setUserPointer(Agent *agent);

        void enableDeactivation();
        void disableDeactivation();

        /// Returns the linear velocity of the object
        Ogre::Vector3 velocity();

        float linearDamping();
        void setDamping(float value);

        float keepVerticalFactor();
        void setKeepVerticalFactor(float value);
        /// A ghost model won't interact with the world, but may still detect collisions.
        void setGhost(bool flag);
        /// World interactions are physics interactions with other physics object of the world.
        void enableWorldInteractions(bool flag);

        BoundingShape shape() const {return mShape;}
        void setShape(OgreModel *const omodel, BoundingShape requestedShape);

    protected:
        /// Creates the model's rigid body with a boundingShape matching the given OgreModel entity
        void createRigidBody(Steel::OgreModel *const omodel);
        /// Destructs the model's rigid body.
        void destroyRigidBody();
        btCollisionShape *createShapeForEntity(Steel::OgreModel *const omodel, Steel::BoundingShape requestedShape, btVector3 &inertia);
        /// Dispatches signals upon valid collisions.
        void collisionCheck(PhysicsModelManager *manager);

        /// Removes the model's rigidbody from the world, as well as its ghostObject, if any.
        void removeFromWorld();
        /// Puts back the model's rigidbody from the world, as well as its ghostObject, if any.
        void addToWorld();

        //not owned
        btDynamicsWorld *mWorld;

        //owned
        btRigidBody *mBody;
        Ogre::Real mMass;
        Ogre::Real mFriction;
        Ogre::Real mDamping;
        bool mIsKinematics;
        float mRotationFactor;
        float mKeepVerticalFactor;
        /// kinematics/rigidBody states stack
        struct State
        {
            bool isKinematics;
            int bodyCollisionFlags;
            bool isSelected;
        };
        std::stack<State> mStates;
        /// Shape of the physic model representing the graphic model.
        BoundingShape mShape;
        bool mIsSelected;

        /// See GHOST_ATTRIBUTE docstring.
        bool mIsGhost;
        /// Bullet ghost object
        btPairCachingGhostObject *mGhostObject;
        /// See EMIT_ON_TAG_ATTRIBUTE docstring
        std::map<Tag, std::set<Signal>> mEmitOnTag;
        /// other agents currently colliding with this one
        std::set<AgentId> mCollidingAgents;
        /// See LEVITATE_ATTRIBUTE
        bool mLevitate;
        /// events for which the object will emit a signal
        std::map<EventType, Signal> mEventSignals;
    };
}
#endif // STEEL_PHYSICSMODEL_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
