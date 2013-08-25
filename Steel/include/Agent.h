#ifndef STEEL_AGENT_H_
#define STEEL_AGENT_H_

#include <limits.h>
#include <exception>
#include <map>

#include <json/json.h>
#include <OgreString.h>
#include <OgreVector3.h>

#include "steeltypes.h"
#include "PhysicsModel.h"

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
            static const char *TAGS_ATTRIBUTES;

            Agent(AgentId id, Level *level);
            virtual ~Agent();
            Agent(const Agent &t);
            /// shallow copy
            Agent &operator=(const Agent &);

            inline AgentId id()
            {
                return mId;
            }
            inline bool isFree()
            {
                return mId==INVALID_ID;
            }

            void init(AgentId id);
            void cleanup();

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

            /// Shortcut to Agent::model(MT_PHYSICS).
            inline PhysicsModel *physicsModel() const
            {
                return (PhysicsModel *) model(MT_PHYSICS);
            }
            /// Shortcut to Agent::modelId(MT_PHYSICS).
            inline ModelId physicsModelId() const
            {
                return modelId(MT_PHYSICS);
            }

            inline bool isSelected()
            {
                return mIsSelected;
            }

            /**
             * Make an agent selected or not.
             * Being (de)selected can have different effects on the agent's models.
             */
            void setSelected(bool selected);

            //////////////////////////////////////////////////////////////////////
            // OgreModel/PhysicsModel shortcuts
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
            void rescale(const Ogre::Vector3& sca);

            void setPosition(const Ogre::Vector3 &pos);
            void setRotation(const Ogre::Quaternion &rot);
            void setScale(const Ogre::Vector3 &sca);
            
            void tag(Tag tag);
            void tag(std::set<Tag> tags);
            void untag(Steel::Tag tag);
            void untag(std::set<Tag> tags);
            std::set<Tag> tags() const;

        private:
            /// Unique id.
            AgentId mId;

            /// Ptr to the level the agent is in.
            Level *mLevel;

            std::map<ModelType, ModelId> mModelIds;

            /// state flag
            bool mIsSelected;

            /** 
             * The agent's tags. Since some tags are refs to the agent models, 
             * a ref count (map value) is kept, to support unlinking from model.
             */
            std::map<Tag, unsigned> mTags;
    };

}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
