#ifndef STEEL_OGREUTILS_H
#define STEEL_OGREUTILS_H

#include <steeltypes.h>

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

            /// Returns the mean of the given class. Class must implement operator+(T const &) and operator/(float).
            template<class T>
            static T mean(std::vector<T> const &v);

            /// prints resource Groups and their content
            static void resourceGroupsInfos();
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
