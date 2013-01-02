/*
 * Camera.h
 *
 *  Created on: 2011-05-11
 *      Author: onze
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <json/json.h>

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
        private:
            Camera();
            Camera(const Camera &camera);
            Camera &operator=(const Camera &camera);
        public:
            Camera(Ogre::SceneManager *sceneManager);
            virtual ~Camera();

            /**
             * deserialization method;
             * returns true if all required fields were found.
             */
            bool fromJson(Json::Value &value);

            /**
             * orient the camera center towards the given screen position;
             */
            void lookTowards(float x, float y, float roll = .0f, float factor = 1.f);

            /**
             * serialization method;
             */
            Json::Value toJson();
            
            /// returns the position of the drop target in front of the camera.
            Ogre::Vector3 dropTargetPosition();

            /// returns the rotation of the drop target in front of the camera.
            Ogre::Quaternion dropTargetRotation();

            /**
             * moves the camera according to the given coordinates.
             */
            void translate(float dx, float dy, float dz, float speed=1.f);

            /**
             * returns a pointer to the internal ogre camera instance.
             */
            inline Ogre::Camera *cam()
            {
                return mCamera;
            };

            /**
             * returns a pointer to the node the internal camera instance is attached to.
             */
            inline Ogre::SceneNode *camNode()
            {
                return mCameraNode;
            };

        protected:
            Ogre::Camera *mCamera;
            Ogre::SceneNode *mCameraNode;
            Ogre::SceneManager *mSceneManager;
    };

}

#endif /* CAMERA_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
