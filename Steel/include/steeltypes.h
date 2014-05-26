#ifndef STEEL_STEELTYPES_H_
#define STEEL_STEELTYPES_H_

#include <limits.h>
#include <list>
#include <map>
#include <set>
#include <vector>
#include <functional>

#include <Ogre.h>

#ifndef MYGUI_DONT_REPLACE_NULLPTR
#define MYGUI_DONT_REPLACE_NULLPTR
#endif

#include "BT/btnodetypes.h"

// http://stackoverflow.com/questions/1486904/how-do-i-best-silence-a-warning-about-unused-variables
#define STEEL_UNUSED(EXPR) do { (void)(EXPR); } while (0)

#define STEEL_DELETE(MYPTR) \
if(nullptr != MYPTR) \
{ \
    delete MYPTR; \
    MYPTR = nullptr; \
}

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
    typedef unsigned int u32;
    typedef unsigned long u64;
    typedef signed int s32;
    typedef signed long s64;
    typedef float f32;
    typedef double f64;

    typedef u64 AgentId;
    typedef u64 ModelId;
    /// invalid Model/Agent id.
    const u64 INVALID_ID = ULONG_MAX;
    typedef std::list<AgentId> Selection;
    typedef std::pair<ModelId, ModelId> ModelPair;
    
    typedef std::vector<Ogre::String> StringVector;

    //////////////////////////////////////////////////////////////
    // TIme related stuff go here
    /// Duration can be negative.
    typedef long int Duration;
    extern Duration DURATION_MAX;
    extern Duration DURATION_MIN;
    /// Basically what returns Ogre::Timer::get<whatevs>
    typedef long unsigned int TimeStamp;

    typedef u32 RefCount;


    /// usable enum values need to stay contiguous starting at 0.
    enum class ModelType : s32
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

    typedef u64 Signal;
    const Signal INVALID_SIGNAL = ULONG_MAX;

    typedef u64 Tag;
    const Signal INVALID_TAG = ULONG_MAX;

    /// Shape of the physic enveloppe of an OgreModel
    enum class BoundingShape
    {
        BOX = 0,
        SPHERE,
        CONVEXHULL,
        TRIMESH,
    };
    Ogre::String toString(BoundingShape e);
    BoundingShape toBoundingShape(Ogre::String s);

    /// Name of a path made of linked LocationModels.
    typedef Ogre::String LocationPathName;

    typedef size_t Hash;
    
    typedef Ogre::String PropertyGridPropertyId;

    class Engine;
    class Level;
    class Model;
    class OgreModel;
    class OgreModelManager;
    class PhysicsModel;
    class PhysicsModelManager;
    class LocationModel;
    class LocationModelManager;
    class BTModel;
    class BTModelManager;
    class BlackBoardModel;
    class BlackBoardModelManager;
    class Agent;
    class AgentManager;
    class SelectionManager;
    class Camera;
    class TagManager;
    class SignalListener;
    class SignalEmitter;
    class SignalManager;
    class PropertyGrid;
    class PropertyGridAdapter;
    class PropertyGridAgentAdapter;
    //class PropertyGridModelAdapter;
    class PropertyGridProperty;
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
