/*
 * ModelManager-impl.cpp
 *
 *  Created on: 2011-06-18
 *      Author: onze
 */

#include "_ModelManager.cpp"
#include "OgreModelManager.h"

#include "OgreModel.h"
#include "BTModel.h"
#include <PhysicsModel.h>

template class Steel::_ModelManager<Steel::BTModel>;
template class Steel::_ModelManager<Steel::OgreModel>;
template class Steel::_ModelManager<Steel::PhysicsModel>;

