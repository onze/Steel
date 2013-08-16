#include <list>
#include <stack>

#include "BT/BTShapeManager.h"
#include <BT/BTFileNode.h>

namespace Steel
{

    BTShapeManager::BTShapeManager():
        mStreamMap(BTShapeStreamMap())
    {

    }

    BTShapeManager::BTShapeManager(const BTShapeManager& other)
    {

    }

    BTShapeManager::~BTShapeManager()
    {

    }

    BTShapeManager& BTShapeManager::operator=(const BTShapeManager& other)
    {
        return *this;
    }

    bool BTShapeManager::operator==(const BTShapeManager& other) const
    {
        return false;
    }

    BTShapeStream *BTShapeManager::getBTShapeStream(Ogre::String streamName)
    {
        // already built ?
        auto it=mStreamMap.find(streamName);
        if(mStreamMap.end()!=it)
        {
            return &(*it).second;
        }
        return NULL;
    }

    bool BTShapeManager::buildShapeStream(Ogre::String streamName,
                                          Steel::File &rootFile,
                                          BTShapeStream *&streamPtr)
    {
        BTShapeStreamMap::iterator it;
        // already built ?
        if(NULL != (streamPtr=getBTShapeStream(streamName)))
            return true;

        BTShapeStream stream;
        stream.mName=streamName;
        // walk the file hierarchy depth first, using the fringe as buffer.
        std::list<BTFileNode> fringe;
        fringe.push_front(BTFileNode(true));
        fringe.push_front(BTFileNode(rootFile));
        unsigned int currentIndex=0;
        std::stack<unsigned int> stack;

        while(fringe.size())
        {
            BTFileNode file=fringe.front();
            fringe.pop_front();

            if(isIgnored(file))
                continue;

            if(file.isGuard())
            {
                // popped back to root ?
                if(stack.size()==0)
                    break;
                stream.mData[stack.top()].end=currentIndex;
                stack.pop();
                continue;
            }

            // push the token to the stream
            stream.mData.push_back( {file.shapeTokenType(),currentIndex,0,file.descriptor().fullPath()});

            // add childNodes to fringe for them to be processed next.
            std::vector<BTFileNode> childNodes=file.childNodes();
            fringe.insert(fringe.begin(),childNodes.begin(),childNodes.end());
            stack.push(currentIndex);
            ++currentIndex;
        }

        mStreamMap[streamName]=stream;
        streamPtr=&(mStreamMap[streamName]);
        return true;
    }

    bool BTShapeManager::isIgnored(const BTFileNode &file)
    {
        return Ogre::StringUtil::startsWith(file.fileName(),"__");
    }

    void BTShapeManager::clearCachedStreams()
    {
        mStreamMap.clear();
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
