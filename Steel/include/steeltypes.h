#ifndef STEEL_STEELTYPES_H_
#define STEEL_STEELTYPES_H_

#include <limits.h>
#include <vector>
#include <list>

#include <OgreString.h>

#include "BT/btnodetypes.h"

namespace Steel
{

    typedef unsigned long AgentId;
    typedef unsigned long ModelId;
    typedef std::list<AgentId> Selection;

    /// invalid Model/Agent id.
    const unsigned long INVALID_ID = ULONG_MAX;

    /// ModelType and modelTypesAsString need to stay in sync, and the usable enum values need to stay contiguous starting at 0.
    enum ModelType
    {
        //MT_FIRST should stay first
        MT_FIRST = -1,

        //put next ones here
        // MT_Ogre should be first of actual types, since this order is the loading order, and all models depend on OgreModels.
        MT_OGRE,
        MT_PHYSICS,
        MT_BT,

        //MT_LAST should stay last (to enable looping).
        MT_LAST
    };

    /// Maps a ModelType to its string representation.
    extern std::vector<Ogre::String> modelTypesAsString;

    typedef unsigned long Signal;
    const Signal INVALID_SIGNAL=ULONG_MAX;
    
    /// Shape of the physic enveloppe of an OgreModel
    enum BoundingShape
    {
        BS_BOX=0,
        BS_SPHERE,
        BS_CONVEXHULL, 
        BS_TRIMESH,
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
