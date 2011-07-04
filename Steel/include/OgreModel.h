/*
 * OgreModel.h
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#ifndef OGREMODEL_H_
#define OGREMODEL_H_

#include <OgreSceneNode.h>
#include <OgreEntity.h>

#include "Model.h"

namespace Steel
{

class OgreModel: public Model
{
private:
	OgreModel();
public:
	OgreModel(Ogre::SceneNode *sceneNode,Ogre::Entity *entity);
	OgreModel(const OgreModel &m);
	OgreModel &operator=(const OgreModel &m);
	virtual ~OgreModel();

	Ogre::Vector3 position();
	Ogre::Quaternion rotation();
	void rotate(Ogre::Vector3 &rotation);
	void setPosition(Ogre::Vector3 pos);
	void setSelected(bool selected);
	void translate(Ogre::Vector3 t);

protected:
	virtual void cleanup();
	Ogre::SceneNode *mSceneNode;
	Ogre::Entity *mEntity;
};

}

#endif /* OGREMODEL_H_ */
