#ifndef STEEL_BLACKBOARDMODELMANAGER_H
#define STEEL_BLACKBOARDMODELMANAGER_H

#include <json/json.h>

#include "steeltypes.h"

#include "_ModelManager.h"
#include "BlackBoardModel.h"

namespace Steel
{
    class BlackBoardModelManager: public _ModelManager<BlackBoardModel>
    {
    public:
        BlackBoardModelManager(Level *level);
        virtual ~BlackBoardModelManager();

        /// modelType associated with this Manager
        virtual inline ModelType modelType()
        {
            return MT_BLACKBOARD;
        };

        /// Creates a new model and returns its id.
        ModelId newModel();

        bool fromSingleJson(Json::Value &model, ModelId &id);
        bool onAgentLinkedToModel(Agent *agent, ModelId mid);

    protected:


    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
