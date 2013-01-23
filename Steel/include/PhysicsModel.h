#ifndef PHYSICSMODEL_H
#define PHYSICSMODEL_H

#include "Model.h"

namespace Steel
{

    class PhysicsModel:public Model
    {

        public:
            PhysicsModel();
            void init();
            PhysicsModel(const PhysicsModel& other);
            virtual PhysicsModel& operator=(const PhysicsModel& other);
            virtual ~PhysicsModel();
            virtual ModelType modelType();

            ///serialize itself into the given Json object
            virtual void toJson(Json::Value &object);

            ///deserialize itself from the given Json object. return true is successful.
            virtual bool fromJson(Json::Value &object);
        protected:
            virtual void cleanup();
    };
}
#endif // PHYSICSMODEL_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
