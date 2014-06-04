#include "models/BlackBoardModel.h"
#include <tools/JsonUtils.h>
#include <SignalManager.h>

namespace Steel
{

    const char *BlackBoardModel::STRING_VARIABLES_ATTRIBUTE = "strings";

    BlackBoardModel::BlackBoardModel(): Model(),
        mVariables()
    {

    }

    BlackBoardModel::BlackBoardModel(const BlackBoardModel &o) : Model(o),
        mVariables(o.mVariables)
    {

    }

    BlackBoardModel::~BlackBoardModel()
    {

    }

    BlackBoardModel &BlackBoardModel::operator=(const BlackBoardModel &o)
    {
        Model::operator=(o);
        mVariables = o.mVariables;
        return *this;
    }

    bool BlackBoardModel::operator==(const BlackBoardModel &o) const
    {
        return Model::operator==(o);
    }

    bool BlackBoardModel::init(BlackBoardModelManager const *manager)
    {
        return true;
    }

    bool BlackBoardModel::fromJson(Json::Value const &root)
    {
        deserializeTags(root);
        mVariables = JsonUtils::asStringStringMap(root[BlackBoardModel::STRING_VARIABLES_ATTRIBUTE]);
        return true;
    }

    void BlackBoardModel::toJson(Json::Value &root)
    {
        serializeTags(root);

        if(mVariables.size())
            root[BlackBoardModel::STRING_VARIABLES_ATTRIBUTE] = JsonUtils::toJson(mVariables);
    }

    void BlackBoardModel::cleanup()
    {
        Model::cleanup();
    }

    void BlackBoardModel::setVariable(Ogre::String const &name, Ogre::String const &value)
    {
        bool isNew = mVariables.end() == mVariables.find(name);

        mVariables.erase(name);
        mVariables[name] = value;

        if(isNew)
            SignalManager::instance().emit(getSignal(PublicSignal::newVariable));
    }

    void BlackBoardModel::setVariable(Ogre::String const &name, AgentId const &value)
    {
        bool isNew = mVariables.end() == mVariables.find(name);

        mVariables.erase(name);
        mVariables[name] = Ogre::StringConverter::toString(value);

        if(isNew)
            SignalManager::instance().emit(getSignal(PublicSignal::newVariable));
    }

    Ogre::String BlackBoardModel::getStringVariable(Ogre::String const &name)
    {
        auto it = mVariables.find(name);

        if(mVariables.end() == it)
            return StringUtils::BLANK;

        return it->second;
    }

    AgentId BlackBoardModel::getAgentIdVariable(Ogre::String const &name)
    {
        auto it = mVariables.find(name);

        if(mVariables.end() == it)
            return INVALID_ID;

        return (AgentId) Ogre::StringConverter::parseUnsignedLong(it->second, INVALID_ID);
    }

    void BlackBoardModel::unsetVariable(Ogre::String const &name)
    {
        bool existed = mVariables.end() != mVariables.find(name);
        
        mVariables.erase(name);
        
        if(existed)
            SignalManager::instance().emit(getSignal(PublicSignal::variableDeleted));
    }

    Signal BlackBoardModel::getSignal(BlackBoardModel::PublicSignal signal) const
    {
#define STEEL_BLACKBOARDMODEL_GETSIGNAL_CASE(NAME) case NAME:return SignalManager::instance().toSignal("Steel::BlackBoardModel::"#NAME)

        switch(signal)
        {
                STEEL_BLACKBOARDMODEL_GETSIGNAL_CASE(PublicSignal::newVariable);
                STEEL_BLACKBOARDMODEL_GETSIGNAL_CASE(PublicSignal::variableDeleted);
        }

#undef STEEL_BLACKBOARDMODEL_GETSIGNAL_CASE
        return INVALID_SIGNAL;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
