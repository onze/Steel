/*
 * Agent.h
 *
 *  Created on: 2011-06-15
 *      Author: onze
 */

#ifndef AGENT_H_
#define AGENT_H_

#include <limits.h>
#include <exception>

#include <json/json.h>
#include <OgreString.h>

#include "steeltypes.h"

namespace Steel
{

    class Level;
    class Model;
    class OgreModel;

    /**
     * Agent is the base class of Steel objects.
     *
     * Agents use composition of Model subclasses to achieve different behaviors. One can think of
     * an agent as an entry in a table, that only contains ids of models it is made of.
     */
    class Agent
    {
        public:
            Agent(Level *level);
            virtual ~Agent();
            Agent(const Agent &t);
            Agent &operator=(const Agent &);

            //getter
            inline AgentId id()
            {
                return mId;
            }

            /// Setup new Agent according to data in the json serialization.
            bool fromJson(Json::Value &models);
            
            /// Assigns a model to the agent for the given type.
            bool linkToModel(ModelType modelType, ModelId modelId);
            
            /// Opposite of linkToModel
            void unlinkFromModel(ModelType modelType);

            /// Returns an address to the model of the given type, if any. returns NULL otherwise.
            Model *model(ModelType modelType);

            /// Return the id of the model of the given type, if any. returns Steel::INVALID_ID otherwise.
            ModelId modelId(ModelType modelType);

            /// Return all ids of all contained model types.
            std::map<ModelType, ModelId> &modelsIds()
            {
                return mModelIds;
            }

            /// Shortcut to Agent::model(MT_OGRE).
            inline OgreModel *ogreModel()
            {
                return (OgreModel *) model(MT_OGRE);
            }

            /// Shortcut to Agent::modelId(MT_OGRE).
            inline ModelId ogreModelId()
            {
                return modelId(MT_OGRE);
            }

            /**
             * Make an agent selected or not. 
             * Being (de)selected can have different effects on the agent's models.
             */
            void setSelected(bool selected);

            Json::Value toJson();
        private:
            //static stuff
            static AgentId sNextId;

            static inline AgentId getNextId()
            {
                if (sNextId == ULONG_MAX)
                    throw "Steel::Thing::sNextId has reached ULONG_MAX.";
                return sNextId++;
            }
            ;
            //end of static stuff

            /// Unique id.
            AgentId mId;
            
            /// Ptr to the level the agent is in.
            Level *mLevel;
            
            std::map<ModelType, ModelId> mModelIds;
    };

}

#endif /* AGENT_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
