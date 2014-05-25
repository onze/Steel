#ifndef STEEL_OGREMODELMANAGER_H_
#define STEEL_OGREMODELMANAGER_H_

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

            OgreModelManager(Level *level, Ogre::SceneManager *sceneManager, Ogre::SceneNode *levelRoot);
            virtual ~OgreModelManager();

            /// Initialize a new OgreModel and returns its identifier.
            ModelId newModel(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot);

            /// Callback used to sync a PhysicsModel to its OgreModel upon linkage.
            bool onAgentLinkedToModel(Agent *agent, ModelId id);

            ///////////////////////////////////////////////////////////
            //getters
            inline Ogre::SceneManager *sceneManager()
            {
                return mSceneManager;
            }

            virtual ModelType modelType()
            {
                return ModelType::OGRE;
            }

        protected:
            bool deserializeToModel(Json::Value const&model, ModelId &mid) override;
            
            Ogre::SceneManager *mSceneManager;
            Ogre::SceneNode *mLevelRoot;
    };

}

#endif /* STEEL_OGREMODELMANAGER_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
