
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

    ModelId BTModelManager::fromSingleJson(Json::Value &root)
    {
        Json::Value value;
        Ogre::String intro="in BTModelManager::fromSingleJson():\n"+root.toStyledString()+"\n";
        if(root.isNull())
        {
            Debug::error(intro)("Empty root. Aborting.").endl();
            return INVALID_ID;
        }

        value=root["rootPath"];
        if(value.isNull())
        {
            Debug::error(intro)("unknown rootPath. Aborting.").endl();
            return INVALID_ID;
        }

        Ogre::String rootPath=value.asString();
        File rootFile(mBasePath.subfile(rootPath));
        if(!rootFile.exists())
        {
            Debug::error(intro)("rootFile ")(rootFile)(" not found. Aborting.").endl();
            return INVALID_ID;
        }

        BTShapeStream *shapeStream=NULL;
        if(!mBTShapeMan.buildShapeStream(rootFile.fileName(),rootFile,shapeStream))
        {
            Debug::error(intro)("could not generate a BT shape root node ")(rootFile)(", see above fo details. Aborting.").endl();
            return INVALID_ID;
        }
        assert(NULL!=shapeStream);

        //TODO: fix me !
        return INVALID_ID;
    }

} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
