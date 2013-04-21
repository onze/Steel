#include <algorithm>

#include "BT/BTFileNode.h"

namespace Steel
{

    BTFileNode::BTFileNode(const bool isGuard):File(),mIsGuard(isGuard)
    {

    }

    BTFileNode::BTFileNode():File()
    {
        mIsGuard=false;
    }

    BTFileNode::BTFileNode(BTFileNode const &o):File(o)
    {
        mIsGuard=o.mIsGuard;
    }

    BTFileNode::BTFileNode(const char *fullpath):File(fullpath)
    {
        mIsGuard=false;
    }

    BTFileNode::BTFileNode(Ogre::String fullpath):File(fullpath)
    {
        mIsGuard=false;
    }

    BTFileNode::~BTFileNode()
    {

    }

    BTFileNode &BTFileNode::operator=(BTFileNode const &o)
    {
        File::operator=(o);
        mIsGuard=o.mIsGuard;
        return *this;
    }

    BTShapeTokenType BTFileNode::shapeTokenType()
    {
        if(exists())
        {
            if(isFile())
            {
                for (BTShapeTokenType it = (BTShapeTokenType) ((int) _BTFirst + 1); it != _BTLast; it = (BTShapeTokenType) ((int) it + 1))
                    if(BTShapeTokenTypeAsString[it]==fileName())
                        return it;
            }
            else
            {
                for (BTShapeTokenType it = (BTShapeTokenType) ((int) _BTFirst + 1); it != _BTLast; it = (BTShapeTokenType) ((int) it + 1))
                {
                    Ogre::String BTShapeTokenTypeName=BTShapeTokenTypeAsString[it];
                    if(subfile(BTShapeTokenTypeName).exists())
                        return it;
                }
            }
        }
        return BTUnknownToken;
    }

    std::vector<BTFileNode> BTFileNode::childNodes()
    {
        std::vector<File> childNodes=ls(File::DIR);
        std::sort<std::vector<File>::iterator>(childNodes.begin(),childNodes.end());
        auto btChildNodes=std::vector<BTFileNode>(childNodes.begin(),childNodes.end());
        btChildNodes.push_back(BTFileNode(true));
        return btChildNodes;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
