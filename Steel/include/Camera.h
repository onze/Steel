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

	/**
	 * TARGET mode only.
	 * rotates the camera of delta radians around its target, on the XZ plane.
	 */
	void rotateAroundTarget(float delta);
	/**
	 * TARGET mode only.
	 * rotates the camera of delta around its target, on a plane perpendicular to the ground and
	 * containing the camera axis.
	 */
	void pitchAroundTarget(float delta);

	/**
	 * TARGET mode only.
	 * zoom the camera of t units towards its target, then multiplies the distance to it by r.
	 */
	void zoom(float t, float r=1.f);

	/**
	 * TARGET mode only.
	 * returns the dist to its target.
	 */
	float zoom();

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
