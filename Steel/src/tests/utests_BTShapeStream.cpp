
#include <algorithm>    // std::mismatch
#include <utility>      // std::pair

#include <OgreString.h>

#include "tests/utests_BTShapeStream.h"

#include <BT/btnodetypes.h>
#include <Debug.h>
#include <BT/BTShapeManager.h>
#include <BT/BTFileNode.h>
#include <Engine.h>

namespace Steel
{
    /**
     * Builds a shapeStream directly from the shapeStream.
     */
    bool test_BTShapeStream()
    {
        Ogre::String intro="test_BTShapeStream(): ",abortMsg="Aborting unit test.";

        BTShapeManager shapeMan;
        Ogre::String streamId="utests/shapes/A - sequence";
        File rootFile("/media/a0/cpp/1210/usmb/data/raw_resources/BT/utests/shapes/A - sequence");
        BTShapeStream *stream=nullptr;
        if(!rootFile.exists())
        {
            Debug::warning(intro)(rootFile)(" not found. ")(abortMsg).endl();
            return false;
        }
        if(!shapeMan.buildShapeStream(streamId,rootFile,stream))
        {
            Debug::error(intro)("Could not create shape stream, see above for details.")(abortMsg).endl();
            return false;
        }

        BTShapeToken groundTruth_data[] =
        {
            {BTSequenceToken,0,7,BTFileNode(rootFile).descriptor()},                                                  //A
            {BTSequenceToken,1,4,BTFileNode(rootFile.subfile("B - sequence")).descriptor()},                          //+-B
            {BTFinderToken,2,3,BTFileNode(rootFile.subfile("B - sequence").subfile("C - finder")).descriptor()},      //| +-C
            {BTNavigatorToken,3,4,BTFileNode(rootFile.subfile("B - sequence").subfile("D - navigator")).descriptor()},//| |-D
            {BTSequenceToken,4,7,BTFileNode(rootFile.subfile("E - sequence")).descriptor()},                          //+-E
            {BTFinderToken,5,6,BTFileNode(rootFile.subfile("E - sequence").subfile("F - finder")).descriptor()},      //  +-F
            {BTNavigatorToken,6,7,BTFileNode(rootFile.subfile("E - sequence").subfile("G - navigator")).descriptor()},//  |-G
        };
        BTShapeStream groundTruth;
        groundTruth.mName = streamId;
        groundTruth.mData = BTShapeStreamData(groundTruth_data,
                                              groundTruth_data+sizeof(groundTruth_data)/sizeof(BTShapeToken));
//         BTShapeStream groundTruth;
//         groundTruth.push_back({BTSequenceToken,0,7});
        std::pair<BTShapeStreamData::iterator,BTShapeStreamData::iterator> mismatch;

        // using default comparison:
        mismatch = std::mismatch (stream->mData.begin(), stream->mData.end(), groundTruth.mData.begin());

        if(mismatch.first!=stream->mData.end())
        {
            Debug::error(intro)("BTShapeStream mismatch with groundTruth:").endl();
            Debug::error("streamId: ")(streamId).endl();
            Debug::error("rootFile: ")(rootFile).endl();
            Debug::error("stream built: ")(stream).endl();
            Debug::error("groundTruth: ")(&groundTruth).endl();
            Debug::error("first mismatch: ").endl().indent();
            Debug::error("stream: ")(*mismatch.first).endl();
            Debug::error("groundTruth: ")(*mismatch.second).unIndent().endl();
            Debug::error(abortMsg).endl();
            return false;
        }

        shapeMan.clearCachedStreams();
        Debug::log("test_BTStateShapes(): passed").endl();
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
