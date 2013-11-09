#ifndef STEEL_LOCATIONMODEL_H
#define STEEL_LOCATIONMODEL_H

#include <json/json.h>

#include "steeltypes.h"
#include "_ModelManager.h"
#include "Model.h"

namespace Steel
{
    class LocationModelManager;
    class LocationModel: public Model
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

        /// Null value for a path.
        static const LocationPathName EMPTY_PATH;

        LocationModel();
        LocationModel(const LocationModel &o);
        virtual ~LocationModel();
        virtual LocationModel &operator=(const LocationModel &o);
        virtual bool operator==(const LocationModel &o) const;

        static ModelType modelType()
        {
            return MT_LOCATION;
        }

        bool init(LocationModelManager *const locationModelMan);

        /// Not to be used. See fromJson(const Json::Value &node, LocationModelManager *locationModelMan).
        bool fromJson(const Json::Value &node);
        bool fromJson(const Json::Value &node, LocationModelManager *const locationModelMan);
        virtual void toJson(Json::Value &node);

        virtual void cleanup();

        inline std::set<AgentId> destinations() const{return mDestinations;}
        bool addDestination(AgentId aid);
        void removeDestination(AgentId aid);
        void removeAllDestinations();
        inline bool hasAnyDestination() const{return mDestinations.size()>0;}
        inline bool hasDestination(AgentId dst) const{return mDestinations.end()==mDestinations.find(dst);}

        inline std::set<AgentId> sources() const{return mSources;}
        bool addSource(AgentId aid);
        void removeSource(AgentId aid);
        void removeAllSources();
        inline bool hasAnySource() const{return mSources.size()>0;}
        inline bool hasSource(AgentId src) const{return mSources.end()==mSources.find(src);}

        void attachAgent(AgentId aid);
        inline AgentId attachedAgent(){return mAttachedAgent;}

        void setPosition(Ogre::Vector3 const &pos);
        Ogre::Vector3 position();

        inline bool hasPath(){return LocationModel::EMPTY_PATH != mPath;}
        inline LocationPathName path(){return mPath;}

        /** Internal. 
         * Set the location's path name, and recursively set all connected locations path name as well.
         * Returns true if all connected locations could be made as part of the given path.
         * If force is true, accept LocationModel::EMPTY_PATH.
         */
        void _setPath(const LocationPathName &name);
    private:
        void applyToNetWork(std::function<void(LocationModel *)> f);
        void insertNetworkModels(std::set<LocationModel *> &network);
        bool propagatePath(LocationModel *m0, LocationModel *m1);
        // not owned
        LocationModelManager *mLocationModelMan;
        //owned
        std::set<AgentId> mSources;
        std::set<AgentId> mDestinations;
        AgentId mAttachedAgent;
        Ogre::Vector3 mPosition;
        /// Name of the path the location is part of.
        LocationPathName mPath;
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
