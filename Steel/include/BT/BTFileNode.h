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
            BTFileNode();
            BTFileNode(const char *fullpath);
            BTFileNode(Ogre::String fullpath);
            virtual ~BTFileNode();

            /**
             * Returns a shape token indicating the type of node the file is pointing to. If
             * the file is points to a directory, a lookup for a direct BTShapeToken subfile is done.
             */
            BTShapeToken shapeTokenType();

            /// Returns a vector of all children of the file, in the order they should be processed.
            std::vector<BTFileNode> childNodes();
    };

}

#endif // STEEL_BTFILENODE_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
