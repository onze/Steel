/*
 *
 */

#include "RigidBodyStateWrapper.h"

namespace Steel
{
    
    RigidBodyStateWrapper::RigidBodyStateWrapper(Ogre::SceneNode *node): BtOgre::RigidBodyState(node),
        mIsTransformed(false)
    {
    }
    
    RigidBodyStateWrapper::RigidBodyStateWrapper(Ogre::SceneNode *node,
                                                 const btTransform &transform,
                                                 const btTransform &offset /*= btTransform::getIdentity()*/):BtOgre::RigidBodyState(node, transform, offset),
        mIsTransformed(false)
    {
    }

    RigidBodyStateWrapper::RigidBodyStateWrapper(const RigidBodyStateWrapper &o): RigidBodyState(o),
        mIsTransformed(o.mIsTransformed)
    {
    }

    void RigidBodyStateWrapper::setWorldTransform(const btTransform &in)
    {
        BtOgre::RigidBodyState::setWorldTransform(in);
        mIsTransformed = true;
    }

    bool RigidBodyStateWrapper::poolTransform()
    {
        bool value = mIsTransformed;
        mIsTransformed = false;
        return value;
    }
}
