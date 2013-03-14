/*
 * ModelManager-impl.cpp
 *
 *  Created on: 2011-06-18
 *      Author: onze
 */

#include "_ModelManager.cpp"

#include "OgreModel.h"
#include "PhysicsModel.h"
#include "BlackBoardModel.h"
#include "BTModel.h"

template class Steel::_ModelManager<Steel::OgreModel>;
template class Steel::_ModelManager<Steel::PhysicsModel>;
template class Steel::_ModelManager<Steel::BlackBoardModel>;
template class Steel::_ModelManager<Steel::BTModel>;

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
