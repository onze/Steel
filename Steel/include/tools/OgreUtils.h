#ifndef OGREUTILS_H
#define OGREUTILS_H
#include <OgrePrerequisites.h>

namespace Ogre
{
    class SceneNode;
}

namespace Steel
{
    class OgreUtils
    {
        public:
            ///see http://www.ogre3d.org/forums/viewtopic.php?p=445350#p445350
            static void destroySceneNode(Ogre::SceneNode* node);

            ///see http://www.ogre3d.org/forums/viewtopic.php?p=445350#p445350
            static void destroyAllAttachedMovableObjects(Ogre::SceneNode* node);
            
            /// prints resource Groups and their content
            static void resourceGroupsInfos();
    };
}
#endif // OGREUTILS_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
