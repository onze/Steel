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
#include "Model.h"
#include "OgreModel.h"

namespace Steel
{

    class Level;

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

            /**
             * setup new Agent according to data in the json serialization.
             */
            bool fromJson(Json::Value &models);

            bool linkToModel(ModelType modelType, ModelId modelId);

            /**
             * returns an address to the model of the given type, if any. returns NULL otherwise.
             */
            Model *model(ModelType modelType);

            /**
             * return the id of the model of the given type, if any. returns Steel::INVALID_ID otherwise.
             */
            ModelId modelId(ModelType modelType);

            /**
             * return all ids of all contained model types.
             */
            std::map<ModelType, ModelId> &modelsIds()
            {
                return mModelIds;
            }

            /**
             * shortcut to Agent::model(MT_OGRE).
             */
            inline OgreModel *ogreModel()
            {

                return (OgreModel *) model(MT_OGRE);
            }

            /**
             * shortcut to Agent::modelId(MT_OGRE).
             */
            inline ModelId ogreModelId()
            {
                return modelId(MT_OGRE);
            }

            /**
             * make an agent selected or not.
             * being (de)selected can have different effects on the agent's models.
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

            /**
             * unique id.
             */
            AgentId mId;
            /**
             * ptr to the level the agent is in.
             */
            Level *mLevel;
            std::map<ModelType, ModelId> mModelIds;
    };

}

#endif /* AGENT_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
