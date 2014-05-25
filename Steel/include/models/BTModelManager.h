#ifndef STEEL_BTMODELMANAGER_H
#define STEEL_BTMODELMANAGER_H

#include "steeltypes.h"
#include "_ModelManager.h"
#include "BTModel.h"
#include "tools/File.h"
#include "BT/BTShapeManager.h"

namespace Steel
{
    class Level;
    class BTModelManager: public _ModelManager<BTModel>
    {
    public:

        BTModelManager(Level *level, Ogre::String mPath);
        virtual ~BTModelManager();
        
        ModelType modelType() const {return ModelType::BT;}
        
        bool fromSingleJson(Json::Value const&model, ModelId &mid, bool updateModel = false) override;

        /**
         * Reads the file system at the given directory, build the corresponding BTree,
         * and returns the id of a BTModel executing it.
         */
        bool buildFromFile(Steel::File const &rootFile, Steel::ModelId &id, bool updateModel);

        /// Main loop iteration. Update each BT model.
        void update(float timestep);

        /// Fullpath of the BT implementing default path following.
        Ogre::String genericFollowPathModelPath();
        
        bool onAgentLinkedToModel(Agent *agent, ModelId mid);

    protected:
        /// Name of the BT implementing default path following.
        static const Ogre::String GENERIC_FOLLOW_PATH_MODEL_NAME;
        // owned
        BTShapeManager mBTShapeMan;
        /// Path to BT folder
        File mBasePath;
        
        /// Hidden. Do not use.
        bool deserializeToModel(Json::Value const &root, ModelId &mid) override;
    private:
    };

}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
