
#include "BTModelManager.h"

namespace Steel
{

    BTModelManager::BTModelManager(Level *level,Ogre::String path) :
        _ModelManager<BTModel>(level),
        mBTShapeMan(),mBasePath(path)
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
        return MT_BT;
    }

    bool BTModelManager::fromSingleJson(Json::Value &root, ModelId &id)
    {
        id=INVALID_ID;
        Json::Value value;
        Ogre::String intro="in BTModelManager::fromSingleJson():\n"+root.toStyledString()+"\n";
        if(root.isNull())
        {
            Debug::error(intro)("Empty root. Aborting.").endl();
            return false;
        }

        value=root["rootPath"];
        if(value.isNull())
        {
            Debug::error(intro)("unknown rootPath. Aborting.").endl();
            return false;
        }

        Ogre::String rootPath=value.asString();
        File rootFile(mBasePath.subfile(rootPath));
        if(!rootFile.exists())
        {
            Debug::error(intro)("rootFile ")(rootFile)(" not found. Aborting.").endl();
            return false;
        }

        return buildFromFile(rootFile, id);
        return true;
    }

    bool BTModelManager::buildFromFile(File &rootFile, ModelId &id)
    {
        Ogre::String intro="in BTModelManager::buildFromFile("+rootFile.fullPath()+"): ";

        // get the stream
        BTShapeStream *shapeStream=NULL;
        if(!mBTShapeMan.buildShapeStream(rootFile.fileName(),rootFile,shapeStream))
        {
            Debug::error(intro)("could not generate a BT shape root node ")(rootFile);
            Debug::error(", see above for details. Aborting.").endl();
            return false;
        }
        assert(NULL!=shapeStream);

        // make it build the state stream
        id=allocateModel();
        if(!mModels[id].init(shapeStream))
        {
            Debug::error(intro)("could not init model. Deallocating it.").endl();
            deallocateModel(id);
            id=INVALID_ID;
        }
        return true;
    }

    void BTModelManager::update(float timestep)
    {
        for (ModelId id = firstId(); id < lastId(); ++id)
        {
            BTModel *m=&(mModels[id]);
            if(m->isFree())
                continue;
            m->update(timestep);
        }
    }

} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
