#ifndef STEEL_PHYSICSMODEL_H
#define STEEL_PHYSICSMODEL_H

#include <stack>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include "Model.h"

class btDynamicsWorld;

namespace Steel
{
    class OgreModel;
    /**
     * Physic representation of an OgreModel (can't work without it as of now)
     */
    class PhysicsModel: public Model
    {
        public:
            /// The object's mass
            static const Ogre::String MASS_ATTRIBUTE;
            /// The shape of the model bounding box. Value should be one of BBOX_SHAPE_NAME_*
            static const Ogre::String BBOX_SHAPE_ATTRIBUTE;
            /// If true, collision with other objects does not affect them (ie hitbox).
            static const Ogre::String GHOST_ATTRIBUTE;

            static const Ogre::String BBOX_SHAPE_NAME_BOX;
            static const Ogre::String BBOX_SHAPE_NAME_CONVEXHULL;
            static const Ogre::String BBOX_SHAPE_NAME_SPHERE;
            static const Ogre::String BBOX_SHAPE_NAME_TRIMESH;

            PhysicsModel();
            PhysicsModel(const PhysicsModel& o);
            void init(btDynamicsWorld *world, OgreModel *omodel);
            virtual PhysicsModel& operator=(const PhysicsModel& other);
            virtual ~PhysicsModel();
            virtual inline ModelType modelType()
            {
                return MT_PHYSICS;
            }

            ///serialize itself into the given Json object
            virtual void toJson(Json::Value &object);
            ///deserialize itself from the given Json object. return true is successful.
            virtual bool fromJson(Json::Value &object);

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
            virtual void cleanup();
        protected:
            /// Maps a bounding shape string to its enum value. Defaults to sphere.
            BoundingShape BBoxShapeFromString(Ogre::String &shape);

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
    };
}
#endif // STEEL_PHYSICSMODEL_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
