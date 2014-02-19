#ifndef STEEL_STEELTYPES_H_
#define STEEL_STEELTYPES_H_

#include <limits.h>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <functional>

#include <Ogre.h>

#include "BT/btnodetypes.h"

// http://stackoverflow.com/questions/1486904/how-do-i-best-silence-a-warning-about-unused-variables
#define STEEL_UNUSED(expr) do { (void)(expr); } while (0)

#define STEEL_BIT(N) 1<<N

// eases enum serialization
#define STEEL_ENUM_TO_STRING_START(ENUM_NAME) switch(e)
#define STEEL_ENUM_TO_STRING_CASE(ENUM_NAME) case ENUM_NAME: return #ENUM_NAME;

#define STEEL_STRING_TO_ENUM_CASE(ENUM_NAME) if(s == #ENUM_NAME) return ENUM_NAME;

// common forwards
namespace Json
{
    class Value;
}

namespace Steel
{
    typedef unsigned long AgentId;
    typedef unsigned long ModelId;
    typedef std::list<AgentId> Selection;
    typedef std::pair<ModelId, ModelId> ModelPair;
    
    //////////////////////////////////////////////////////////////
    // TIme related stuff go here
    /// Duration can be negative.
    typedef long int Duration;
    /// Basically what returns Ogre::Timer::get<whatevs>
    typedef long unsigned int TimeStamp;
    
    
    typedef unsigned int RefCount;

    /// invalid Model/Agent id.
    const unsigned long INVALID_ID = ULONG_MAX;

    /// usable enum values need to stay contiguous starting at 0.
    enum class ModelType : int
    {
        //ModelType::FIRST should stay first
        FIRST = -1,

        //put next ones here
        // ModelType::Ogre should be first of actual types, since this order is the loading order, and all models depend on OgreModels.
        OGRE,
        PHYSICS,
        LOCATION,
        BLACKBOARD,
        BT,

        //ModelType::LAST should stay last (to enable looping).
        LAST
    };
    
    Ogre::String toString(ModelType e);
    ModelType toModelType(Ogre::String s);

    typedef unsigned long Signal;
    const Signal INVALID_SIGNAL=ULONG_MAX;

    typedef unsigned long Tag;
    const Signal INVALID_TAG=ULONG_MAX;

    /// Shape of the physic enveloppe of an OgreModel
    enum BoundingShape
    {
        BS_BOX=0,
        BS_SPHERE,
        BS_CONVEXHULL,
        BS_TRIMESH,
    };
    
    /// Name of a path made of linked LocationModels.
    typedef Ogre::String LocationPathName;
    
    
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
