/*
 * OgreModel.h
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#ifndef OGREMODEL_H_
#define OGREMODEL_H_

#include <OgreSceneNode.h>

#include "Model.h"

namespace Steel
{

class OgreModel: public Model
{
private:
	OgreModel();
public:
	OgreModel(Ogre::SceneNode *sceneNode);
	OgreModel(const OgreModel &m);
	OgreModel &operator=(const OgreModel &m);
	virtual ~OgreModel();

	Ogre::Vector3 position();
	void setSelected(bool selected);
	void translate(Ogre::Vector3 t);

protected:
	Ogre::SceneNode *mSceneNode;
};

}

#endif /* OGREMODEL_H_ */
