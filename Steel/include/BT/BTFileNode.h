#ifndef STEEL_BTFILENODE_H
#define STEEL_BTFILENODE_H

#include "BT/btnodetypes.h"
#include "tools/File.h"

namespace Steel
{
/**
 * BTFileNode is a File subclass that offers small useful commodities like:
 * - folder auto ordering when walking through child nodes
 * - easy access to descriptor file
 */
class BTFileNode: public File
{
public:
    BTFileNode(const bool isGuard = false);
    BTFileNode();
    BTFileNode(BTFileNode const &o);
    BTFileNode(const char *fullpath);
    BTFileNode(Ogre::String fullpath);
    BTFileNode(const File &file);
    virtual ~BTFileNode();
    BTFileNode &operator=(BTFileNode const &o);

    /**
     * Returns a shape token indicating the type of node the file is pointing to. If
     * the file is points to a directory, a lookup for a direct BTShapeToken subfile is done.
     */
    BTShapeTokenType shapeTokenType();

    /// Returns the node's descriptor as a BTFileNode.
    File descriptor();

    /// Returns a vector of all children of the file, in the order they should be processed.
    std::vector<BTFileNode> childNodes();

    // getters
    inline bool isGuard()
    {
        return mIsGuard;
    }

protected:
    /**
     * False, unless set at construction time. A guard Node is the last (or only) children of a node, so that
     * childNodes never returns an emtpy vector.
     * Note: guards are only used during construction of the BTShapeStream, not at execution time.
     */
    bool mIsGuard;
};

}

#endif // STEEL_BTFILENODE_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
