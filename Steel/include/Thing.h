/*
 * Thing.h
 *
 *  Created on: 2011-06-15
 *      Author: onze
 */

#ifndef THING_H_
#define THING_H_

#include <limits.h>
#include <exception>

#include <OgreString.h>

#include "steeltypes.h"
#include "Model.h"
#include "OgreModel.h"

namespace Steel
{

class Level;

/**
 * Thing is the base class of Steel objects. (Object was too common, Entity was taken by Ogre, so I did not see that
 * many possibilities.)
 *
 * Thing uses composition of Model subclasses to achieve different behaviors. One can think of things as entries in a
 * table, that merely contains only ids of the models the are made of.
 */
class Thing
{
public:
	Thing(Level *level);
	virtual ~Thing();
	Thing(const Thing &t);
	Thing &operator=(const Thing &);

	//getter
	inline ThingId id()
	{
		return mId;
	}
	;
	void addModel(ModelType modelType, ModelId modelId);
	OgreModel *ogreModel();

	Model *model(ModelType modelType);

private:
	//static stuff
	static ThingId sNextId;
	static inline ThingId getNextId()
	{
		if (sNextId == ULONG_MAX)
			throw "Steel::Thing::sNextId has reached ULONG_MAX.";
		return sNextId++;
	}
	;
	//end of static stuff
	/**
	 * unique id.
	 */
	ThingId mId;
	/**
	 * ptr to the level the thing is in.
	 */
	Level *mLevel;
	std::map<ModelType, ModelId> mModelIds;
};

}

#endif /* THING_H_ */
