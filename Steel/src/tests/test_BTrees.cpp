
#include "tests/test_BTrees.h"
#include <BTModelManager.h>
#include <tools/StringUtils.h>
#include <tools/File.h>


namespace Steel
{
    void test_BTrees()
    {
        Ogre::String intro="in test_BTrees(): file ";
        BTModelManager *btMan=new BTModelManager(NULL,"/media/a0/cpp/1210/usmb/install_dir/data/raw_resources/BT");
        // load BTree serialization
        File rootFile("/media/a0/cpp/1210/usmb/data/resources/BTree models/patrol.model");
        if(!rootFile.exists())
        {
            Debug::warning(intro)(rootFile)("not found. Aborting unit test.").endl();
            return;
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
            return;
        }
        ModelId mid=btMan->fromSingleJson(root);
        assert(mid!=INVALID_ID);
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
