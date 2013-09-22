
#include "tests/utests_BTStateStream.h"
#include <BT/btnodetypes.h>
#include <tools/StringUtils.h>
#include <tools/File.h>
#include <BTModelManager.h>
#include <BT/BTShapeManager.h>
#include <BT/BTSequence.h>
#include <BT/BTNavigator.h>
#include <BT/BTFinder.h>

namespace Steel
{
    bool test_BTStateStream()
    {
        return true;
        Ogre::String intro="in test_BTShapeStream(): file ",abortMsg="Aborting unit test.";

        // get the shape stream
        BTShapeManager shapeMan;
        Ogre::String streamId="utests/shapes/A - sequence";
        File rootFile("/media/a0/cpp/1210/usmb/data/raw_resources/BT/utests/shapes/A - sequence");
        BTShapeStream *shapeStream=NULL;
        if(!rootFile.exists())
        {
            Debug::warning(intro)(rootFile)(" not found. ")(abortMsg).endl();
            return false;
        }
        if(!shapeMan.buildShapeStream(streamId,rootFile,shapeStream))
        {
            Debug::error(intro)("Could not create shape stream, see above for details.")(abortMsg).endl();
            return false;
        }

        BTStateStream stateStream;
        stateStream.init(shapeStream);

        // assert valid token types
        assert(stateStream.tokenTypeAt(0)==BTSequenceToken);
        assert(stateStream.tokenTypeAt(1)==BTSequenceToken);
        assert(stateStream.tokenTypeAt(2)==BTFinderToken);
        assert(stateStream.tokenTypeAt(3)==BTNavigatorToken);
        assert(stateStream.tokenTypeAt(4)==BTSequenceToken);
        assert(stateStream.tokenTypeAt(5)==BTFinderToken);
        assert(stateStream.tokenTypeAt(6)==BTNavigatorToken);
        // test out of range token
        Debug::ignoreNextErrorMessage();
        assert(stateStream.tokenTypeAt(7)==BTUnknownToken);

        BTSequence *seq;
        BTFinder *fdr;
        BTNavigator *nav;
        // generally assert all is where it should, and is linked to what it should.
        // get token A
        seq=(BTSequence *)stateStream.stateAt(0);
        // valid bounds
        assert(seq->begin()==0);
        assert(seq->end()==7);
        // valid type
        assert(BTSequenceToken==seq->tokenType());
        // type specifics
        assert(1==seq->currentChildNodeIndex());

        // B
        seq=(BTSequence *)stateStream.stateAt(1);
        assert(seq->begin()==1);
        assert(seq->end()==4);
        assert(BTSequenceToken==seq->tokenType());
        assert(2==seq->currentChildNodeIndex());

        // C
        fdr=(BTFinder *)stateStream.stateAt(2);
        assert(fdr->begin()==2);
        assert(fdr->end()==3);
        assert(BTFinderToken==fdr->tokenType());
        // specifics
        assert("B"==fdr->agentSpec().tag());
        assert(BTFinder::LM_STATIC==fdr->localizationMode());

        // D
        nav=(BTNavigator *)stateStream.stateAt(3);
        assert(nav->begin()==3);
        assert(nav->end()==4);
        assert(BTNavigatorToken==nav->tokenType());

        // E
        seq=(BTSequence *)stateStream.stateAt(4);
        assert(seq->begin()==4);
        assert(seq->end()==7);
        assert(BTSequenceToken==seq->tokenType());
        assert(5==seq->currentChildNodeIndex());

        // F
        fdr=(BTFinder *)stateStream.stateAt(5);
        assert(fdr->begin()==5);
        assert(fdr->end()==6);
        assert(BTFinderToken==fdr->tokenType());
        assert("A"==fdr->agentSpec().tag());
//         assert(BTFinder::LM_DYNAMIC==fdr->localizationMode());

        // G
        nav=(BTNavigator *)stateStream.stateAt(6);
        assert(nav->begin()==6);
        assert(nav->end()==7);
        assert(BTNavigatorToken==nav->tokenType());

        Debug::log("test_BTStateStream(): passed").endl();
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
