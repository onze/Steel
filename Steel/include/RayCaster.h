/*
 * RayCaster.h
 *
 *  Created on: 2011-06-19
 *      Author: onze
 */

#ifndef STEEL_RAYCASTER_H_
#define STEEL_RAYCASTER_H_

#include <list>

#include <OgreRay.h>
#include <OgreVector3.h>
#include <OgreQuaternion.h>

#include "steeltypes.h"
#include "EngineEventListener.h"

namespace Ogre
{
    class SceneManager;
    class SceneNode;
    class Vector3;
    class Entity;
    class RaySceneQuery;
}

namespace Steel
{
    class Engine;
    /**
     * Instances of this class do raycasting.
     * Make sure an Ogre context is ready before instanciating them though.
     * the code is an adaptation of http://www.ogre3d.org/tikiwiki/Raycasting+to+the+polygon+level to Steel.
     */
    class RayCaster:public EngineEventListener
    {
        public:
            RayCaster(Engine *engine);
            virtual ~RayCaster();
            /**
             * raycast from a point in to the scene.
             * returns success or failure.
             * on success the given list of filled with the results.
             */
            bool fromRay(Ogre::Ray &ray, std::list<Ogre::SceneNode *> &nodes);
            void getMeshInformation(Ogre::Entity *entity,
                                    size_t &vertex_count,
                                    Ogre::Vector3* &vertices,
                                    size_t &index_count,
                                    Ogre::uint32* &indices,
                                    const Ogre::Vector3 &position,
                                    const Ogre::Quaternion &orient,
                                    const Ogre::Vector3 &scale);
            
            virtual void onLevelSet(Level *level);
            virtual void onLevelUnset(Level *level);
        protected:
            //not owned
            Ogre::SceneManager *mSceneManager;
            //owned
            Ogre::RaySceneQuery *mRaySceneQuery;
    };

}

#endif /* STEEL_RAYCASTER_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
