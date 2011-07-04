/*
 * Model.cpp
 *
 *  Created on: 2011-06-16
 *      Author: onze
 */

#include "../include/Model.h"

namespace Steel
{

Model::Model() :
	mRefCount(0)
{

}

Model::Model(const Model &m)
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
