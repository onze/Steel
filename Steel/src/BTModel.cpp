/*
 * BTModel.cpp
 *
 *  Created on: Jan 18, 2012
 *      Author: onze
 */

#include "BTModel.h"

namespace Steel
{

    BTModel::BTModel()
    {
    }

    BTModel::BTModel ( const BTModel &m )
    {
    }

    BTModel::~BTModel()
    {
    }

    BTModel &BTModel::operator= ( const BTModel &m )
    {
        return *this;
    }

    void BTModel::init()
    {
    }

    ModelType BTModel::modelType()
    {
        return MT_BT;
    }

/// deserialize itself from the given Json object
    bool BTModel::fromJson ( Json::Value &node )
    {
        return true;
    }

/// serialize itself into the given Json object
    void BTModel::toJson ( Json::Value &node )
    {

    }

    void BTModel::cleanup()
    {

    }

} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
