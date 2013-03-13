#ifndef STEEL_[[('%(TypeName)s'%ref).upper()]]MODEL_H
#define STEEL_[[('%(TypeName)s'%ref).upper()]]MODEL_H

#include <json/json.h>

#include "steeltypes.h"
#include "_ModelManager.h"
#include "Model.h"

namespace Steel
{
    class [['%(ClassName)s'%ref]]Model:public Model
    {

        public:
            [['%(ClassName)s'%ref]]Model();
            [['%(ClassName)s'%ref]]Model(const [['%(ClassName)s'%ref]]Model& other);
            virtual ~[['%(ClassName)s'%ref]]Model();
            virtual [['%(ClassName)s'%ref]]Model& operator=(const [['%(ClassName)s'%ref]]Model& other);
            virtual bool operator==(const [['%(ClassName)s'%ref]]Model& other) const;
            
            virtual inline ModelType modelType()
            {
                return MT_[[('%(TypeName)s'%ref).upper()]];
            }
            virtual bool fromJson(Json::Value &object);
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
