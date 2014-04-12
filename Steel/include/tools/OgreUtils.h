#ifndef STEEL_OGREUTILS_H
#define STEEL_OGREUTILS_H

#include "steeltypes.h"
#include "tools/File.h"

namespace Ogre
{
    class SceneNode;
    class ColourValue;
}

namespace Steel
{
    class OgreUtils
    {
    public:
        /// See http://www.ogre3d.org/forums/viewtopic.php?p=445350#p445350
        static void destroySceneNode(Ogre::SceneNode *node);

        /// See http://www.ogre3d.org/forums/viewtopic.php?p=445350#p445350
        static void destroyAllAttachedMovableObjects(Ogre::SceneNode *node);

        /// Returns the mean of the given class. Class must implement operator+(T const &) and operator/(float).
        template<class T>
        static T mean(std::vector<T> const &v);

        /// Prints resource Groups and their content
        static void resourceGroupsInfos();

        /// Loads the given file as a 2d texture and returns a pointer to it. Requires the Ogre resource name and group to register it in.
        static Ogre::TexturePtr loadTextureFromFile(File const &file, Ogre::String const &name, Ogre::String const &group);

        /// Returns the pixel color of the given texture at the given location. White by default.
        static Ogre::ColourValue pixelColor(Ogre::TexturePtr const &tex, unsigned const x, unsigned const y);
    };
}

namespace std
{
    /// Ogre::ColourValue comparator
    bool operator<(Ogre::ColourValue const &c0, Ogre::ColourValue const &c1);
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
