#ifndef STEEL_BLACKBOARDMODEL_H
#define STEEL_BLACKBOARDMODEL_H

#include <json/json.h>

#include "steeltypes.h"
#include "_ModelManager.h"
#include "Model.h"

namespace Steel
{
    class BlackBoardModelManager;
    class BlackBoardModel: public Model
    {

    public:
        BlackBoardModel();
        BlackBoardModel(const BlackBoardModel &o);
        virtual ~BlackBoardModel();
        virtual BlackBoardModel &operator=(const BlackBoardModel &o);
        virtual bool operator==(const BlackBoardModel &o) const;

        static ModelType modelType()
        {
            return MT_BLACKBOARD;
        }
        virtual bool init(BlackBoardModelManager const *manager);
        virtual bool fromJson(Json::Value &node);
        virtual void toJson(Json::Value &node);
        virtual void cleanup();
        
        void setVariable(Ogre::String const &name, Ogre::String const &value);
        void setVariable(Ogre::String const &name, AgentId const &value);
        void unsetVariable(Ogre::String const &name);
        Ogre::String getStringVariable(Ogre::String const &name);
        AgentId getAgentIdVariable(Ogre::String const &name);

    private:
        static const char *STRING_VARIABLES_ATTRIBUTE;
        
        /// TODO de/serialize me
        std::map<Ogre::String, Ogre::String> mVariables;
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
