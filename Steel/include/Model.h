/*
 * Model.h
 *
 *  Created on: 2011-06-16
 *      Author: onze
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <json/json.h>

#include "Debug.h"

namespace Steel
{

class Model
{
public:
	Model();
	Model(const Model &m);
	virtual ~Model();

	virtual Model &operator=(const Model &m);
	inline void incRef()
	{
		++mRefCount;
	}
	inline void decRef()
	{
		if (--mRefCount <= 0L)
			cleanup();
	}
	inline bool isFree()
	{
		return mRefCount <= 0L;
	}
	///serialize itself into the given Json object
	virtual void toJson(Json::Value &object)=0;
	///deserialize itself from the given Json object
	virtual void fromJson(Json::Value &object)=0;
	//getters
	inline unsigned long refCount()
	{
		return mRefCount;
	}
protected:
	/**
	 * called by decRef() when the ref count gets below 0.
	 */
	virtual void cleanup()
	{
	}
	;
	/**
	 * Number of things referencing it.
	 */
	unsigned long mRefCount;
};

}

#endif /* MODEL_H_ */
