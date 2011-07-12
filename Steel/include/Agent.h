/*
 * Agent.h
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
 * Agent is the base class of Steel objects. (Object was too common, Entity was taken by Ogre, so I did not see that
 * many possibilities.)
 *
 * Agent uses composition of Model subclasses to achieve different behaviors. One can think of things as entries in a
 * table, that merely contains only ids of the models the are made of.
 */
class Agent
{
public:
	Agent(Level *level);
	virtual ~Agent();
	Agent(const Agent &t);
	Agent &operator=(const Agent &);

	//getter
	inline AgentId id()
	{
		return mId;
	}
	;
	void addModel(ModelType modelType, ModelId modelId);

	/**
	 * returns an address to the model of the given type, if any. returns NULL otherwise.
	 */
	Model *model(ModelType modelType);
	/**
	 * return the id of the model of the given type, if any. returns Steel::INVALID_ID otherwise.
	 */
	ModelId modelId(ModelType modelType);
	/**
	 * shortcut to Agent::model(MT_OGRE).
	 */
	inline OgreModel *ogreModel()
	{
		return (OgreModel *) model(MT_OGRE);
	}
	/**
	 * shortcut to Agent::modelId(MT_OGRE).
	 */
	inline ModelId ogreModelId()
	{
		return modelId(MT_OGRE);
	}

private:
	//static stuff
	static AgentId sNextId;
	static inline AgentId getNextId()
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
	AgentId mId;
	/**
	 * ptr to the level the thing is in.
	 */
	Level *mLevel;
	std::map<ModelType, ModelId> mModelIds;
};

}

#endif /* THING_H_ */
