#ifndef STEEL_AGENT_H_
#define STEEL_AGENT_H_

#include "steeltypes.h"
#include "SignalEmitter.h"
#include <tools/StringUtils.h>

namespace Steel
{

    class BlackBoardModel;
    class BTModel;
    class Level;
    class LocationModel;
    class Model;
    class OgreModel;
    class PhysicsModel;

    /**
     * Agent is the base class of Steel objects.
     *
     * Agents use composition of Model subclasses to achieve different behaviors. One can think of
     * an agent as an entry in a table, that only contains ids of models it is made of.
     */
    class Agent : public SignalEmitter
    {
    public:
        static const char *NAME_ATTRIBUTE;
        static const char *TAGS_ATTRIBUTE;
        static const char *ID_ATTRIBUTE;
        static const char *BEHAVIORS_STACK_ATTRIBUTE;

        Agent(AgentId id, Level *level);
        virtual ~Agent();
        Agent(const Agent &t);
        /// shallow copy
        Agent &operator=(const Agent &);

        static void staticInit();

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

        /// Returns true if the agent is linked to a model of the given type.
        bool hasModel(ModelType modelType) const;
        /// Returns an address to the model of the given type, if any. returns nullptr otherwise.
        Model *model(ModelType modelType) const;
        /// Return the id of the model of the given type, if any. returns Steel::INVALID_ID otherwise.
        ModelId modelId(ModelType modelType) const;

        /// Return all ids of all contained model types.
        std::map<ModelType, ModelId> &modelsIds() {return mModelIds;}

        inline OgreModel *ogreModel() const {return (OgreModel *) model(ModelType::OGRE);}
        inline ModelId ogreModelId() const {return modelId(ModelType::OGRE);}

        inline PhysicsModel *physicsModel() const {return (PhysicsModel *) model(ModelType::PHYSICS);}
        inline ModelId physicsModelId() const {return modelId(ModelType::PHYSICS);}

        inline LocationModel *locationModel() const {return (LocationModel *) model(ModelType::LOCATION);}
        inline ModelId locationModelId() const {return modelId(ModelType::LOCATION);}

        inline BTModel *btModel() const {return (BTModel *) model(ModelType::BT);}
        inline ModelId btModelId() const {return modelId(ModelType::BT);}

        inline BlackBoardModel *blackBoardModel() const {return (BlackBoardModel *) model(ModelType::BLACKBOARD);}
        inline ModelId blackBoardModelId() const {return modelId(ModelType::BLACKBOARD);}

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
        /// Translates the agent by the given vector.
        void move(const Ogre::Vector3 &dpos);
        void setPosition(const Ogre::Vector3 &pos);

        Ogre::Quaternion rotation() const;
        /// Rotate the graphic model by r.x in the x axis, etc.
        void rotate(const Ogre::Vector3 &rot);
        /// Rotate the graphic model by the given quaternion.
        void rotate(const Ogre::Quaternion &q);
        void setRotation(const Ogre::Quaternion &q);

        Ogre::Vector3 scale() const;
        /// Rescale the agent by the given factor (current_scale*given_scale).
        void rescale(const Ogre::Vector3 &sca);
        void setScale(const Ogre::Vector3 &sca);

        /// shortcut to PhysicsModel::angularVelocity
        Ogre::Vector3 angularVelocity() const;
        /// shortcut to PhysicsModel::velocity
        Ogre::Vector3 velocity() const;
        /// shortcut to PhysicsModel::mass
        float mass() const;

        /// shortcut to PhysicsModel::applyImpulse
        void applyCentralImpulse(Ogre::Vector3 const &f);
        /// shortcut to PhysicsModel::applyCentralForce
        void applyCentralForce(Ogre::Vector3 const &f);

        /// shortcut to PhysicsModel::applyTorque
        void applyTorque(Ogre::Vector3 const &tq);
        /// shortcut to PhysicsModel::applyTorqueImpulse
        void applyTorqueImpulse(Ogre::Vector3 const &tq);


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
        void untag(Tag tag);
        void untag(std::set<Tag> tags);
        std::set<Tag> tags() const;
        bool isTagged(Tag tag);

        //////////////////////////////////////////////////////////////////////
        // persistence
        /// A persistent agent is saved with the level it lives in.
        bool isPersistent();
        void setPersistent(bool flag);

        //////////////////////////////////////////////////////////////////////
        // handy
        inline Ogre::String const &name()const {return mName;}
        inline bool hasName()const {return StringUtils::BLANK != name();}
        inline void setName(Ogre::String const &name) {mName = name;}

        //////////////////////////////////////////////////////////////////////
        // signals
        /// Emitted signals TODO: use PublicSignal pattern instead
        enum class EventType : int
        {
            //TODO: implement PublicSignal pattern
            SELECTED = 1,
            UNSELECTED = 2,
        };
        /// Returns the signal for the given event
        Signal signal(EventType e);
    private:
        /// Emit with an auto lookup for the corresponding signal value.
        void emit(Agent::EventType e);

    private:
        /// Tags used internally, grouped here.
        struct PropertyTags
        {
            Tag persistent;
        };
        static PropertyTags sPropertyTags;

        /// Unique id.
        AgentId mId = INVALID_ID;
        Ogre::String mName = "";

        /// Ptr to the level the agent is in.
        Level *mLevel = nullptr;

        std::map<ModelType, ModelId> mModelIds;

        /// state flag
        bool mIsSelected = false;

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
