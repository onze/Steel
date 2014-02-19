
#include "BTModelManager.h"
#include <Level.h>
#include <Agent.h>
#include <BlackBoardModelManager.h>
#include <Debug.h>

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

    ModelType BTModelManager::modelType()
    {
        return ModelType::BT;
    }

    Ogre::String BTModelManager::genericFollowPathModelPath()
    {
        return File(mBasePath).subfile(BTModelManager::GENERIC_FOLLOW_PATH_MODEL_NAME).fullPath();
    }

    bool BTModelManager::fromSingleJson(Json::Value &root, ModelId &id)
    {
        Json::Value value;
        Ogre::String intro = "in BTModelManager::fromSingleJson():\n" + root.toStyledString() + "\n";

        if(root.isNull())
        {
            Debug::error(intro)("Empty root. Aborting.").endl();
            return false;
        }

        value = root[BTModel::SHAPE_NAME_ATTRIBUTE];

        if(value.isNull())
        {
            Debug::error(intro)("unknown BTModel ")(BTModel::SHAPE_NAME_ATTRIBUTE)(" ").quotes(value)(". Aborting.").endl();
            return false;
        }

        Ogre::String rootPath = value.asString();
        File rootFile(mBasePath.subfile(rootPath));

        if(!rootFile.exists())
        {
            Debug::error(intro)("rootFile ")(rootFile)(" not found. Aborting.").endl();
            return false;
        }

        if(!buildFromFile(rootFile, id))
        {
            return false;
        }

        if(!mModels[id].fromJson(root))
        {
            return false;
        }

        // agentTags
        if(!mModels[id].deserializeTags(root))
        {
            Debug::error(intro)("could not deserialize tags. Aborting.").endl();
            return false;
        }

        return true;
    }

    bool BTModelManager::buildFromFile(File const &rootFile, ModelId &id)
    {
        Ogre::String intro = "in BTModelManager::buildFromFile(" + rootFile.fullPath() + "): ";

        // get the stream
        BTShapeStream *shapeStream = nullptr;

        if(!mBTShapeMan.buildShapeStream(rootFile.fileName(), rootFile, shapeStream))
        {
            Debug::error(intro)("could not generate a BT shape root node ")(rootFile);
            Debug::error(", see above for details. Aborting.").endl();
            return false;
        }

        assert(nullptr != shapeStream);

        // make it build the state stream
        id = allocateModel(id);

        if(INVALID_ID == id)
            return false;

        if(!mModels[id].init(this, shapeStream))
        {
            deallocateModel(id);
            id = INVALID_ID;
        }

        return true;
    }

    void BTModelManager::update(float timestep)
    {
        for(ModelId id = firstId(); id < lastId(); ++id)
        {
            BTModel *m = &(mModels[id]);

            if(m->isFree())
                continue;

            m->update(timestep);
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
