/*
 * OgreModelManager.h
 *
 *  Created on: 2011-06-16
 *      Author: onze
 */

#ifndef OGREMODELMANAGER_H_
#define OGREMODELMANAGER_H_

#include <list>

#include <json/json.h>

#include <OgreSceneNode.h>
#include <OgreSceneManager.h>

#include "steeltypes.h"
#include "_ModelManager.h"
#include "OgreModel.h"

namespace Steel
{
/**
 * instances of this class handle ogre related stuff.
 */
class OgreModelManager: public _ModelManager<OgreModel>
{
public:
	OgreModelManager();
	OgreModelManager(Ogre::SceneManager *sceneManager, Ogre::SceneNode *levelRoot);
	virtual ~OgreModelManager();
	/**
	 * initialize new OgreModel according to data in the json serialization.
	 */
	bool fromJson(Json::Value &models);
	/**
	 * initialize a new OgreModel and returns its identifier.
	 */
	ModelId newModel(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot);
	///////////////////////////////////////////////////////////
	//getters
	inline Ogre::SceneManager *sceneManager()
	{
		return mSceneManager;
	}

protected:
	Ogre::SceneManager *mSceneManager;
	Ogre::SceneNode *mLevelRoot;
};

}

#endif /* OGREMODELMANAGER_H_ */
