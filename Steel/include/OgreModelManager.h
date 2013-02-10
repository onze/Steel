#ifndef STEEL_OGREMODELMANAGER_H_
#define STEEL_OGREMODELMANAGER_H_

#include <list>

#include <json/json.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

#include "steeltypes.h"
#include "_ModelManager.h"
#include "OgreModel.h"

namespace Steel
{
    class Level;
    /**
     * instances of this class handle ogre related stuff.
     */
    class OgreModelManager: public _ModelManager<OgreModel>
    {
        public:
            OgreModelManager(Level *level,Ogre::SceneManager *sceneManager, Ogre::SceneNode *levelRoot);
            virtual ~OgreModelManager();

            /**
             * batched call to fromSingleJson.
             */
            std::vector<ModelId> fromJson(Json::Value &models);

            /**
             * initialize a new OgreModel according to data in the json serialization.
             */
            ModelId fromSingleJson(Json::Value &model);

            /**
             * initialize a new OgreModel and returns its identifier.
             */
            ModelId newModel(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot);
            ///////////////////////////////////////////////////////////
            //getters
            inline Ogre::SceneManager *sceneManager()
            {
                return mSceneManager;
            }

            virtual ModelType modelType()
            {
                return MT_OGRE;
            };

        protected:
            Ogre::SceneManager *mSceneManager;
            Ogre::SceneNode *mLevelRoot;
    };

}

#endif /* STEEL_OGREMODELMANAGER_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
