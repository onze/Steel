/*
 * ModelManager.cpp
 *
 *  Created on: 2011-07-02
 *      Author: onze
 */

#include "ModelManager.h"
#include <Model.h>

namespace Steel
{
    std::set<Tag> ModelManager::modelTags(ModelId mid)
    {
        std::set<Tag> output;
        if(!isValid(mid))
            return output;
        Model *model=at(mid);
        if(NULL==model)
            return output;
        return model->tags();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
