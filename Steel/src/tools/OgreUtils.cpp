#include "tools/OgreUtils.h"

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreColourValue.h>
#include <OgrePixelFormat.h>
#include <OgreImage.h>

#include "Debug.h"

namespace Steel
{
    void OgreUtils::destroyAllAttachedMovableObjects(Ogre::SceneNode *node)
    {
        if(!node) return;

        // Destroy all the attached objects
        Ogre::SceneNode::ObjectIterator itObject = node->getAttachedObjectIterator();

        while(itObject.hasMoreElements())
            node->getCreator()->destroyMovableObject(itObject.getNext());

        // Recurse to child SceneNodes
        Ogre::SceneNode::ChildNodeIterator itChild = node->getChildIterator();

        while(itChild.hasMoreElements())
        {
            Ogre::SceneNode *pChildNode = static_cast<Ogre::SceneNode *>(itChild.getNext());
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
        if(v.size() == 0)
            return T();

        if(v.size() == 1)
            return v[0];

        T mean = v[0];

        for(auto it = v.begin() + 1; it != v.end(); ++it)
            mean += *it;

        return mean / float(v.size());
    }
    // template definition
    template Ogre::Vector3 OgreUtils::mean<Ogre::Vector3>(std::vector<Ogre::Vector3> const &);

    void OgreUtils::resourceGroupsInfos()
    {
        Debug::log("== Resources locations ==").indent().endl();
        auto resGroupNames = Ogre::ResourceGroupManager::getSingleton().getResourceGroups();

        for(auto it = resGroupNames.begin(); it != resGroupNames.end(); ++it)
        {
            Debug::log(*it)(":").endl();
            Debug::log("\tlocations:");
            Debug::log(Ogre::ResourceGroupManager::getSingleton().getResourceLocationList(*it)).endl();
            Debug::log("\tfiles:");
            Debug::log(Ogre::ResourceGroupManager::getSingleton().getResourceDeclarationList(*it)).endl();
        }

        Debug::log.unIndent().endl();
    }

    Ogre::TexturePtr OgreUtils::loadTextureFromFile(File const &file, Ogre::String const &name, Ogre::String const &group)
    {
        Ogre::TexturePtr texPtr;

        if(file.exists())
        {
            Ogre::Image img = Ogre::Image().load(file.fullPath(), group);
            texPtr = Ogre::TextureManager::getSingleton().loadImage(name, group, img);
        }
        else
        {
            Debug::error("OgreUtils::loadTextureFromFile(): ")("file not found: ")(file).endl();
            texPtr.setNull();
        }

        return texPtr;
    }

    Ogre::ColourValue OgreUtils::pixelColor(Ogre::TexturePtr const &tex, unsigned const x, unsigned const y)
    {
        Ogre::ColourValue color(Ogre::ColourValue::White);

        if(tex.isNull())
            return color;

        /// segfaults
        size_t index = y * tex->getWidth() + x;

        if(index >= tex->getWidth()*tex->getHeight())
            return color;

        Ogre::PixelBox box(1, 1, 1, Ogre::PixelFormat::PF_A8R8G8B8);
        Ogre::HardwarePixelBufferSharedPtr buffer = tex->getBuffer();
        // windows, see buffer.blitToMemory ?
        const Ogre::PixelBox &pixBox  = buffer->lock(box, Ogre::HardwareBuffer::HBL_NO_OVERWRITE);

        Ogre::uint8 *pDest = static_cast<Ogre::uint8 *>(pixBox.data);
        size_t elemSize = Ogre::PixelUtil::getNumElemBytes(pixBox.format);
        Ogre::uint8 *offset = pDest + index * elemSize;

        switch(box.format)
        {
            case Ogre::PixelFormat::PF_A8R8G8B8:
                color.a = float(*(offset++)) / 255.f;
                color.r = float(*(offset++)) / 255.f;
                color.g = float(*(offset++)) / 255.f;
                color.b = float(*(offset++)) / 255.f;
                break;

            case Ogre::PixelFormat::PF_B8G8R8A8:
            default:
                color.b = float(*(offset++)) / 255.f;
                color.g = float(*(offset++)) / 255.f;
                color.r = float(*(offset++)) / 255.f;
                color.a = float(*(offset++)) / 255.f;
                break;
        }

        buffer->unlock();

        return color;
    }
}
namespace std
{
    bool operator<(Ogre::ColourValue const &c0, Ogre::ColourValue const &c1)
    {
        return c0.getAsRGBA() < c1.getAsRGBA();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
