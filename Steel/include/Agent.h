#ifndef STEEL_AGENT_H_
#define STEEL_AGENT_H_

#include <limits.h>
#include <exception>

#include <json/json.h>
#include <OgreString.h>
#include <OgreVector3.h>

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
            Json::Value toJson();

            /// Assigns a model to the agent for the given type.
            bool linkToModel(ModelType modelType, ModelId modelId);
            /// Opposite of linkToModel
            void unlinkFromModel(ModelType modelType);

            /// Returns an address to the model of the given type, if any. returns NULL otherwise.
            Model *model(ModelType modelType) const;
            /// Return the id of the model of the given type, if any. returns Steel::INVALID_ID otherwise.
            ModelId modelId(ModelType modelType) const;

            /// Return all ids of all contained model types.
            std::map<ModelType, ModelId> &modelsIds()
            {
                return mModelIds;
            }
            /// Shortcut to Agent::model(MT_OGRE).
            inline OgreModel *ogreModel() const
            {
                return (OgreModel *) model(MT_OGRE);
            }
            /// Shortcut to Agent::modelId(MT_OGRE).
            inline ModelId ogreModelId() const
            {
                return modelId(MT_OGRE);
            }

            /**
             * Make an agent selected or not.
             * Being (de)selected can have different effects on the agent's models.
             */
            void setSelected(bool selected);

            Ogre::Vector3 position() const;
            Ogre::Quaternion rotation() const;
            Ogre::Vector3 scale() const;

            /// Translates the agent by the given vector.
            void move(const Ogre::Vector3 &dpos);
            /// Rotate the agent by r.x in the x axis, etc.
            void rotate(const Ogre::Vector3 &rot);
            /// Rotate the agent by the given quaternion
            void rotate(const Ogre::Quaternion &q);
            /// Rescale the agent by the given factor (current_scale*given_scale).
            void rescale(const Ogre::Vector3 &scale);

            void setPosition(const Ogre::Vector3 &pos);
            void setRotation(const Ogre::Quaternion &rot);
            void setScale(const Ogre::Vector3 &sca);

        private:
            //static stuff
            static AgentId sNextId;

            static inline AgentId getNextId()
            {
                if (sNextId == ULONG_MAX)
                    throw "Steel::Agent::sNextId has reached ULONG_MAX.";
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

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
