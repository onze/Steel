#include <algorithm>

#include "BT/BTFileNode.h"

namespace Steel
{

    BTFileNode::BTFileNode():File()
    {

    }

    BTFileNode::BTFileNode(const char *fullpath):File(fullpath)
    {

    }

    BTFileNode::BTFileNode(Ogre::String fullpath):File(fullpath)
    {

    }

    BTFileNode::~BTFileNode()
    {

    }

    BTShapeToken BTFileNode::shapeTokenType()
    {
        if(exists())
        {
            if(isFile())
            {
                for (BTShapeToken it = (BTShapeToken) ((int) _BTFirst + 1); it != _BTLast; it = (BTShapeToken) ((int) it + 1))
                    if(BTShapeTokenAsString[it]==fileName())
                        return it;
            }
            else
            {
                for (BTShapeToken it = (BTShapeToken) ((int) _BTFirst + 1); it != _BTLast; it = (BTShapeToken) ((int) it + 1))
                {
                    Ogre::String BTShapeTokenName=BTShapeTokenAsString[it];
                    if(subfile(BTShapeTokenName).exists())
                        return it;
                }
            }
        }
        return BTUnknown;
    }

    std::vector<BTFileNode> BTFileNode::childNodes()
    {
        std::vector<File> childNodes=ls(File::DIR);
        std::sort<std::vector<File>::iterator>(childNodes.begin(),childNodes.end());
        return std::vector<BTFileNode>(childNodes.begin(),childNodes.end());
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
