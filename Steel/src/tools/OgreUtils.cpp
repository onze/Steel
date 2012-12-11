#include "tools/OgreUtils.h"

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

namespace Steel
{
    void OgreUtils::destroyAllAttachedMovableObjects(Ogre::SceneNode *node)
    {
        if(!node) return;

        // Destroy all the attached objects
        Ogre::SceneNode::ObjectIterator itObject = node->getAttachedObjectIterator();

        while ( itObject.hasMoreElements() )
            node->getCreator()->destroyMovableObject(itObject.getNext());

        // Recurse to child SceneNodes
        Ogre::SceneNode::ChildNodeIterator itChild = node->getChildIterator();

        while (itChild.hasMoreElements())
        {
            Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
            OgreUtils::destroyAllAttachedMovableObjects(pChildNode);
        }
    }

    void OgreUtils::destroySceneNode(Ogre::SceneNode *node)
    {
        if(!node)
            return;
        OgreUtils::destroyAllAttachedMovableObjects(node);
        node->removeAndDestroyAllChildren();
        node->getCreator()->destroySceneNode(node);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
