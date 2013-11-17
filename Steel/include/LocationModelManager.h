#ifndef STEEL_LOCATIONMODELMANAGER_H
#define STEEL_LOCATIONMODELMANAGER_H

#include <json/json.h>
#include <vector>
#include <map>

#include "steeltypes.h"

#include "_ModelManager.h"
#include "LocationModel.h"

namespace Steel
{
    class DynamicLines;
    class Level;
    class LocationModelManager: public _ModelManager<LocationModel>
    {
        typedef _ModelManager<LocationModel> super;
        /// Holds paths roots
        static const char *PATH_ROOTS_ATTRIBUTE;
    public:
        LocationModelManager(Level *level);
        virtual ~LocationModelManager();

        /// modelType associated with this Manager
        virtual inline ModelType modelType()
        {
            return MT_LOCATION;
        }

        std::vector<ModelId> fromJson(Json::Value &model);
        bool fromSingleJson(Json::Value &model, ModelId &mid);
        void toJson(Json::Value &object);
        bool onAgentLinkedToModel(Agent *agent, ModelId mid);
        void onAgentUnlinkedFromModel(Agent *agent, ModelId mid);

        /////////////////////////////////////////////////
        // Specific to model linking
        bool linkAgents(AgentId srcAgentId, AgentId dstAgentId);
        /// Link 2 locations together.
        bool linkLocations(ModelId srcId, ModelId dstId);
        /// Unlinks 2 agents' locations iff they were linked to each other. Return true if they were.
        bool unlinkAgents(AgentId srcAgentId, AgentId dstAgentId);
        /// Unlinks 2 locations iff they were linked to each other. Return true if they were.
        bool unlinkLocations(ModelId mid0, ModelId mid1);
        /// Unlinks a location from its sources and destinations.
        void unlinkLocation(ModelId mid);

        /////////////////////////////////////////////////
        // Location path manipulations
        /// Set the path name of a model and all others models linked to it. Name cannot be the empty string.
        void setModelPath(ModelId mid, LocationPathName const &name);
        /// Removes all location models from the path, and removes this path.
        void unsetModelPath(ModelId mid);
        /// Returns true if the given model is part of a path.
        bool hasModelPath(ModelId mid);
        /**
         * Sets a root agent for a given path. A root is automatically set when a path is assigned
         * to a location. If force is true, overwrite any existing previous value.
         */
        void setPathRoot(AgentId aid, bool force = false);
        /// Internals. Removes the path root. Does not check if other models beong to this path.
        void _unsetPathRoot();
        /// Returns the root of the path, as an AgentId.
        AgentId pathRoot(LocationPathName const &name);

        /////////////////////////////////////////////////
        // misc
        void moveLocation(ModelId mid, Ogre::Vector3 const &pos);
        /// Updates the debug lines of the given model.
        void updateDebugLines(ModelId mid);
        /// Updates the debug line between 2 models, if one exists.
        void updateDebugLine(ModelPair const &key);

    private:
        /// The pair content is (src, dst)
        ModelPair makeKey(ModelId mid0, ModelId mid1);
        bool getDebugLine(ModelPair const &key, DynamicLines *&line);
        void removeDebugLines(ModelId mid);
        void removeDebugLine(ModelPair const &key);
        /// Returns all debug lines keys involving the given model id.
        std::list<ModelPair> collectModelPairs(ModelId mid);
        // owned
        std::map<ModelPair, DynamicLines *> mDebugLines;

        /// keys if a path name, value is the id of the agent attached to the root LocationModel.
        std::map<LocationPathName, AgentId> mPathsRoots;


    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
