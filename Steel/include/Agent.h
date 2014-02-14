#ifndef STEEL_AGENT_H_
#define STEEL_AGENT_H_

#include <limits.h>
#include <exception>
#include <map>

#include <list>
#include <json/json.h>
#include <OgreString.h>
#include <OgreVector3.h>

#include "steeltypes.h"
#include "SignalEmitter.h"

namespace Steel
{

    class Level;
    class Model;

    class OgreModel;
    class PhysicsModel;
    class LocationModel;
    class BTModel;
    class BlackBoardModel;

    /**
     * Agent is the base class of Steel objects.
     *
     * Agents use composition of Model subclasses to achieve different behaviors. One can think of
     * an agent as an entry in a table, that only contains ids of models it is made of.
     */
    class Agent : public SignalEmitter
    {
    public:
        static const char *TAGS_ATTRIBUTE;
        static const char *ID_ATTRIBUTE;
        static const char *BEHAVIORS_STACK_ATTRIBUTE;

        Agent(AgentId id, Level *level);
        virtual ~Agent();
        Agent(const Agent &t);
        /// shallow copy
        Agent &operator=(const Agent &);

        inline AgentId id() {return mId;}
        inline bool isFree() {return mId == INVALID_ID;}

        //void init(AgentId id);
        void cleanup();

        /// Setup new Agent according to data in the json serialization.
        bool fromJson(Json::Value &models);
        Json::Value toJson();

        /// Assigns a model to the agent for the given type.
        bool linkToModel(ModelType modelType, ModelId modelId);
        /// Opposite of linkToModel
        void unlinkFromModel(ModelType modelType);

        /// Returns an address to the model of the given type, if any. returns nullptr otherwise.
        Model *model(ModelType modelType) const;
        /// Return the id of the model of the given type, if any. returns Steel::INVALID_ID otherwise.
        ModelId modelId(ModelType modelType) const;

        /// Return all ids of all contained model types.
        std::map<ModelType, ModelId> &modelsIds() {return mModelIds;}

        inline OgreModel *ogreModel() const {return (OgreModel *) model(MT_OGRE);}
        inline ModelId ogreModelId() const {return modelId(MT_OGRE);}

        inline PhysicsModel *physicsModel() const {return (PhysicsModel *) model(MT_PHYSICS);}
        inline ModelId physicsModelId() const {return modelId(MT_PHYSICS);}

        inline LocationModel *locationModel() const {return (LocationModel *) model(MT_LOCATION);}
        inline ModelId locationModelId() const {return modelId(MT_LOCATION);}

        inline BTModel *btModel() const {return (BTModel *) model(MT_BT);}
        inline ModelId btModelId() const {return modelId(MT_BT);}

        inline BlackBoardModel *blackBoardModel() const {return (BlackBoardModel *) model(MT_BLACKBOARD);}
        inline ModelId blackBoardModelId() const {return modelId(MT_BLACKBOARD);}

        inline bool isSelected() {return mIsSelected;}

        /**
         * Makes an agent un/selected.
         * Being (de)selected can have different effects on the agent's models.
         */
        void setSelected(bool selected);

        bool setBTPath(Ogre::String const &name);
        void unsetBTPath();
        bool hasBTPath();
        /**
         * Sets a new BTModel as current one (pushing any previously set one on the stack).
         * This bt makes the agent follow the path which the given agent belongs to.
         * Returns operation success.
         */
        bool followNewPath(AgentId aid);
        bool stopFollowingPath(AgentId aid);
        /// Pushes the current BT (if any) on the stack, and set the given one as current. Returns operation success.
        bool pushBT(ModelId btid);
        /// Stops&forgets the current bt, and restores last pushed bt as current one. Returns operation success.
        void popBT();

        //////////////////////////////////////////////////////////////////////
        // OgreModel/PhysicsModel shortcuts
        Ogre::Vector3 position() const;
        Ogre::Quaternion rotation() const;
        Ogre::Quaternion bodyRotation() const;
        Ogre::Vector3 bodyAngularVelocity() const;
        Ogre::Vector3 scale() const;
        Ogre::Vector3 velocity() const;
        float mass() const;

        /// Translates the agent by the given vector.
        void move(const Ogre::Vector3 &dpos);
        /// shortcut to PhysicsModel::applyImpulse
        void applyCentralImpulse(Ogre::Vector3 const &f);
        /// shortcut to PhysicsModel::applyCentralForce
        void applyCentralForce(Ogre::Vector3 const &f);
        void applyTorque(Ogre::Vector3 const &tq);
        void applyTorqueImpulse(Ogre::Vector3 const &tq);
        /// Rotate the graphic model by r.x in the x axis, etc. Does not impact the agent body rotation.
        void rotate(const Ogre::Vector3 &rot);
        /// Rotate the graphic model by the given quaternion. Does not impact the agent body rotation.
        void rotate(const Ogre::Quaternion &q);
        /// Rotate the agents body (physics model)
        void rotateBody(const Ogre::Quaternion &q);
        void setBodyRotation(const Ogre::Quaternion &q);
        Ogre::Quaternion bodyRotation();
        /// Rescale the agent by the given factor (current_scale*given_scale).
        void rescale(const Ogre::Vector3 &sca);

        void setPosition(const Ogre::Vector3 &pos);
        void setRotation(const Ogre::Quaternion &rot);
        void setScale(const Ogre::Vector3 &sca);

        //////////////////////////////////////////////////////////////////////
        // LocationModel shortcuts

        /// Shortcut to LocationModel()->setPath. Will attach to a new model if needed.
        bool setLocationPath(const Steel::LocationPathName &name);
        void unsetLocationPath();
        Ogre::String locationPath();
        bool hasLocationPath();

        //////////////////////////////////////////////////////////////////////
        // BTModel shortcuts
        Ogre::String BTPath();

        //////////////////////////////////////////////////////////////////////
        // BlackBoardModel shortcuts

        //////////////////////////////////////////////////////////////////////
        // tagging shortcuts
        void tag(Tag tag);
        void tag(std::set<Tag> tags);
        void untag(Steel::Tag tag);
        void untag(std::set<Tag> tags);
        std::set<Tag> tags() const;

        //////////////////////////////////////////////////////////////////////
        // signals
        /// Emitted signals
        enum class EventType : int
        {
            SELECTED = 1,
            UNSELECTED = 2,
        };
        /// Returns the signal for the given event
        Signal signal(EventType e);
    private:
        /// Emit with an auto lookup for the corresponding signal value.
        void emit(Agent::EventType e);
    public:
        

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

        /// Stack of behaviors. Current one is in mModelIds though.
        std::list<ModelId> mBehaviorsStack;

        typedef std::map<EventType, Signal> SignalMap;
        SignalMap mSignalMap;
    };

}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
