
#include "Debug.h"
#include "Level.h"
#include "SignalManager.h"
#include "models/Agent.h"
#include "models/BlackBoardModelManager.h"
#include "models/BTModelManager.h"

namespace Steel
{
    const Ogre::String BTModelManager::GENERIC_FOLLOW_PATH_MODEL_NAME = "followPath";

    BTModelManager::BTModelManager(Level *level, Ogre::String path) :
        _ModelManager<BTModel>(level),
        mBTShapeMan(), mBasePath(path)
    {
        if(!mBasePath.exists())
        {
            Debug::warning("BTModelManager raw resources path ")(mBasePath);
            Debug::warning(" not found. This run is not likely to end well...").endl();
        }
    }

    BTModelManager::~BTModelManager()
    {
        // TODO Auto-generated destructor stub
    }

    Ogre::String BTModelManager::genericFollowPathModelPath()
    {
        return File(mBasePath).subfile(BTModelManager::GENERIC_FOLLOW_PATH_MODEL_NAME).fullPath();
    }
    
    bool BTModelManager::deserializeToModel(Json::Value const&root, ModelId &mid)
    {
        Debug::error(STEEL_METH_INTRO, "invalid code path.").endl().breakHere();
        return false;
    }

    bool BTModelManager::fromSingleJson(Json::Value const&root, ModelId &id, bool updateModel/* = false*/)
    {
        Json::Value value;

        if(root.isNull())
        {
            Debug::error(STEEL_METH_INTRO, "Empty root. Aborting.").endl();
            return false;
        }

        value = root[BTModel::SHAPE_NAME_ATTRIBUTE];

        if(value.isNull())
        {
            Debug::error(STEEL_METH_INTRO, "unknown BTModel attribute").quotes(BTModel::SHAPE_NAME_ATTRIBUTE)(" with value ").quotes(value)(". Aborting.").endl();
            return false;
        }

        Ogre::String rootPath = value.asString();
        File rootFile(mBasePath.subfile(rootPath));

        if(!rootFile.exists())
        {
            Debug::error(STEEL_METH_INTRO, "rootFile ", rootFile, " not found. Aborting.").endl();
            return false;
        }

        if(!buildFromFile(rootFile, id, updateModel))
            return false;

        if(!mModels[id].fromJson(root))
            return false;

        // agentTags
        if(!mModels[id].deserializeTags(root))
        {
            Debug::error(STEEL_METH_INTRO, "could not deserialize tags. Aborting.").endl();
            return false;
        }

        return true;
    }

    bool BTModelManager::buildFromFile(File const &rootFile, ModelId &mid, bool updateModel)
    {
        // get the stream
        BTShapeStream *shapeStream = nullptr;

        if(!mBTShapeMan.buildShapeStream(rootFile.fileName(), rootFile, shapeStream))
        {
            Debug::error(STEEL_METH_INTRO, "could not generate a BT shape root node ", rootFile, ", see above for details. Aborting.").endl();
            return false;
        }

        assert(nullptr != shapeStream);

        // make it build the state stream
        ModelId id = mid;
        id = allocateModel(id);

        if(INVALID_ID == id && !updateModel)
            return false;

        if(!mModels[id].init(this, shapeStream))
        {
            deallocateModel(id);
            mid = INVALID_ID;
            return false;
        }

        mid = id;
        return true;
    }

    void BTModelManager::update(float timestep)
    {
        SignalManager::instance().fireEmittedSignals();

        for(BTModel & model : mModels)
        {
            if(model.isFree())
                continue;

            model.update(timestep);
        }
    }

    bool BTModelManager::onAgentLinkedToModel(Agent *agent, ModelId mid)
    {
        BTModel *model = at(mid);

        if(nullptr == model)
            return false;

        model->setOwnerAgent(agent->id());

        if(INVALID_ID != agent->blackBoardModelId())
            model->setBlackboardModelId(agent->blackBoardModelId());

        return true;
    }

} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
