#ifndef STEEL_PHYSICSMODEL_H
#define STEEL_PHYSICSMODEL_H

#include <stack>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include "Model.h"

class btDynamicsWorld;

namespace Steel
{
    class OgreModel;
    class PhysicsModel: public Model
    {
        public:
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
        protected:
            virtual void cleanup();
            //not owned
            btDynamicsWorld *mWorld;
            //owned
            btRigidBody* mBody;
            Ogre::Real mMass;
            bool mIsKinematics;
            /// kinematics/rigidBody states stack
            std::stack<bool> mStates;
    };
}
#endif // STEEL_PHYSICSMODEL_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
