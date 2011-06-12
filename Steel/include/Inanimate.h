/*
 * Inanimate.h
 *
 *  Created on: 2011-06-06
 *      Author: onze
 */

#ifndef INANIMATE_H_
#define INANIMATE_H_

#include <Ogre.h>

namespace Steel
{

class Inanimate
{
private:
	Inanimate();
public:
	Inanimate(Ogre::SceneNode *node);
	virtual ~Inanimate();
	//getters
	Ogre::SceneNode *node(){return mNode;};
protected:
	Ogre::SceneNode* mNode;
};

}

#endif /* INANIMATE_H_ */
