/*
 * OgreModel.h
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#ifndef OGREMODEL_H_
#define OGREMODEL_H_

#include <json/json.h>

#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <OgreEntity.h>

#include "Model.h"

namespace Steel
{

    class OgreModel: public Model
    {
        public:
            OgreModel();
            void init(	Ogre::String meshName,
                        Ogre::Vector3 pos,
                        Ogre::Quaternion rot,
                        Ogre::SceneNode *mLevelRoot,
                        Ogre::SceneManager *mSceneManager);
            OgreModel(const OgreModel &m);
            OgreModel &operator=(const OgreModel &m);
            virtual ~OgreModel();

            virtual ModelType modelType();
            Ogre::Vector3 position();
            Ogre::Quaternion rotation();
            void rotate(Ogre::Vector3 &rotation);
            void setNodeAny(Ogre::Any any);
            void setPosition(Ogre::Vector3 const &pos);
            void setRotation(Ogre::Quaternion const &rot);
            void setSelected(bool selected);
            void translate(Ogre::Vector3 t);

            ///deserialize itself from the given Json object
            virtual bool fromJson(Json::Value &node, Ogre::SceneNode *levelRoot=NULL, Ogre::SceneManager *sceneManager=NULL);

            ///serialize itself into the given Json object
            virtual void toJson(Json::Value &node);

        protected:
            ///made private to forbid its use. The deserialisation method to use needs more params.
            virtual bool fromJson(Json::Value &node)
            {
                node.isNull();
                return false;
            }
            ;
            virtual void cleanup();
            Ogre::SceneNode *mSceneNode;
            Ogre::Entity *mEntity;
    };

}

#endif /* OGREMODEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
