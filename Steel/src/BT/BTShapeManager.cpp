#include <list>

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

    bool BTShapeManager::buildShapeStream(Ogre::String streamId, Steel::File &rootFile, BTShapeStream *&streamPtr)
    {
        BTShapeStream stream;
        // walk the file hierarchy depth first, using the fringe as buffer.
        std::list<BTFileNode> fringe;
        fringe.push_front(BTFileNode(rootFile));
        while(fringe.size())
        {
            BTFileNode file=fringe.front();
            fringe.pop_front();

            // push the type to the stream
            stream.push_back(file.shapeTokenType());

            // add childNodes to fringe for them to be processed next.
            std::vector<BTFileNode> childNodes=file.childNodes();
            if(childNodes.size()>0)
                fringe.insert(fringe.begin(),childNodes.begin(),childNodes.end());
        }

        mStreamMap[streamId]=stream;
        streamPtr=&(mStreamMap[streamId]);
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
