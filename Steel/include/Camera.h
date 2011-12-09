/*
 * Camera.h
 *
 *  Created on: 2011-05-11
 *      Author: onze
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <OGRE/OgreCamera.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>

namespace Steel
{

class Camera
{
	/**
	 * wraps ogre camera and add features to ease its manipulation.
	 */
private:
	Camera();
	Camera(const Camera &camera);
	Camera &operator=(const Camera &camera);
public:
	Camera(Ogre::SceneManager *sceneManager);
	virtual ~Camera();

	void lookTowards(float x, float y, float roll = .0f, float factor = 1.f);
	void translate(float dx, float dy, float dz);

	inline Ogre::Camera *cam()
	{
		return mCamera;
	}
	;
	inline Ogre::SceneNode *camNode()
	{
		return mCameraNode;
	}
	;

protected:
	Ogre::Camera *mCamera;
	Ogre::SceneNode *mCameraNode;
	Ogre::SceneManager *mSceneManager;
};

}

#endif /* CAMERA_H_ */
