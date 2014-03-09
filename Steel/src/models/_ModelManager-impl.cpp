/*
 * ModelManager-impl.cpp
 *
 *  Created on: 2011-06-18
 *      Author: onze
 */

#include "_ModelManager.cpp"

#include "models/OgreModel.h"
#include "models/PhysicsModel.h"
#include "models/BTModel.h"
#include "models/LocationModel.h"
#include "models/BlackBoardModel.h"

template class Steel::_ModelManager<Steel::OgreModel>;
template class Steel::_ModelManager<Steel::PhysicsModel>;
template class Steel::_ModelManager<Steel::BTModel>;
template class Steel::_ModelManager<Steel::LocationModel>;
template class Steel::_ModelManager<Steel::BlackBoardModel>;

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
