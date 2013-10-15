#ifndef STEEL_BTSHAPEMANAGER_H
#define STEEL_BTSHAPEMANAGER_H

#include <map>
#include <vector>
#include "OgreString.h"

#include "BT/btnodetypes.h"
#include "BTFileNode.h"
#include "tools/File.h"

namespace Steel
{
    /**
     * Handles behavior tree shape streams, that is BTShapeStream.
     * Could be a manager on its own.
     */
    class BTShapeManager
    {

        public:

            BTShapeManager();
            BTShapeManager(const BTShapeManager& o);
            virtual ~BTShapeManager();
            virtual BTShapeManager& operator=(const BTShapeManager& o);
            virtual bool operator==(const BTShapeManager& o) const;

            /**
             * Builds a shape stream out of a file system, saves it under the given streamId,
             * sets the given stream pointer to the stream address, and returns true if all operations
             * succeeded.
             * @param streamId the name of the root folder of the tree, from the base BT resources folder path.
             * @param rootFile File pointing to the rot folder of the tree.
             * @param stream pointer to the created shape stream.
             * @return true if no error occured.
             */
            bool buildShapeStream(Ogre::String streamName, Steel::File& rootFile, Steel::BTShapeStream*& stream);

            /// Returns a pointer to a cached stream, or nullptr if no stream with the given name has been built.
            BTShapeStream *getBTShapeStream(Ogre::String streamName);

            /// Removes all streams from the map
            void clearCachedStreams();

            /// Returns true if the file matches the ignored files pattern (name starts with a double underscore).
            bool isIgnored(const BTFileNode &file);

        protected:
            // not owned

            // owned
            /// map streamIds to shapeStreams
            BTShapeStreamMap mStreamMap;
    };

}

#endif // STEEL_BTSHAPEMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
