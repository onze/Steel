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

    bool BTShapeManager::buildShapeStream(Ogre::String streamId,
                                          Steel::File &rootFile,
                                          BTShapeStream *&streamPtr)
    {
        BTShapeStreamMap::iterator it;
        // already built ?
        if(mStreamMap.end()!=(it=mStreamMap.find(streamId)))
        {
            streamPtr=&(*it).second;
            return true;
        }

        BTShapeStream stream;
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

            if(file.isGuard())
            {
                // popped back to root ?
                if(stack.size()==0)
                    break;
                stream[stack.top()].end=currentIndex;
                stack.pop();
                continue;
            }

            // push the token to the stream
            stream.push_back( {file.shapeTokenType(),currentIndex,0});
            
            // add childNodes to fringe for them to be processed next.
            std::vector<BTFileNode> childNodes=file.childNodes();
            fringe.insert(fringe.begin(),childNodes.begin(),childNodes.end());
            stack.push(currentIndex);
            ++currentIndex;
        }

        mStreamMap[streamId]=stream;
        streamPtr=&(mStreamMap[streamId]);
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
