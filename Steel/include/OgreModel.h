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
            Ogre::Vector3 scale();
            
            
            /// Translates the model's scenenode by the given vector.
            void move(Ogre::Vector3 const &dpos);
            /// Rotate the model's scenenode by r.x in the x axis, etc.
            void rotate(Ogre::Vector3 &r);
            /// Rotate the model's scenenode by the given quaternion
            void rotate(Ogre::Quaternion &q);
            /// Rescale the model's scenenode by the given factor (current_scale*given_scale).
            void rescale(Ogre::Vector3 const &scale);
            
            void setPosition(Ogre::Vector3 const &pos);
            void setRotation(Ogre::Quaternion const &rot);
            void setScale(Ogre::Vector3 const &sca);
            void setSelected(bool selected);
            
            void setNodeAny(Ogre::Any any);

            ///deserialize itself from the given Json object
            virtual bool fromJson(Json::Value &node, Ogre::SceneNode *levelRoot=NULL, Ogre::SceneManager *sceneManager=NULL);
            ///serialize itself into the given Json object
            virtual void toJson(Json::Value &node);

            // getters
            inline Ogre::Entity* entity()
            {
                return mEntity;
            }
            inline Ogre::SceneNode* sceneNode()
            {
                return mSceneNode;
            }

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
            Ogre::SceneManager *mSceneManager;
    };

}

#endif /* OGREMODEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
