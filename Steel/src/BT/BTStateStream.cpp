
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
#include <BT/BTShapeManager.h>

namespace Steel
{
    BTStateStream::BTStateStream():
        mShapeStream(NULL),
        mStateOffsets(std::vector<size_t>()),
        mData(NULL),mDataSize(0)
    {

    }

    BTStateStream::BTStateStream(const BTStateStream& o)
    {
        this->operator=(o);
    }

    BTStateStream::~BTStateStream()
    {
        clear();
    }

    BTStateStream& BTStateStream::operator=(const BTStateStream& o)
    {
        clear();
        buildFromShapeStream(o.mShapeStream);
        return *this;
    }

    bool BTStateStream::empty()
    {
        return NULL==mShapeStream?true:mShapeStream->mData.size()==0;
    }

    Ogre::String BTStateStream::debugName()
    {
        return "<BTStateStream "+Ogre::StringConverter::toString(this)+">";
    }

    bool BTStateStream::init(BTShapeStream *shapeStream)
    {
        return buildFromShapeStream(shapeStream);
    }

    bool BTStateStream::buildFromShapeStream(BTShapeStream *shapeStream)
    {
        static const Ogre::String intro= "in BTStateStream::buildFromShapeStream(): ";
        if(!empty())
            clear();
        
        if(NULL == shapeStream)
        {
            Debug::warning(intro)("trying to build from NULL shapeStream. Aborting.").endl();
            return false;
        }
        
        mShapeStream=shapeStream;
        // determine states indices and total size of the stream
        mDataSize=0;
        for(auto it=shapeStream->mData.begin(); it!=shapeStream->mData.end(); ++it)
        {
            BTShapeToken token=*it;
            size_t tokenSize=sizeOfState(token.type);
            if(tokenSize==0)
            {
                Debug::error(intro)("got size 0 for token ")(token).endl();
                return false;
            }
            mStateOffsets.push_back(mDataSize);
            mDataSize+=tokenSize;
        }

        // allocate memory for states
        mData=::operator new(mDataSize);

        // initialize states
        for(size_t i=0; i<mShapeStream->mData.size(); ++i)
        {
            BTShapeToken token=mShapeStream->mData.at(i);
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
            case BTDebugToken:
                node=new((BTDebug *)(base+offset)) BTDebug(token);
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
        for(BTStateIndex i=0; i<mStateOffsets.size(); ++i)
        {
            BTShapeToken token=tokenAt(i);
            size_t base=(size_t) mData;
            BTStateOffset offset=mStateOffsets.at(i);
            switch(token.type)
            {
                case BTSequenceToken:
                    ((BTSequence *)(base+offset))->~BTSequence();
                    break;
                case BTSelectorToken:
                    ((BTSelector *)(base+offset))->~BTSelector();
                    break;
                case BTFinderToken:
                    ((BTFinder *)(base+offset))->~BTFinder();
                    break;
                case BTNavigatorToken:
                    ((BTNavigator *)(base+offset))->~BTNavigator();
                    break;
                case BTSignalListenerToken:
                    ((BTSignalListener *)(base+offset))->~BTSignalListener();
                    break;
                case BTDebugToken:
                    ((BTDebug *)(base+offset))->~BTDebug();
                    break;
                case _BTFirst:
                case _BTLast:
                case BTUnknownToken:
                    Debug::error("BTStateStream::clear(): unknown BTShapeTokenType for token: ")(token).endl();
                    break;
            }
        }
        mStateOffsets.clear();
        if(NULL!=mData)
            ::operator delete(mData);
        mData=NULL;
        mShapeStream=NULL;
    }

    BTNode* BTStateStream::stateAt(BTStateIndex index)
    {
        if(index>mShapeStream->mData.size())
        {
            Debug::error("BTStateStream::stateAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return NULL;
        }
        size_t base=(size_t)mData;
        return (BTNode *)(base+mStateOffsets[(size_t)index]);
    }

    BTShapeTokenType BTStateStream::tokenTypeAt(BTStateIndex index)
    {
        if(index>=mShapeStream->mData.size())
        {
            Debug::error("BTStateStream::tokenTypeAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return BTUnknownToken;
        }
        return mShapeStream->mData.at((size_t)index).type;
    }

    BTShapeToken BTStateStream::tokenAt(BTStateIndex index)
    {
        if(index>=mShapeStream->mData.size())
        {
            Debug::error("BTStateStream::tokenAt(")(index)("): ")(debugName())(" index out of range.").endl();
            return {BTUnknownToken,0UL,0UL,""};
            //return BTShapeToken();
        }
        return mShapeStream->mData.at((size_t)index);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
