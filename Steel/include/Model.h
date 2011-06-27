/*
 * Model.h
 *
 *  Created on: 2011-06-16
 *      Author: onze
 */

#ifndef MODEL_H_
#define MODEL_H_

namespace Steel
{

class Model
{
public:
	Model();
	virtual ~Model();
	inline void incRef()
	{
		++mRefCount;
	}
	;
	inline unsigned long decRef()
	{
		return isFree() ? 0L : --mRefCount;
	}
	inline bool isFree()
	{
		return mRefCount == 0;
	}
	;
protected:
	/**
	 * Number of things referencing it.
	 */
	unsigned long mRefCount;
};

}

#endif /* MODEL_H_ */
