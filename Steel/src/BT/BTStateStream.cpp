
#include "Debug.h"
#include "BT/BTStateStream.h"
#include "BT/BTSequence.h"
#include "BT/BTSelector.h"
// #include "BT/BTDecorator.h"
// #include "BT/BTCounter.h"
#include "BT/BTFinder.h"
#include "BT/BTNavigator.h"

namespace Steel
{
    BTStateStream::BTStateStream():
        mShapeStream(NULL),
        mStateOffsets(std::vector<size_t>()),
        mData(NULL),mDataSize(0)
    {

    }

    BTStateStream::BTStateStream(const BTStateStream& other)
    {

    }

    BTStateStream::~BTStateStream()
    {
        clear();
    }

    BTStateStream& BTStateStream::operator=(const BTStateStream& other)
    {
        return *this;
    }

    bool BTStateStream::operator==(const BTStateStream& other) const
    {
        return false;
    }

    void BTStateStream::clear()
    {
        mStateOffsets.clear();
        if(NULL!=mData)
            ::operator delete(mData);
        mData=NULL;
        mShapeStream=NULL;
    }

    bool BTStateStream::empty()
    {
        return NULL==mShapeStream?0:mShapeStream->size()==0;
    }

    Ogre::String BTStateStream::debugName()
    {
        return "<BTStateStream "+Ogre::StringConverter::toString(this)+">";
    }

    bool BTStateStream::init(BTShapeStream *shapeStream)
    {
        return builFromShapeStream(shapeStream);
    }

    bool BTStateStream::builFromShapeStream(BTShapeStream *shapeStream)
    {
        if(!empty())
            clear();
        mShapeStream=shapeStream;
        // determine states indices and total size of the stream
        mDataSize=0;
        for(auto it=shapeStream->begin(); it!=shapeStream->end(); ++it)
        {
            BTShapeToken token=*it;
            size_t tokenSize=sizeOfState(token.type);
            if(tokenSize==0)
            {
                Debug::error("BTStateStream::init(): got size 0 for token type ")(token.type).endl();
                return false;
            }
            mStateOffsets.push_back(mDataSize);
            mDataSize+=tokenSize;
        }

        // allocate memory for states
        mData=::operator new(mDataSize);

        // initialize states
        for(size_t i=0; i<mShapeStream->size(); ++i)
        {
            BTShapeToken token=mShapeStream->at(i);
            if(!placeStateAt(mStateOffsets[i],token))
            {
                Debug::error("BTStateStream::init(): can't place state of type ")(token.type);
                Debug::error(" at offset ")(mStateOffsets[i])("/")(mDataSize).endl();
                return false;
            }
        }
        return true;
    }

    // offset from the beginning of the memory allocated for states
    bool BTStateStream::placeStateAt(size_t offset, BTShapeToken &token)
    {
        assert(offset<mDataSize);
        size_t base=(size_t) mData;
        BTNode *node=NULL;
        // placement new http://www.parashift.com/c++-faq-lite/placement-new.html
        switch(token.type)
        {
            case BTSequenceToken:
                node=new((BTSequence *)(base+offset)) BTSequence(token);
                break;
            case BTSelectorToken:
                node=new((BTSelector *)(base+offset)) BTSelector(token);
                break;
            case BTFinderToken:
                node=new((BTFinder *)(base+offset)) BTFinder(token);
                break;
            case BTNavigatorToken:
                node=new((BTNavigator *)(base+offset)) BTNavigator(token);
                break;
            case BTUnknownToken:
            default:
                Ogre::String msg="BTStateStream::placeStateAt(): unknown BTShapeTokenType ";
                Debug::error(msg)(Ogre::StringConverter::toString(token.type)).endl();
                return false;
        }
        node->reset();
        return true;
    }

    void *BTStateStream::stateAt(size_t index)
    {
        if(index>mShapeStream->size())
        {
            Debug::error("BTStateStream::stateAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return NULL;
        }
        size_t base=(size_t)mData;
        return (void *)(base+mStateOffsets[index]);
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
            case BTUnknownToken:
            default:
                Debug::error("BTStateStream::stateSize(): unknown BTShapeTokenType ")(tokenType).endl();
        }
        return 0;
    }

    BTShapeTokenType BTStateStream::tokenTypeAt(size_t index)
    {
        if(index>=mShapeStream->size())
        {
            Debug::error("BTStateStream::tokenTypeAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return BTUnknownToken;
        }
        return mShapeStream->at(index).type;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
