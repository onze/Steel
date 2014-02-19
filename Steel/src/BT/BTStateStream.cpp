
#include "Debug.h"
#include "BT/BTStateStream.h"
#include "BT/BTSequence.h"
#include "BT/BTSelector.h"
// #include "BT/BTDecorator.h"
// #include "BT/BTCounter.h"
#include "BT/BTFinder.h"
#include "BT/BTNavigator.h"
#include "BT/BTDebug.h"
#include "BT/BTSignalListener.h"
#include "BT/BTShapeManager.h"
#include <tools/File.h>

namespace Steel
{
    BTStateStream::BTStateStream():
        mShapeStream(nullptr),
        mStateOffsets(),
        mDataSize(0), mData(nullptr)
    {

    }

    BTStateStream::BTStateStream(BTStateStream const &o):
        mShapeStream(nullptr),
        mStateOffsets(),
        mDataSize(0), mData(nullptr)
    {
        // mem copy fails to copy valid pointers to functions, used in strategy bind in some nodes (nevgator, finder, etc)
        if(nullptr != o.mShapeStream)
            buildFromShapeStream(o.mShapeStream);
    }

    BTStateStream::~BTStateStream()
    {
        clear();
    }

    BTStateStream &BTStateStream::operator=(BTStateStream const &o)
    {
        clear();
        buildFromShapeStream(o.mShapeStream);
        return *this;
    }

    bool BTStateStream::empty()
    {
        return nullptr == mShapeStream ? true : mShapeStream->mData.size() == 0;
    }

    Ogre::String BTStateStream::debugName()
    {
        return "<BTStateStream @" + Ogre::StringConverter::toString(reinterpret_cast<long>(this)) + ">";
    }

    bool BTStateStream::init(BTShapeStream *shapeStream)
    {
        return buildFromShapeStream(shapeStream);
    }

    bool BTStateStream::buildFromShapeStream(BTShapeStream *shapeStream)
    {
        static const Ogre::String intro = "in BTStateStream::buildFromShapeStream(): ";

        if(!empty())
            clear();

        if(nullptr == shapeStream)
        {
            Debug::warning(intro)("trying to build from nullptr shapeStream. Aborting.").endl();
            return false;
        }

        mShapeStream = shapeStream;
        mStateOffsets.clear();
        // determine states indices and total size of the stream
        mDataSize = 0;

        for(auto it = shapeStream->mData.begin(); it != shapeStream->mData.end(); ++it)
        {
            BTShapeToken token = *it;
            size_t tokenSize = sizeOfState(token.type);

            if(tokenSize == 0)
            {
                Debug::error(intro)("got size 0 for token ")(token).endl();
                return false;
            }

            mStateOffsets.push_back(mDataSize);
            mDataSize += tokenSize;
        }

        // allocate memory for states
        mData =::operator new(mDataSize);

        // initialize states
        for(size_t i = 0; i < mShapeStream->mData.size(); ++i)
        {
            BTShapeToken token = mShapeStream->mData.at(i);

            if(!placeStateAt(mStateOffsets[i], token))
            {
                Debug::error(intro)("can't place state of type ")(token.type)
                ("(")(BTShapeTokenTypeAsString[token.type])(") at offset ")(mStateOffsets[i])("/")(mDataSize).endl();
                return false;
            }
        }

        return true;
    }

    // offset from the beginning of the memory allocated for states
    bool BTStateStream::placeStateAt(BTStateOffset _offset, BTShapeToken &token)
    {
        size_t offset = (size_t)_offset;
        assert(offset < mDataSize);
        size_t base = (size_t) mData;
        BTNode *node = nullptr;

        // placement new http://www.parashift.com/c++-faq-lite/placement-new.html
        switch(token.type)
        {
            case BTSequenceToken:
                node = new((BTSequence *)(base + offset)) BTSequence(token);
                break;

            case BTSelectorToken:
                node = new((BTSelector *)(base + offset)) BTSelector(token);
                break;

            case BTFinderToken:
                node = new((BTFinder *)(base + offset)) BTFinder(token);
                break;

            case BTNavigatorToken:
                node = new((BTNavigator *)(base + offset)) BTNavigator(token);
                break;

            case BTSignalListenerToken:
                node = new((BTSignalListener *)(base + offset)) BTSignalListener(token);
                break;

            case BTDebugToken:
                node = new((BTDebug *)(base + offset)) BTDebug(token);
                break;

            case _BTFirst:
            case _BTLast:
            case BTUnknownToken:
                Ogre::String msg = "BTStateStream::placeStateAt(): unknown BTShapeTokenType ";
                Debug::error(msg)(Ogre::StringConverter::toString(token.type)).endl();
                return false;
        }

        node->reset();
        return true;
    }

    size_t BTStateStream::sizeOfState(BTShapeTokenType tokenType)
    {
        switch(tokenType)
        {
            case BTSequenceToken:
                return sizeof(BTSequence);

            case BTSelectorToken:
                return sizeof(BTSelector);

            case BTFinderToken:
                return sizeof(BTFinder);

            case BTNavigatorToken:
                return sizeof(BTNavigator);

            case BTSignalListenerToken:
                return sizeof(BTSignalListener);

            case BTDebugToken:
                return sizeof(BTDebug);

            case _BTFirst:
            case _BTLast:
            case BTUnknownToken:
                Debug::error("BTStateStream::sizeOfState(): unknown BTShapeTokenType ")(tokenType)
                (" (")(BTShapeTokenTypeAsString[tokenType])(")").endl();
        }

        return 0;
    }

    void BTStateStream::clear()
    {
        for(BTStateIndex i = 0; i < mStateOffsets.size(); ++i)
        {
            BTShapeToken token = tokenAt(i);
            size_t base = (size_t) mData;
            BTStateOffset offset = mStateOffsets.at(i);

            switch(token.type)
            {
                case BTSequenceToken:
                    ((BTSequence *)(base + offset))->~BTSequence();
                    break;

                case BTSelectorToken:
                    ((BTSelector *)(base + offset))->~BTSelector();
                    break;

                case BTFinderToken:
                    ((BTFinder *)(base + offset))->~BTFinder();
                    break;

                case BTNavigatorToken:
                    ((BTNavigator *)(base + offset))->~BTNavigator();
                    break;

                case BTSignalListenerToken:
                    ((BTSignalListener *)(base + offset))->~BTSignalListener();
                    break;

                case BTDebugToken:
                    ((BTDebug *)(base + offset))->~BTDebug();
                    break;

                case _BTFirst:
                case _BTLast:
                case BTUnknownToken:
                    Debug::error("BTStateStream::clear(): unknown BTShapeTokenType for token: ")(token).endl();
                    break;
            }
        }

        mStateOffsets.clear();
        ::operator delete(mData);
        mData = nullptr;
        mShapeStream = nullptr;
    }

    BTNode *BTStateStream::stateAt(BTStateIndex index)
    {
        if(index > mShapeStream->mData.size())
        {
            Debug::error("BTStateStream::stateAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return nullptr;
        }

        size_t base = (size_t)mData;
        return (BTNode *)(base + mStateOffsets[(size_t)index]);
    }

    BTShapeTokenType BTStateStream::tokenTypeAt(BTStateIndex index)
    {
        if(index >= mShapeStream->mData.size())
        {
            Debug::error("BTStateStream::tokenTypeAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return BTUnknownToken;
        }

        return mShapeStream->mData.at((size_t)index).type;
    }

    BTShapeToken BTStateStream::tokenAt(BTStateIndex index)
    {
        if(nullptr == mShapeStream)
        {
            Debug::error("BTStateStream::tokenAt(")(index)("): ")(debugName())(" no valid shapestream.").endl();
            return {BTUnknownToken, 0UL, 0UL, Ogre::StringUtil::BLANK};
        }

        if(index >= mShapeStream->mData.size())
        {
            Debug::error("BTStateStream::tokenAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return {BTUnknownToken, 0UL, 0UL, Ogre::StringUtil::BLANK};
        }

        return mShapeStream->mData.at((size_t)index);
    }
    
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // UNIT TESTS
    bool utest_BTStateStream(UnitTestExecutionContext const* context)
    {
        return true;
        Ogre::String intro = "in test_BTShapeStream(): file ", abortMsg = "Aborting unit test.";
        
        // get the shape stream
        BTShapeManager shapeMan;
        Ogre::String streamId = "utests/shapes/A - sequence";
        File rootFile("/media/a0/cpp/1210/usmb/data/raw_resources/BT/utests/shapes/A - sequence");
        BTShapeStream *shapeStream = nullptr;
        
        if(!rootFile.exists())
        {
            Debug::warning(intro)(rootFile)(" not found. ")(abortMsg).endl();
            return false;
        }
        
        if(!shapeMan.buildShapeStream(streamId, rootFile, shapeStream))
        {
            Debug::error(intro)("Could not create shape stream, see above for details.")(abortMsg).endl();
            return false;
        }
        
        BTStateStream stateStream;
        stateStream.init(shapeStream);
        
        // assert valid token types
        assert(stateStream.tokenTypeAt(0) == BTSequenceToken);
        assert(stateStream.tokenTypeAt(1) == BTSequenceToken);
        assert(stateStream.tokenTypeAt(2) == BTFinderToken);
        assert(stateStream.tokenTypeAt(3) == BTNavigatorToken);
        assert(stateStream.tokenTypeAt(4) == BTSequenceToken);
        assert(stateStream.tokenTypeAt(5) == BTFinderToken);
        assert(stateStream.tokenTypeAt(6) == BTNavigatorToken);
        // test out of range token
        Debug::ignoreNextErrorMessage();
        assert(stateStream.tokenTypeAt(7) == BTUnknownToken);
        
        BTSequence *seq;
        BTFinder *fdr;
        BTNavigator *nav;
        // generally assert all is where it should, and is linked to what it should.
        // get token A
        seq = (BTSequence *)stateStream.stateAt(0);
        // valid bounds
        assert(seq->begin() == 0);
        assert(seq->end() == 7);
        // valid type
        assert(BTSequenceToken == seq->tokenType());
        // type specifics
        assert(1 == seq->currentChildNodeIndex());
        
        // B
        seq = (BTSequence *)stateStream.stateAt(1);
        assert(seq->begin() == 1);
        assert(seq->end() == 4);
        assert(BTSequenceToken == seq->tokenType());
        assert(2 == seq->currentChildNodeIndex());
        
        // C
        fdr = (BTFinder *)stateStream.stateAt(2);
        assert(fdr->begin() == 2);
        assert(fdr->end() == 3);
        assert(BTFinderToken == fdr->tokenType());
        // specifics
        
        // D
        nav = (BTNavigator *)stateStream.stateAt(3);
        assert(nav->begin() == 3);
        assert(nav->end() == 4);
        assert(BTNavigatorToken == nav->tokenType());
        
        // E
        seq = (BTSequence *)stateStream.stateAt(4);
        assert(seq->begin() == 4);
        assert(seq->end() == 7);
        assert(BTSequenceToken == seq->tokenType());
        assert(5 == seq->currentChildNodeIndex());
        
        // F
        fdr = (BTFinder *)stateStream.stateAt(5);
        assert(fdr->begin() == 5);
        assert(fdr->end() == 6);
        assert(BTFinderToken == fdr->tokenType());
        
        // G
        nav = (BTNavigator *)stateStream.stateAt(6);
        assert(nav->begin() == 6);
        assert(nav->end() == 7);
        assert(BTNavigatorToken == nav->tokenType());
        
        Debug::log("test_BTStateStream(): passed").endl();
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
