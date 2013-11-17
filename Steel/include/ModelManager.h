#ifndef STEEL_MODELMANAGER_H_
#define STEEL_MODELMANAGER_H_

#include <json/json.h>
#include <vector>
#include <set>

#include "steeltypes.h"

namespace Steel
{
    class Agent;
    class Model;
    /**
     * Pure abstract class used as a common interface for the template-specialized versions of modelManagers.
     * The way modelManager design is laid out is this way:
     * - ModelManager "implements" the common interface, so that any model manager can be pointed at with the same pointer.
     * - _ModelManager<class M> is a templated subclass of ModelManager, that implements common behavior wrt to model.
     * - ModelManager<BlahModel> is a subclass that specializes in one single type of model, and implements details specifics to
     * the respective model it takes care of.
     */
    class ModelManager
    {
        public:
            static const char *MODELS_ATTRIBUTES;
            
            virtual Model *at(ModelId id)=0;

            virtual void incRef(ModelId id)=0;
            virtual void decRef(ModelId id)=0;
            virtual bool isValid(ModelId id)=0;
            /// Triggered by the level after an agent linked itself to an owned model
            virtual bool onAgentLinkedToModel(Agent *agent, ModelId id)=0;
            virtual void onAgentUnlinkedFromModel(Agent *agent, ModelId id)=0;

            virtual std::vector<ModelId> fromJson(Json::Value &models)=0;
            /// Meant to be reimplemented by subclass when models use extra params in their M::fromJson
            virtual bool fromSingleJson(Json::Value &model, ModelId &id)=0;
            virtual void toJson(Json::Value &object)=0;
            /// Returns the model tags, or an empty set if the given id is not valid.
            virtual std::set<Tag> modelTags(ModelId mid)=0;
            
            /// Clears every models.
            virtual void clear()=0;
    };

}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
