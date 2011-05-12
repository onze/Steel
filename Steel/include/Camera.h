/*
 * Camera.h
 *
 *  Created on: 2011-05-11
 *      Author: onze
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

namespace Steel
{

class Camera
{
	/**
	 * wraps ogre camera and add features to ease its manipulation.
	 */
public:
	/**
	 * in TARGET mode, the camera constantly looks at an Ogre::MovableObject.
	 */
	enum Mode
	{
		FREE, TARGET
	};
	Camera(void);
	Camera(Ogre::SceneManager *sceneManager);
	Camera(const Camera &camera);
	virtual ~Camera();
	Camera &operator=(const Camera &camera);
	void setMode(Mode mode);

	void rotateAroundTarget(float delta);
	void pitchAroundTarget(float delta);

	inline Ogre::Camera *cam(){return mCamera;};


protected:
	Ogre::SceneManager *mSceneManager;
	Ogre::Camera *mCamera;
	Ogre::SceneNode *mCameraNode;
	Ogre::SceneNode *mTarget;
	int mMode;
};

}

#endif /* CAMERA_H_ */
