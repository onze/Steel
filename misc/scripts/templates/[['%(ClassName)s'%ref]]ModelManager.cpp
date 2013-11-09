#include "[['%(ClassName)s'%ref]]ModelManager.h"

namespace Steel
{
    [['%(ClassName)s'%ref]]ModelManager::[['%(ClassName)s'%ref]]ModelManager(Level *level):
        _ModelManager<[['%(ClassName)s'%ref]]Model>(level)
    {

    }

    [['%(ClassName)s'%ref]]ModelManager::~[['%(ClassName)s'%ref]]ModelManager()
    {

    }

    bool [['%(ClassName)s'%ref]]ModelManager::fromSingleJson(Json::Value &model, ModelId &id)
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
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
