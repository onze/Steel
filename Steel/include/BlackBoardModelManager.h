#ifndef STEEL_BLACKBOARDMODELMANAGER_H
#define STEEL_BLACKBOARDMODELMANAGER_H

#include "steeltypes.h"

#include "_ModelManager.h"
#include "BlackBoardModel.h"

namespace Steel
{
    class BlackBoardModelManager:public _ModelManager<BlackBoardModel>
    {
        public:
            BlackBoardModelManager(Level *level);
            virtual ~BlackBoardModelManager();

            /// modelType associated with this Manager
            virtual inline ModelType modelType()
            {
                return MT_BLACKBOARD;
            };
            
            ModelId fromSingleJson(Json::Value &model);

        protected:

    };
}
#endif // BLACKBOARDMODELMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
