#ifndef STEEL_BLACKBOARDMODEL_H
#define STEEL_BLACKBOARDMODEL_H

#include <json/json.h>

#include "steeltypes.h"
#include "_ModelManager.h"
#include "Model.h"

namespace Steel
{
    class BlackBoardModel: public Model
    {

        public:
            BlackBoardModel();
            BlackBoardModel(const BlackBoardModel& other);
            virtual ~BlackBoardModel();
            virtual BlackBoardModel& operator=(const BlackBoardModel& other);
            virtual bool operator==(const BlackBoardModel& other) const;

            virtual inline ModelType modelType()
            {
                return MT_BLACKBOARD;
            }
            virtual bool fromJson(Json::Value &object);
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
