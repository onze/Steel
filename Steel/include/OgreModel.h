/*
 * OgreModel.h
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#ifndef OGREMODEL_H_
#define OGREMODEL_H_

#include <json/json.h>

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreEntity.h>

#include "Model.h"

namespace Steel
{

class OgreModel: public Model
{
public:
	OgreModel();
	void init(	Ogre::String meshName,
				Ogre::Vector3 pos,
				Ogre::Quaternion rot,
				Ogre::SceneNode *mLevelRoot,
				Ogre::SceneManager *mSceneManager);
	OgreModel(const OgreModel &m);
	OgreModel &operator=(const OgreModel &m);
	virtual ~OgreModel();

	Ogre::Vector3 position();
	Ogre::Quaternion rotation();
	void rotate(Ogre::Vector3 &rotation);
	void setNodeAny(Ogre::Any any);
	void setPosition(Ogre::Vector3 pos);
	void setSelected(bool selected);
	void translate(Ogre::Vector3 t);
	///serialize itself into the given Json object
	virtual void toJson(Json::Value &object);
	///deserialize itself from the given Json object
	virtual void fromJson(Json::Value &object);

protected:
	virtual void cleanup();
	Ogre::SceneNode *mSceneNode;
	Ogre::Entity *mEntity;
};

}

#endif /* OGREMODEL_H_ */
