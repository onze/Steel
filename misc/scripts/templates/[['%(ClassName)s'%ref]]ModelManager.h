#ifndef STEEL_[[('%(TypeName)s'%ref).upper()]]MODELMANAGER_H
#define STEEL_[[('%(TypeName)s'%ref).upper()]]MODELMANAGER_H

#include <json/json.h>

#include "steeltypes.h"

#include "_ModelManager.h"
#include "[['%(ClassName)s'%ref]]Model.h"

namespace Steel
{
    class [['%(ClassName)s'%ref]]ModelManager:public _ModelManager<[['%(ClassName)s'%ref]]Model>
    {
        public:
            [['%(ClassName)s'%ref]]ModelManager(Level *level);
            virtual ~[['%(ClassName)s'%ref]]ModelManager();

            /// modelType associated with this Manager
            virtual inline ModelType modelType()
            {
                return MT_[[('%(TypeName)s'%ref).upper()]];
            };
            
            bool fromSingleJson(Json::Value &model, ModelId &id);

        protected:

    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
