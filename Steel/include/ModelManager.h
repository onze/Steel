#ifndef STEEL_MODELMANAGER_H_
#define STEEL_MODELMANAGER_H_

#include <json/json.h>
#include <vector>

#include "steeltypes.h"

namespace Steel
{
    class Agent;
    class Model;
    /**
     * Pure abstract class used as a common interface for the template-specialized versions of modelManagers.
     * The way modelManager design is laid out is this way:
     * - ModelManager "implements" the common interface, so that any model manager can be pointed at with the same pointer.
     * - _ModelManager<M> is a templated subclass of ModelManager, that implements common behavior.
     * - ModelManager<BlahModel> are subclasses that specialize in one single type of model, and implements details specifics to
     * the respective model they take care of.
     */
    class ModelManager
    {
        public:
            virtual Model *at(ModelId id)=0;

            virtual void incRef(ModelId id)=0;
            virtual void decRef(ModelId id)=0;
            virtual bool isValid(ModelId id)=0;
            /// Triggered by the level after an agent linked itself to an owned model
            virtual bool onAgentLinkedToModel(Agent *agent, ModelId mid)=0;

            virtual std::vector<ModelId> fromJson(Json::Value &models)=0;
            /// Meant to be reimplemented by subclass when models use extra params in their M::fromJson
            virtual ModelId fromSingleJson(Json::Value &models)=0;
            virtual void toJson(Json::Value &object)=0;
    };

}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
