#ifndef STEEL_BLACKBOARDMODEL_H
#define STEEL_BLACKBOARDMODEL_H

#include "steeltypes.h"
#include "Model.h"

namespace Steel
{
    class BlackBoardModelManager;

    class BlackBoardModel: public Model
    {
        DECLARE_STEEL_MODEL(BlackBoardModel, ModelType::BLACKBOARD);

    public:
        BlackBoardModel();
        BlackBoardModel(const BlackBoardModel &o);
        virtual ~BlackBoardModel();
        BlackBoardModel &operator=(const BlackBoardModel &o);
        bool operator==(const BlackBoardModel &o) const;

        bool init(BlackBoardModelManager const *manager);
        bool fromJson(Json::Value const &node);
        void toJson(Json::Value &node);
        void cleanup();

        void setVariable(Ogre::String const &name, Ogre::String const &value);
        void setVariable(Ogre::String const &name, AgentId const &value);
        void unsetVariable(Ogre::String const &name);
        Ogre::String getStringVariable(Ogre::String const &name);
        AgentId getAgentIdVariable(Ogre::String const &name);
        
        // const getters
        StringStringMap const &variables() const {return mVariables;}
        
        enum class PublicSignal : u32
        {
            /// Emitted when the a variable is set for the first time
            newVariable = 1,
            /// Emitted when the a variable is unset
            variableDeleted
        };
        Signal getSignal(BlackBoardModel::PublicSignal signal) const;

    private:
        static const char *STRING_VARIABLES_ATTRIBUTE;

        /// TODO de/serialize me
        StringStringMap mVariables;
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
