#ifndef STEEL_PHYSICSMODEL_H
#define STEEL_PHYSICSMODEL_H

#include <stack>
#include <map>

#include "steeltypes.h"
#include "Model.h"
#include "SignalEmitter.h"

class btDynamicsWorld;
class btPairCachingGhostObject;
class btRigidBody;

namespace Steel
{
    class PhysicsModelManager;
    class OgreModel;
    class Agent;
    class Level;
    /**
     * Physic representation of an OgreModel (can't work without it as of now)
     */
    class PhysicsModel: public Model, SignalEmitter
    {
        public:
            /// The object's mass
            static const Ogre::String MASS_ATTRIBUTE;
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

            PhysicsModel();
            PhysicsModel(const PhysicsModel& o);
            void init(btDynamicsWorld *world, OgreModel *omodel);
            PhysicsModel& operator=(const PhysicsModel& other);
            virtual ~PhysicsModel();
            
            static ModelType modelType()
            {
                return MT_PHYSICS;
            }

            ///serialize itself into the given Json object
            void toJson(Json::Value& node);
            ///deserialize itself from the given Json object. return true is successful.
            bool fromJson(Json::Value &object);

            void pushState();
            /** Set the current state to the top of the states stack. Returns the current
             * state (0: rigidBody, 1: kinematics). If the stack is empty, does nothing an returns th current state.*/
            bool popState();

            void toKinematics();
            void toRigidBody();

            void move(const Ogre::Vector3 &dpos);
            void setPosition(const Ogre::Vector3 &pos);

            void rescale(const Ogre::Vector3 &sca);
            void setScale(const Ogre::Vector3 &sca);

            void setSelected(bool selected);
            void cleanup();

            void update(float timestep, PhysicsModelManager *manager);
            void setUserPointer(Agent* agent);
        protected:
            /// Dispatches signals upon valid collisions.
            void collisionCheck(PhysicsModelManager *manager);

            /// Removes the model's rigidbody from the world, as well as its ghostObject, if any.
            void removeFromWorld();
            /// Puts back the model's rigidbody from the world, as well as its ghostObject, if any.
            void addToWorld();

            /// Maps a bounding shape string to its enum value. Defaults to sphere.
            BoundingShape BBoxShapeFromString(Ogre::String &shape);
            Ogre::String StringShapeFromBBox(BoundingShape &shape);

            //not owned
            btDynamicsWorld *mWorld;

            //owned
            btRigidBody* mBody;
            Ogre::Real mMass;
            bool mIsKinematics;
            /// kinematics/rigidBody states stack
            std::stack<bool> mStates;
            /// Shape of the physic model representing the graphic model.
            BoundingShape mShape;

            /// See GHOST_ATTRIBUTE docstring.
            bool mIsGhost;
            /// Bullet ghost object
            btPairCachingGhostObject *mGhostObject;
            /// See EMIT_ON_TAG_ATTRIBUTE docstring
            std::map<Tag,std::set<Signal>> mEmitOnTag;
            /// other agents currently colliding with this one
            std::set<AgentId> mCollidingAgents;
    };
}
#endif // STEEL_PHYSICSMODEL_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
