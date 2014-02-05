#ifndef STEEL_BTRIGIDBODYWRAPPER_H
#define STEEL_BTRIGIDBODYWRAPPER_H

#include <BtOgrePG.h>

namespace Steel
{
    /**
     * Because neither btOgre nor bullet offer motionState changes notifications, 
     * this class subclasses btRigidBody and can be pooled to retreive whether 
     * it has been modified since the last time it was pooled.
     */
    class RigidBodyStateWrapper: public BtOgre::RigidBodyState
    {
    public:
        RigidBodyStateWrapper(Ogre::SceneNode *node);
        RigidBodyStateWrapper(Ogre::SceneNode *node, 
                              const btTransform &transform, 
                              const btTransform &offset = btTransform::getIdentity());
        RigidBodyStateWrapper(const RigidBodyStateWrapper &o);
        RigidBodyStateWrapper &operator=(const RigidBodyStateWrapper &o);
        
        /// Returns whether this body worldtransform has changed since the last call. Resets the flag.
        bool poolTransform();
        
        /// btRigidBody overwriting
        virtual void setWorldTransform(const btTransform &in);
    protected:
        bool mIsTransformed;
    };
}
#endif // STEEL_BTRIGIDBODYWRAPPER_H
