#ifndef EDITORBRUSH_H
#define EDITORBRUSH_H

#include <vector>

#include <OIS.h>
#include <OgreQuaternion.h>
#include <OgreVector3.h>
#include <OgreFrameListener.h>


namespace Ogre
{
    class SceneNode;
    class Terrain;
}

namespace Steel
{
    class Engine;
    class Editor;
    class InputManager;

    class EditorBrush:public Ogre::FrameListener
    {
            /**
             * This class is meant to instanced once, by the editor. It handles editing
             * interactions between the mouse and Steel.
             * Such interactions include:
             * - translation/rotation/scaling of models
             * - terrain edition
             */
        public:
            enum BrushMode {NONE=0,TRANSLATE, ROTATE, SCALE,TERRAFORM };
            EditorBrush();
            EditorBrush(const EditorBrush& other);
            virtual ~EditorBrush();
            virtual EditorBrush& operator=(const EditorBrush& other);
            virtual bool operator==(const EditorBrush& other) const;

            void init(Engine *engine,Editor *editor,InputManager *mInputMan);
            void shutdown();

            /// called by Ogre once per frame. Used during terraforming.
            bool frameRenderingQueued(const Ogre::FrameEvent &evt);

            void getBrush(BrushMode mode);
            void dropBrush();

            // mouse events called by the editor
            bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseMoved(const OIS::MouseEvent& evt);
    
            void onShow();
            void onHide();
            
            /// command interface
            void processCommand(std::vector<Ogre::String> command);
            
            /// saves the current editing mode
            void pushMode();
            
            ///restore the last saved editing mode
            void popMode();
            
            //getters
            inline BrushMode mode()
            {
                return mMode;
            }
            //setters
            void setMode(BrushMode mode);

        protected:
            //not owned
            Engine *mEngine;
            Editor *mEditor;
            InputManager *mInputMan;

            //owned
            /// holds the current way dragging selection will affect it
            BrushMode mMode;

            /// holds the position to put the selection back to, if edition is cancelled
            Ogre::Vector3 mSelectionPosBeforeTransformation;
            std::vector<Ogre::Quaternion> mSelectionRotBeforeTransformation;

            /// true when selection is begin dragged (translated/rotated/scaled)
            bool mIsDraggingSelection;
            /// set to true when cancelling a selection drag movement.
            bool mIsDraggingSelectionCancelled;

            /// disk used as visual terraforming brush
            Ogre::SceneNode *mTerraBrushVisual;
            /// factor by which the terraforming brush is scaled up (down: 1/mTerraScale)
            float mTerraScale;
            
            std::vector<BrushMode> mModeStack;
            std::set<Ogre::Terrain *> mModifiedTerrains;
    };
}
#endif // EDITORBRUSH_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
