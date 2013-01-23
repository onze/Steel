/*
 * Manager.h
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#ifndef _MODELMANAGER_H_
#define _MODELMANAGER_H_

#include <json/json.h>

#include <vector>
#include <list>

#include "Debug.h"
#include "steeltypes.h"
#include "ModelManager.h"

namespace Steel
{

//M is the managed model type
    template<class M>
    class _ModelManager : public ModelManager
    {
        public:
            _ModelManager();
            virtual ~_ModelManager();
            /**
             * Returns a pointer to the model referenced by the given id, if the model is in use. Return NULL
             * otherwise. Since model addresses can change without notice, models pointers lifetime is not
             * supposed to be longer than the function they're created in (and watchout when multithreading).
             * ModelIds are meant for this, as seen in Agent.h/cpp.
             */
            virtual M *at(ModelId id);

            /**
             * Increments the ref count of the given model.
             */
            virtual bool incRef(ModelId id);

            /**
             * clears every models from its memory.
             */
            void clear();

            /**
             * returns true if the given model is in use.
             */
            bool isValid(ModelId id);

            /**
             * decrements the given model, and deletes it if nobody uses it.
             */
            void releaseModel(ModelId modelId);

            /**
             * initialize new Model according to data in the json serialization.
             */
            virtual std::vector<ModelId> fromJson(Json::Value &models)
            {
                Debug::warning("_ModelManager::fromJson(): NotImplemented by subclass.");
                return std::vector<ModelId>();
            }

            /**
             * initialize new Model according to data in the json serialization.
             */
            virtual ModelId fromSingleJson(Json::Value &model)
            {
                Debug::warning("_ModelManager::fromSingleJson(): NotImplemented by subclass.");
                return INVALID_ID;
            }

            /**
             * dump all models' json representation into the given object.
             */
            void toJson(Json::Value &object);
        protected:
            /**
             * Finds a free id to place the given model at. returns this ModelId.
             * If allocation was not possible, returns Steel::INVALID_ID.
             */
            ModelId allocateModel();
            /**
             * contains models.
             */
            std::vector<M> mModels;
            std::list<ModelId> mModelsFreeList;

    };
}

#endif /* _MODELMANAGER_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
