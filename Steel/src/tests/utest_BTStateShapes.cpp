
#include <algorithm>    // std::mismatch
#include <utility>      // std::pair

#include <OgreString.h>

#include "tests/utest_BTStateShapes.h"

#include <BT/BTShapeManager.h>
#include <Debug.h>

namespace Steel
{
    /**
     * Builds a shapeStream directly from the shapeStream
     */
    bool test_BTStateShapes()
    {
        Ogre::String intro="test_BTStateShapes(): ",abortMsg="Aborting unit test.";

        BTShapeManager shapeMan;
        Ogre::String streamId="utests/shapes/A - sequence";
        File rootFile("/media/a0/cpp/1210/usmb/data/raw_resources/BT/utests/shapes/A - sequence");
        BTShapeStream *stream=NULL;
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
            {BTSequenceToken,0,7},//A
            {BTSequenceToken,1,4},//+-B
            {BTLocalizerToken,2,3},//| +-C
            {BTNavigatorToken,3,4},//| |-D
            {BTSequenceToken,4,7},//+-E
            {BTLocalizerToken,5,6},//  +-F
//             {BTNavigatorToken,6,7},//  |-G
        };

        BTShapeStream groundTruth = BTShapeStream (
                                        groundTruth_data,
                                        groundTruth_data+sizeof(groundTruth_data)/sizeof(BTShapeToken)
                                    );
//         BTShapeStream groundTruth;
//         groundTruth.push_back({BTSequenceToken,0,7});
        std::pair<BTShapeStream::iterator,BTShapeStream::iterator> mismatch;

        // using default comparison:
        mismatch = std::mismatch (stream->begin(), stream->end(), groundTruth.begin());

        if(mismatch.first!=stream->end())
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

        Debug::log("test_BTStateShapes(): passed").endl();
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
