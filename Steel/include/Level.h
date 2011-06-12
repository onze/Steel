/*
 * Level.h
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifndef LEVEL_H_
#define LEVEL_H_

#include <vector>

#include "Inanimate.h"

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
	unsigned long createInanimate(	Ogre::String name,
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

	Inanimate *getInanimate(unsigned long id);
	//setters
	inline static void setPath(Ogre::String _path)
	{
		sPath = _path;
	}
	;
protected:
	//static
	static Ogre::String sPath;
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
	std::vector<Steel::Inanimate *> mInanimates;
};

}

#endif /* LEVEL_H_ */
