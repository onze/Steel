
#include "Debug.h"
#include "BT/BTStateStream.h"
#include "BT/BTSequence.h"
#include "BT/BTSelector.h"
// #include "BT/BTDecorator.h"
// #include "BT/BTCounter.h"
#include "BT/BTLocalizer.h"
#include "BT/BTNavigator.h"

namespace Steel
{
    BTStateStream::BTStateStream():
        mTokenTypes(std::vector<BTShapeTokenType>()),
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
        mTokenTypes.clear();
        mStateOffsets.clear();
        if(NULL!=mData)
            ::operator delete(mData);
        mData=NULL;
    }

    bool BTStateStream::empty()
    {
        return mTokenTypes.size()==0;
    }

    bool BTStateStream::init(BTShapeStream *shapeStream)
    {
        if(!empty())
            clear();

        // determine states indices and total size of the stream
        mDataSize=0;
        for(auto it=shapeStream->begin(); it!=shapeStream->end(); ++it)
        {
            BTShapeToken token=*it;
            size_t tokenSize=stateSize(token.type);
            if(tokenSize==0)
            {
                Debug::error("BTStateStream::init(): got size 0 for token type ")(token.type).endl();
                return false;
            }
            mTokenTypes.push_back(token.type);
            mStateOffsets.push_back(mDataSize);
            mDataSize+=tokenSize;

        }

        // allocate memory for states
        mData=::operator new(mDataSize);

        // initialize states
        for(size_t i=0; i<mTokenTypes.size(); ++i)
        {
            if(!placeStateAt(mStateOffsets[i],mTokenTypes[i]))
            {
                Debug::error("BTStateStream::init(): can't place state of type ")(mTokenTypes[i]);
                Debug::error(" at offset ")(mStateOffsets[i])("/")(mDataSize).endl();
                return false;
            }
        }
        return true;
    }

    bool BTStateStream::placeStateAt(size_t offset, BTShapeTokenType tokenType)
    {
        assert(offset<mDataSize);
        size_t base=(size_t) mData;
        switch(tokenType)
        {
                // placement new
                //http://www.parashift.com/c++-faq-lite/placement-new.html
            case BTSequenceToken:
                new((BTSequence *)(base+offset)) BTSequence();
                break;
            case BTSelectorToken:
                new((BTSelector *)(base+offset)) BTSelector();
                break;
            case BTLocalizerToken:
                new((BTLocalizer *)(base+offset)) BTLocalizer();
                break;
            case BTNavigatorToken:
                new((BTNavigator *)(base+offset)) BTNavigator();
                break;
            case BTUnknownToken:
            default:
                Ogre::String msg="BTStateStream::placeStateAt(): unknown BTShapeTokenType "+Ogre::StringConverter::toString(tokenType);
                Debug::error(msg).endl();
                return false;
        }
        return true;
    }

    size_t BTStateStream::stateSize(BTShapeTokenType tokenType)
    {
        switch(tokenType)
        {
            case BTSequenceToken:
                return sizeof(BTSequence);
            case BTSelectorToken:
                return sizeof(BTSelector);
            case BTLocalizerToken:
                return sizeof(BTLocalizer);
            case BTNavigatorToken:
                return sizeof(BTNavigator);
            case BTUnknownToken:
            default:
                Debug::error("BTStateStream::stateSize(): unknown BTShapeTokenType ")(tokenType).endl();
        }
        return 0;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
