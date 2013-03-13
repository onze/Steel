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

    ModelId [['%(ClassName)s'%ref]]ModelManager::fromSingleJson(Json::Value &model)
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
