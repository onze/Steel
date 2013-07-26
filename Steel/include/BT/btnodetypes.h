#ifndef STEEL_BTNODETYPES_H_
#define STEEL_BTNODETYPES_H_

#include <map>
#include <OgreString.h>

namespace Steel
{

/// tells the type of processing a node does.
enum BTShapeTokenType
{
    // Should stay first enum value
    _BTFirst = -1,

    BTSequenceToken,
    BTSelectorToken,
    BTDecoratorToken,
    BTCounterToken,
    BTFinderToken,
    BTNavigatorToken,
    BTUnknownToken,

    // Should stay last value
    _BTLast
};

/// Holds shape information
class BTShapeToken
{
public:
    BTShapeTokenType type;
    /// node index of the token
    unsigned begin;
    /// node index of the next token (last child's +1)
    unsigned end;
    /// path to file with parameters
    Ogre::String contentFile;

    bool operator==(BTShapeToken const &o) const
    {
        return type == o.type && begin == o.begin && end == o.end && contentFile == o.contentFile;
    }
};

/** Flatten BTree (the tree shape). This is the abstract part of a tree,
 * that's shared amongst all models using this shape.*/
typedef std::vector<BTShapeToken> BTShapeStream;

/// Shapes storage structure
typedef std::map<Ogre::String, BTShapeStream> BTShapeStreamMap;

/// Allows for slow lookup
extern std::vector<Ogre::String> BTShapeTokenTypeAsString;

/// BTNodes state values.
enum BTState
{
    FAILURE = 1 << 3, READY = 1 << 0, RUNNING = 1 << 1, SUCCESS = 1 << 2,
};
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

#endif
