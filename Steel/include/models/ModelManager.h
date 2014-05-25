#ifndef STEEL_MODELMANAGER_H_
#define STEEL_MODELMANAGER_H_

#include "steeltypes.h"

namespace Steel
{
    class Agent;
    class Model;
    /**
     * Pure abstract class used as a common interface for the template-specialized versions of model managers.
     * Model managers are designed this way:
     * - ModelManager is the common interface, so that any model manager can be pointed at with the same pointer.
     * - _ModelManager<class M> is a templated subclass of ModelManager, that implements common behavior wrt to model.
     * - ModelManager<BlahModel> is a subclass that specializes in one single type of model, and implements details specifics to
     * the respective model it takes care of.
     * 
     * So for details about methods listed here, have a look into _ModelManager.h
     */
    class ModelManager
    {
    public:
        static const char *MODELS_ATTRIBUTES;

        virtual Model *at(ModelId id) = 0;

        virtual void incRef(ModelId id) = 0;
        virtual void decRef(ModelId id) = 0;
        virtual bool isValid(ModelId id) = 0;
        /// Triggered by the level after an agent linked itself to an owned model
        virtual bool onAgentLinkedToModel(Agent *agent, ModelId id) = 0;
        virtual void onAgentUnlinkedFromModel(Agent *agent, ModelId id) = 0;

        virtual std::vector<ModelId> fromJson(Json::Value const&nodes) = 0;
        /// Meant to be reimplemented by subclasses when models use extra params in their M::fromJson
        virtual bool fromSingleJson(Json::Value const &node, ModelId &id, bool updateModel = false) = 0;
        
        /// Serializes all models referenced by modelIds in object node.
        virtual void toJson(Json::Value &nodes, std::list<ModelId> const &modelIds) = 0;
        /// Redirects the call to the pointed model. Returns false if the model is invalid.
        virtual bool toSingleJson(ModelId mid, Json::Value &node) = 0;

        // /// Swaps the 2 models.
        //virtual void swapModels(ModelId midLeft, ModelId midRight) = 0;

        /// Returns the model tags, or an empty set if the given id is not valid.
        virtual std::set<Tag> modelTags(ModelId mid) = 0;

        /// Clears every models.
        virtual void clear() = 0;
    };

}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
