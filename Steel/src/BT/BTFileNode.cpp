#include <algorithm>

#include "BT/BTFileNode.h"
#include <Debug.h>

namespace Steel
{

    BTFileNode::BTFileNode(const bool isGuard): File(), mIsGuard(isGuard)
    {

    }

    BTFileNode::BTFileNode(): File()
    {
        mIsGuard = false;
    }

    BTFileNode::BTFileNode(BTFileNode const &o): File(o)
    {
        mIsGuard = o.mIsGuard;
    }

    BTFileNode::BTFileNode(const char *fullpath): File(fullpath)
    {
        mIsGuard = false;
    }

    BTFileNode::BTFileNode(Ogre::String fullpath): File(fullpath)
    {
        mIsGuard = false;
    }

    BTFileNode::BTFileNode(const File &file): File(file)
    {
        mIsGuard = false;
    }

    BTFileNode::~BTFileNode()
    {

    }

    BTFileNode &BTFileNode::operator=(BTFileNode const &o)
    {
        File::operator=(o);
        mIsGuard = o.mIsGuard;
        return *this;
    }

    File BTFileNode::descriptor()
    {
        if(exists())
        {
            if(isFile())
            {
                return *this;
            }
            else
            {
                File sub;

                for(BTShapeTokenType it = (BTShapeTokenType)((int) BTShapeTokenType::_BTFirst + 1);
                        it != BTShapeTokenType::_BTLast;
                        it = (BTShapeTokenType)((int) it + 1))
                {
                    Ogre::String BTShapeTokenTypeName = BTShapeTokenTypeAsString[(int)it];
                    sub = subfile(BTShapeTokenTypeName);

                    if(sub.exists())
                        return sub;
                }
            }
        }

        return subfile(BTShapeTokenTypeAsString[(int)BTShapeTokenType::BTUnknownToken]);
    }

    BTShapeTokenType BTFileNode::shapeTokenType()
    {
        File desc = descriptor();

        if(desc.exists())
        {
            for(auto it = (BTShapeTokenType)((int) BTShapeTokenType::_BTFirst + 1); it != BTShapeTokenType::_BTLast; it = (BTShapeTokenType)((int) it + 1))
                if(toString(it) == desc.fileName())
                    return it;
        }

        Debug::warning(STEEL_METH_INTRO, "unknown token type for file ", *this, " with descriptor ", desc.fullPath(), ":").endl()
        (desc.read()).endl();
        return BTShapeTokenType::BTUnknownToken;
    }

    std::vector<BTFileNode> BTFileNode::childNodes()
    {
        std::vector<File> childNodes = ls(File::DIR);
        std::sort<std::vector<File>::iterator>(childNodes.begin(), childNodes.end());
        auto btChildNodes = std::vector<BTFileNode>(childNodes.begin(), childNodes.end());
        btChildNodes.push_back(BTFileNode(true));
        return btChildNodes;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
