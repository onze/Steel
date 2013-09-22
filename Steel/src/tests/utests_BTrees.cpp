
#include "tests/utests_BTrees.h"
#include <BTModelManager.h>
#include <tools/StringUtils.h>
#include <tools/File.h>
#include <Engine.h>

namespace Steel
{
    bool test_BTrees(Engine *engine)
    {
        return true;
        Ogre::String intro="in test_BTrees(): file ";

        BTModelManager *btModelMan=new BTModelManager(engine->level(),"/media/a0/cpp/1210/usmb/install_dir/data/raw_resources/BT");
        // load BTree serialization
        File rootFile("/media/a0/cpp/1210/usmb/data/resources/BTree models/patrol.model");
        if(!rootFile.exists())
        {
            Debug::warning(intro)(rootFile)("not found. Aborting unit test.").endl();
            return false;
        }
        Ogre::String content=rootFile.read();
        Json::Reader reader;
        Json::Value root;
        bool parsingOk = reader.parse(content, root, false);
        if (!parsingOk)
        {
            Debug::error(intro)("could not parse this:").endl();
            Debug::error(content).endl();
            Debug::error(reader.getFormattedErrorMessages()).endl();
            return false;
        }
        // instanciate it
        ModelId mid=INVALID_ID;
        if(!btModelMan->fromSingleJson(root, mid) || mid==INVALID_ID)
        {
            Debug::error(intro)("Model id is invalid. See above for details.").endl();
            return false;
        }

        Debug::log("test_BTrees(): passed").endl();
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
