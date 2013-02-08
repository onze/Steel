#include "tools/OgreUtils.h"

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

#include <Debug.h>

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

    template<class T>
    T OgreUtils::mean(std::vector<T> const &v)
    {
        if(v.size()==0)
            return T();
        if(v.size()==1)
            return v[0];
        T mean=v[0];
        for(auto it=v.begin()+1; it!=v.end(); ++it)
            mean+=*it;
        return mean/float(v.size());
    }
    // template definition
    template Ogre::Vector3 OgreUtils::mean<Ogre::Vector3>(std::vector<Ogre::Vector3> const &);

    void OgreUtils::resourceGroupsInfos()
    {
        Debug::log("== Resources locations ==").indent().endl();
        auto resGroupNames=Ogre::ResourceGroupManager::getSingleton().getResourceGroups();
        for(auto it=resGroupNames.begin(); it!=resGroupNames.end(); ++it)
        {
            Debug::log(*it)(":").endl();
            Debug::log("\tlocations:");
            Debug::log(Ogre::ResourceGroupManager::getSingleton().getResourceLocationList(*it)).endl();
            Debug::log("\tfiles:");
            Debug::log(Ogre::ResourceGroupManager::getSingleton().getResourceDeclarationList(*it)).endl();
        }
        Debug::log.unIndent().endl();
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
