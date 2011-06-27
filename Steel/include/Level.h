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
	Level(Ogre::String name, Ogre::SceneManager *sceneManager);
	virtual ~Level();
	/**
	 *
	 */
	ThingId newThing(	Ogre::String name,
						Ogre::Vector3 pos = Ogre::Vector3::ZERO,
						Ogre::Quaternion rot = Ogre::Quaternion::IDENTITY);
	bool isOver(void);
	bool unload(void);
	//getters
	inline static Ogre::String path(void)
	{
		return sPath;
	}
	;

	Thing *getThing(ThingId id);
	inline OgreModelManager *ogreModelMan()
	{
		return mOgreModelMan;
	}
	//setters
	inline static void setPath(Ogre::String _path)
	{
		sPath = _path;
	}
	;
protected:
	//static
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
	std::map<ThingId, Steel::Thing *> mThings;
	/**
	 * responsible for ogreModel
	 */
	OgreModelManager *mOgreModelMan;
};

}

#endif /* LEVEL_H_ */
