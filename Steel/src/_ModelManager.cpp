/*
 * Manager.cpp
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#include "_ModelManager.h"
#include "Debug.h"
#include "Model.h"

namespace Steel
{

    template<class M>
    _ModelManager<M>::_ModelManager() :
        ModelManager()
    {
        mModels = std::vector<M>();
        mModelsFreeList = std::list<ModelId>();
    }

    template<class M>
    _ModelManager<M>::~_ModelManager()
    {
        clear();
    }

    template<class M>
    M *_ModelManager<M>::at(ModelId id)
    {
        if (!isValid(id))
            return NULL;

        return &(mModels[id]);
    }

    template<class M>
    bool _ModelManager<M>::incRef(ModelId id)
    {
        if (id < 0 || id >= mModels.size())
        {
            Debug::warning("void _ModelManager<")(modelTypesAsString[mModels[id].modelType()]);
            Debug::warning(">::incRef(): modelId ")(id)("\" does not exist.").endl();
            return false;
        }
        mModels[id].incRef();
        return true;
    }

    template<class M>
    void _ModelManager<M>::clear()
    {
        mModels.clear();
        mModelsFreeList.clear();
    }

    template<class M>
    ModelId _ModelManager<M>::allocateModel()
    {
        ModelId id = INVALID_ID;
        if (mModelsFreeList.size() > 0)
        {
            id = mModelsFreeList.front();
            mModelsFreeList.pop_front();
        }
        else
        {
            id = (ModelId) mModels.size();
            mModels.push_back(M());
        }
//	mModels[id].incRef();
        return id;
    }

    template<class M>
    bool _ModelManager<M>::isValid(ModelId id)
    {
        return id >= 0 && id < mModels.size() && !mModels[id].isFree();
    }

    template<class M>
    void _ModelManager<M>::releaseModel(ModelId id)
    {
        if (!isValid(id))
            return;
        M *m = at(id);
        m->decRef();
        //TODO: use a heap (priority queue), with (mModelsFreeList.size()-id) as priority
        if (m->isFree())
            mModelsFreeList.push_front(id);
    }

    template<class M>
    void _ModelManager<M>::toJson(Json::Value &object)
    {
        Debug::log("_ModelManager(): ")(mModels.size())(" models in stock").endl();
        for (ModelId id = 0; id < mModels.size(); ++id)
        {
            Model *m = (Model *) &(mModels[id]);
            if (m->isFree())
                continue;
            m->toJson(object[Ogre::StringConverter::toString(id)]);
        }
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
