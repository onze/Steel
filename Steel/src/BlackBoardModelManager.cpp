#include "BlackBoardModelManager.h"

namespace Steel
{
    BlackBoardModelManager::BlackBoardModelManager(Level *level):
        _ModelManager<BlackBoardModel>(level)
    {

    }

    BlackBoardModelManager::~BlackBoardModelManager()
    {

    }

    ModelId BlackBoardModelManager::fromSingleJson(Json::Value &model)
    {
        ModelId id = allocateModel();
        //get values for load
        //incRef(id);
        int loadingOk=mModels[id].fromJson(model);
        //TODO discard, quarantine, repair ?
        if(!loadingOk)
        {
            id=INVALID_ID;
        }
        return id;
    }
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
