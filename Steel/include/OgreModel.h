#ifndef STEEL_OGREMODEL_H_
#define STEEL_OGREMODEL_H_

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
                        Ogre::Vector3 scale,
                        Ogre::SceneNode *mLevelRoot,
                        Ogre::SceneManager *mSceneManager);
            OgreModel(const OgreModel &m);
            OgreModel &operator=(const OgreModel &m);
            virtual ~OgreModel();

            virtual ModelType modelType();
            
            Ogre::Vector3 position() const;
            Ogre::Quaternion rotation() const;
            Ogre::Vector3 scale() const;
            
            /// Translates the model's scenenode by the given vector.
            void move(const Ogre::Vector3 &dpos);
            /// Rotate the model's scenenode by r.x in the x axis, etc.
            void rotate(const Ogre::Vector3 &r);
            /// Rotate the model's scenenode by the given quaternion
            void rotate(const Ogre::Quaternion &q);
            /// Rescale the model's scenenode by the given factor (current_scale*given_scale).
            void rescale(const Ogre::Vector3 &scale);
            
            void setPosition(const Ogre::Vector3 &pos);
            void setRotation(const Ogre::Quaternion &rot);
            void setScale(const Ogre::Vector3 &sca);
            
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

#endif /* STEEL_OGREMODEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
