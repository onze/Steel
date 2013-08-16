/*
 * Model.cpp
 *
 *  Created on: 2011-06-16
 *      Author: onze
 */

#include "Model.h"

namespace Steel
{

    Model::Model() :
        mRefCount(0)
    {

    }

    Model::Model(const Model &m):mRefCount(0)
    {
        this->operator =(m);
    }

    Model::~Model()
    {
    }

    Model &Model::operator=(const Model &m)
    {
        mRefCount=m.mRefCount;
        return *this;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
