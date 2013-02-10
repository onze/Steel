#ifndef STEEL_CYLINDER_H
#define STEEL_CYLINDER_H
#include <OgreMesh.h>

namespace Ogre
{
    class SceneNode;
    class SceneManager;
}

namespace Steel
{
    class Cylinder
    {
            /**
             * TODO: Make it the subclass of a common manual object instancer.
             */
        private:
            Cylinder();
            Cylinder(const Cylinder& other);
            virtual ~Cylinder();
            virtual Cylinder& operator=(const Cylinder& other);
            virtual bool operator==(const Cylinder& other) const;

        public:
            static Ogre::MeshPtr getMesh(Ogre::SceneManager *sceneManager,
                                         int radius,
                                         int height);
            static Ogre::SceneNode *getSceneNode(Ogre::SceneManager *sceneManager,
                                                 Ogre::SceneNode *parent,
                                                 Ogre::String name,
                                                 int radius,
                                                 int height);
    };

}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
