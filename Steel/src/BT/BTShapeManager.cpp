#include <list>
#include <stack>

#include "BT/BTShapeManager.h"
#include <BT/BTFileNode.h>
#include <Debug.h>

namespace Steel
{

    BTShapeManager::BTShapeManager():
        mStreamMap(BTShapeStreamMap())
    {
    }

    BTShapeManager::BTShapeManager(const BTShapeManager &other)
    {
    }

    BTShapeManager::~BTShapeManager()
    {
    }

    BTShapeManager &BTShapeManager::operator=(const BTShapeManager &other)
    {
        return *this;
    }

    bool BTShapeManager::operator==(const BTShapeManager &other) const
    {
        return false;
    }

    BTShapeStream *BTShapeManager::getBTShapeStream(Ogre::String streamName)
    {
        // already built ?
        auto it = mStreamMap.find(streamName);

        if(mStreamMap.end() != it)
        {
            return &(*it).second;
        }

        return nullptr;
    }

    bool BTShapeManager::buildShapeStream(Ogre::String streamName,
                                          File const &rootFile,
                                          BTShapeStream *&streamPtr)
    {
        BTShapeStreamMap::iterator it;

        // already built ?
        if(nullptr != (streamPtr = getBTShapeStream(streamName)))
            return true;

        BTShapeStream stream;
        stream.mName = streamName;
        // walk the file hierarchy depth first, using the fringe as buffer.
        std::list<BTFileNode> fringe;
        fringe.push_front(BTFileNode(true));
        fringe.push_front(BTFileNode(rootFile));
        unsigned int currentIndex = 0;
        std::stack<unsigned int> stack;

        while(fringe.size())
        {
            BTFileNode file = fringe.front();
            fringe.pop_front();

            if(isIgnored(file))
                continue;

            if(file.isGuard())
            {
                // popped back to root ?
                if(stack.size() == 0)
                    break;

                stream.mData[stack.top()].end = currentIndex;
                stack.pop();
                continue;
            }

            // push the token to the stream
            stream.mData.push_back( {file.shapeTokenType(), currentIndex, 0, file.descriptor().fullPath()});

            // add childNodes to fringe for them to be processed next.
            std::vector<BTFileNode> childNodes = file.childNodes();
            fringe.insert(fringe.begin(), childNodes.begin(), childNodes.end());
            stack.push(currentIndex);
            ++currentIndex;
        }

        mStreamMap[streamName] = stream;
        streamPtr = &(mStreamMap[streamName]);
        return true;
    }

    bool BTShapeManager::isIgnored(const BTFileNode &file)
    {
        return Ogre::StringUtil::startsWith(file.fileName(), "__");
    }

    void BTShapeManager::clearCachedStreams()
    {
        mStreamMap.clear();
    }

    //////////////////////////////////////////////////////////////////////////////////////////
    // UNIT TESTS
    bool utest_BTShapeStream(UnitTestExecutionContext const *context)
    {
        Ogre::String intro = "test_BTShapeStream(): ";

        BTShapeManager shapeMan;
        Ogre::String streamId = "utests/shapes/A - sequence";
        File rootFile("/media/a0/cpp/1210/usmb/data/raw_resources/BT/utests/shapes/A - sequence");
        BTShapeStream *stream = nullptr;

        if(!rootFile.exists())
        {
            Debug::warning(intro)(rootFile)(" not found. ").endl();
            return false;
        }

        if(!shapeMan.buildShapeStream(streamId, rootFile, stream))
        {
            Debug::error(intro)("Could not create shape stream, see above for details.").endl();
            return false;
        }

        BTShapeToken groundTruth_data[] =
        {
            {BTShapeTokenType::BTSequenceToken, 0, 7, BTFileNode(rootFile).descriptor()},                                               //A
            {BTShapeTokenType::BTSequenceToken, 1, 4, BTFileNode(rootFile.subfile("B - sequence")).descriptor()},                       //+-B
            {BTShapeTokenType::BTFinderToken, 2, 3, BTFileNode(rootFile.subfile("B - sequence").subfile("C - finder")).descriptor()},   //| +-C
            {BTShapeTokenType::BTNavigatorToken, 3, 4, BTFileNode(rootFile.subfile("B - sequence").subfile("D - navigator")).descriptor()}, //| |-D
            {BTShapeTokenType::BTSequenceToken, 4, 7, BTFileNode(rootFile.subfile("E - sequence")).descriptor()},                       //+-E
            {BTShapeTokenType::BTFinderToken, 5, 6, BTFileNode(rootFile.subfile("E - sequence").subfile("F - finder")).descriptor()},   //  +-F
            {BTShapeTokenType::BTNavigatorToken, 6, 7, BTFileNode(rootFile.subfile("E - sequence").subfile("G - navigator")).descriptor()}, //  |-G
        };
        BTShapeStream groundTruth;
        groundTruth.mName = streamId;
        groundTruth.mData = BTShapeStreamData(groundTruth_data,
                                              groundTruth_data + sizeof(groundTruth_data) / sizeof(BTShapeToken));
        //         BTShapeStream groundTruth;
        //         groundTruth.push_back({BTSequenceToken,0,7});
        std::pair<BTShapeStreamData::iterator, BTShapeStreamData::iterator> mismatch;

        // using default comparison
        mismatch = std::mismatch(stream->mData.begin(), stream->mData.end(), groundTruth.mData.begin());

        if(mismatch.first != stream->mData.end())
        {
            Debug::error(intro)("BTShapeStream mismatch with groundTruth:").endl();
            Debug::error("streamId: ")(streamId).endl();
            Debug::error("rootFile: ")(rootFile).endl();
            Debug::error("stream built: ")(stream).endl();
            Debug::error("groundTruth: ")(&groundTruth).endl();
            Debug::error("first mismatch: ").endl().indent();
            Debug::error("stream: ")(*mismatch.first).endl();
            Debug::error("groundTruth: ")(*mismatch.second).unIndent().endl();
            return false;
        }

        shapeMan.clearCachedStreams();
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
