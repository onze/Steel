#include "BlackBoardModelManager.h"
#include <BTModel.h>
#include <Agent.h>

namespace Steel
{
    BlackBoardModelManager::BlackBoardModelManager(Level *level):
    _ModelManager<BlackBoardModel>(level)
    {

    }

    BlackBoardModelManager::~BlackBoardModelManager()
    {

    }
    
    ModelId BlackBoardModelManager::newModel()
    {
        ModelId id = allocateModel();
        
        if(!mModels[id].init(this))
        {
            deallocateModel(id);
            id = INVALID_ID;
        }
        
        return id;
    }

    bool BlackBoardModelManager::fromSingleJson(Json::Value &model, ModelId &id)
    {
        id = allocateModel(id);
        if(INVALID_ID == id)
            return false;
        
        if(!mModels[id].fromJson(model))
        {
            deallocateModel(id);
            id = INVALID_ID;
            return false;
        }
        return true;
    }
    
    bool BlackBoardModelManager::onAgentLinkedToModel(Agent *agent, ModelId mid)
    {
        return true;
    }
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
