
#include "Debug.h"
#include "BT/BTStateStream.h"
#include "BT/BTSequence.h"
#include "BT/BTSelector.h"
// #include "BT/BTDecorator.h"
// #include "BT/BTCounter.h"
#include "BT/BTFinder.h"
#include "BT/BTNavigator.h"
#include "BT/BTDebugPrinter.h"
#include "BT/BTSignalListener.h"

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
        return NULL==mShapeStream?true:mShapeStream->size()==0;
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
        static const Ogre::String intro= "in BTStateStream::builFromShapeStream(): ";
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
                Debug::error(intro)("got size 0 for token type ")(token.type)
                (" (")(BTShapeTokenTypeAsString[token.type])(")").endl();
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
    bool BTStateStream::placeStateAt(BTStateIndex _offset, BTShapeToken &token)
    {
        size_t offset=(size_t)_offset;
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
            case BTSignalListenerToken:
                node=new((BTSignalListener *)(base+offset)) BTSignalListener(token);
                break;
            case BTDebugPrinterToken:
                node=new((BTDebugPrinter *)(base+offset)) BTDebugPrinter(token);
                break;
            case _BTFirst:
            case _BTLast:
            case BTUnknownToken:
                Ogre::String msg="BTStateStream::placeStateAt(): unknown BTShapeTokenType ";
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
            // TOKEN                                      CLASS
            case BTSequenceToken:           return sizeof(BTSequence);
            case BTSelectorToken:           return sizeof(BTSelector);
            case BTFinderToken:             return sizeof(BTFinder);
            case BTNavigatorToken:          return sizeof(BTNavigator);
            case BTSignalListenerToken:     return sizeof(BTSignalListener);
            case BTDebugPrinterToken:       return sizeof(BTDebugPrinter);
            case _BTFirst:
            case _BTLast:
            case BTUnknownToken:
                Debug::error("BTStateStream::sizeOfState(): unknown BTShapeTokenType ")(tokenType)
                (" (")(BTShapeTokenTypeAsString[tokenType])(")").endl();
        }
        return 0;
    }

    BTNode* BTStateStream::stateAt(BTStateIndex index)
    {
        if(index>mShapeStream->size())
        {
            Debug::error("BTStateStream::stateAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return NULL;
        }
        size_t base=(size_t)mData;
        return (BTNode *)(base+mStateOffsets[(size_t)index]);
    }

    BTShapeTokenType BTStateStream::tokenTypeAt(BTStateIndex index)
    {
        if(index>=mShapeStream->size())
        {
            Debug::error("BTStateStream::tokenTypeAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return BTUnknownToken;
        }
        return mShapeStream->at((size_t)index).type;
    }
    
    BTShapeToken BTStateStream::tokenAt(BTStateIndex index)
    {
        if(index>=mShapeStream->size())
        {
            Debug::error("BTStateStream::tokenAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return {BTUnknownToken,0UL,0UL,""};
            //return BTShapeToken();
        }
        return mShapeStream->at((size_t)index);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
