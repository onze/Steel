#ifndef STEEL_BTSHAPEMANAGER_H
#define STEEL_BTSHAPEMANAGER_H

#include <map>
#include <vector>
#include "OgreString.h"

#include "BT/btnodetypes.h"
#include "tools/File.h"

namespace Steel
{
    /**
     * Handles Behavior Trees shapes.
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
             * succeeded. Otherwise, returns false.
             */
            bool buildShapeStream(Ogre::String streamId, Steel::File& rootFile, Steel::BTShapeStream*& stream);
            
    protected:
        // not owned
        
        // owned
        BTShapeStreamMap mStreamMap;
    };

}

#endif // STEEL_BTSHAPEMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
