#ifndef STEEL_LOCATIONMODEL_H
#define STEEL_LOCATIONMODEL_H

#include <json/json.h>

#include "steeltypes.h"
#include "_ModelManager.h"
#include "Model.h"
#include "AgentManager.h"

namespace Steel
{
    class LocationModelManager;
    class LocationModel:public Model
    {

        public:
            /// AgentId of the previous location in this location's path.
            static const char *SOURCE_ATTRIBUTE;
            /// AgentId of the next location in this location's path.
            static const char *DESTINATION_ATTRIBUTE;
            /// Agent this location refers to.
            static const char *ATTACHED_AGENT_ATTRIBUTE;
            /// Name of the path this location is part of.
            static const char *PATH_ATTRIBUTE;

            LocationModel();
            LocationModel(const LocationModel& o);
            virtual ~LocationModel();
            virtual LocationModel& operator=(const LocationModel& o);
            virtual bool operator==(const LocationModel& o) const;

            static ModelType modelType()
            {
                return MT_LOCATION;
            }
            
            bool init(LocationModelManager * const locationModelMan);

            /// Not to be used. See fromJson(const Json::Value &node, LocationModelManager *locationModelMan).
            bool fromJson(const Json::Value &node);
            bool fromJson(const Json::Value &node, LocationModelManager * const locationModelMan);
            virtual void toJson(Json::Value &node);

            virtual void cleanup();

            inline ModelId destination()
            {
                return mDestination;
            }
            bool setDestination(ModelId aid);
            void unsetDestination();
            inline bool hasDestination()
            {
                return INVALID_ID!=mDestination;
            }

            inline ModelId source()
            {
                return mSource;
            }
            bool setSource(ModelId aid);
            void unsetSource();
            inline bool hasSource()
            {
                return INVALID_ID!=mSource;
            }

            void attachAgent(AgentId aid);
            inline AgentId attachedAgent()
            {
                return mAttachedAgent;
            }
            
            void setPosition(Ogre::Vector3 const &pos);
            Ogre::Vector3 position();
            
            inline bool hasPath()
            {
                return ""!=mPath;
            }
            inline Ogre::String path()
            {
                return mPath;
            }
            /**
             * Set the location's path name, and recursively set all connected locations path name as well.
             * Returns true if all connected locations could be made as part of the given path.
             */
            bool setPath(Ogre::String const &name);
            void unsetPath();

        private:
            /// same as setPath, but if force is true, accept the empty string.
            bool _setPath(Ogre::String const &name, bool force=false);
            // not owned
            LocationModelManager *mLocationModelMan;
            //owned
            AgentId mSource;
            AgentId mDestination;
            AgentId mAttachedAgent;
            Ogre::Vector3 mPosition;
            /// Name of the path the location is part of.
            Ogre::String mPath;
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
