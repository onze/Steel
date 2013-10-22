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
    class LocationModelManager:public _ModelManager<LocationModel>
    {
        public:
            LocationModelManager(Level *level);
            virtual ~LocationModelManager();

            /// modelType associated with this Manager
            virtual inline ModelType modelType()
            {
                return MT_LOCATION;
            }
            
            /// Creates a new model and returns its id.
            ModelId newModel();

            /// Same treatment as parent class, plus some postprocessing.
            std::vector<ModelId> fromJson(Json::Value &models);
            bool fromSingleJson(Json::Value &model, ModelId &mid);
            
            bool linkAgents(AgentId srcAgentId, AgentId dstAgentId);
            /// Link 2 locations together.
            bool linkLocations(ModelId srcId, ModelId dstId);
            
            bool unlinkAgents(AgentId srcAgentId, AgentId dstAgentId);
            /// Unlinks 2 locations iff they were linked to each other. Return true if they were.
            bool unlinkLocations(ModelId mid0, ModelId mid1);
            /// Unlinks a location from its source anf destination.
            void unlinkLocation(ModelId mid);

            bool onAgentLinkedToModel(Agent *agent, ModelId mid);

            void moveLocation(ModelId mid, Ogre::Vector3 const &pos);
            /// Updates the debug lines (from source, to destination) of the given model.
            void updateDebugLine(ModelId mid);
            /// Updates the debug line between 2 models, if it exists.
            void updateDebugLine(ModelId srcId, ModelId dstId);

        private:
            ModelPair makeKey(ModelId mid0, ModelId mid1);
            bool getDebugLine(ModelId mid0, ModelId mid1, DynamicLines *&line);
            void removeDebugLine(ModelId mid0, ModelId mid1);
            // owned
            std::map<ModelPair, DynamicLines *> mDebugLines;


    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
