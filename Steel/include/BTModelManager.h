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
            BTModelManager(Level *level,Ogre::String mPath);
            virtual ~BTModelManager();
            
            virtual ModelId fromSingleJson(Json::Value &root);
            virtual ModelType modelType();

        protected:
            // owned
            BTShapeManager mBTShapeMan;
            /// Path to BT folder
            File mBasePath;
    };

}
#endif /* STEEL_BTMODELMANAGER_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
