/*
 * Level.h
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifndef LEVEL_H_
#define LEVEL_H_

#include <map>

#include "steeltypes.h"
#include "OgreModelManager.h"
#include "Thing.h"

namespace Steel
{

class Level
{
private:
	Level();
	Level(Level &level);
	Level &operator=(const Level &level);
public:
	/**
	 * since ogre can't reload resource from the same resourceGroup easily, a level can duplicate it's
	 */
	Level(Ogre::String name, Ogre::SceneManager *sceneManager);
	virtual ~Level();
	Ogre::String addAuxiliaryResourceName(Ogre::String name);
	void deleteThing(ThingId id);
	/**
	 * creates a new instance of Thing.
	 * name: name of the mesh to use
	 * pos: position of the node
	 * rot: rotation of the node
	 * involvesNewResources: if false (default), needed resources are assumed to be declared to Ogre::ResourceManager.
	 */
	ThingId newThing(	Ogre::String name,
						Ogre::Vector3 pos = Ogre::Vector3::ZERO,
						Ogre::Quaternion rot = Ogre::Quaternion::IDENTITY,
						bool involvesNewResources=false);
	bool isOver(void);
	bool unload(void);
	//getters
	inline static Ogre::String path(void)
	{
		return sPath;
	}
	Thing *getThing(ThingId id);
	/**
	 * fills the ModelId list with ids of Things that own nodes in the the given list.
	 */
	void getThingsIdsFromSceneNodes(std::list<Ogre::SceneNode *> &nodes, std::list<ModelId> &selection);
	inline OgreModelManager *ogreModelMan()
	{
		return mOgreModelMan;
	}
	//not used yet
	ModelManager *modelManager(ModelType modelType);
	//setters
	inline static void setPath(Ogre::String _path)
	{
		sPath = _path;
	}
	;
protected:
	//static
	///path to level folders
	static Ogre::String sPath;
	//endof static

	Ogre::String mName;
	/**
	 * Pointer to steel's global scene manager.
	 * TODO: move scene manager init to level constructor, so that different levels can use different scene managers.
	 */
	Ogre::SceneManager *mSceneManager;
	/**
	 * root node of the level. All level-dependant entities are its children.
	 */
	Ogre::SceneNode *mLevelRoot;
	/**
	 * entity container.
	 */
	std::map<ThingId, Thing *> mThings;
	/**
	 * responsible for ogreModel
	 */
	OgreModelManager *mOgreModelMan;
	///
	unsigned int mResGroupAux;
};

}

#endif /* LEVEL_H_ */
