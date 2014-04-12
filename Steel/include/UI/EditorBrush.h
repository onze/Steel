#ifndef STEEL_EDITORBRUSH_H
#define STEEL_EDITORBRUSH_H

#include <vector>
#include <functional>

#include <OIS.h>
#include <OgreQuaternion.h>
#include <OgreVector3.h>
#include <OgreFrameListener.h>

#include "steeltypes.h"
#include "InputSystem/Input.h"
#include "terrain/TerrainManager.h"
#include "tools/DynamicLines.h"

namespace Ogre
{
    class SceneNode;
    class Terrain;
}

namespace Steel
{

    class ConfigFile;
    class Engine;
    class Editor;
    class InputManager;
    class SelectionBox;

    class EditorBrush: public Ogre::FrameListener
    {
            /**
             * This class is meant to instanced once, by the editor. It handles editing
             * interactions between the mouse and Steel.
             * Such interactions include:
             * - translation/rotation/scaling of models
             * - terrain edition
             */
        public:
            static const Ogre::String MODE;
            static const Ogre::String TERRA_SCALE;
            static const Ogre::String TERRA_SCALE_FACTOR;

            enum class BrushMode : unsigned int
            {
                NONE = 0, 
                TRANSLATE, 
                ROTATE, 
                SCALE, 
                TERRAFORM, 
                LINK
            };
            
            EditorBrush();
            EditorBrush(const EditorBrush& other);
            virtual ~EditorBrush();
            virtual EditorBrush& operator=(const EditorBrush& other);
            virtual bool operator==(const EditorBrush& other) const;

            void init(Engine *engine, Editor *editor, InputManager *mInputMan);
            void shutdown();

            /// called by Ogre once per frame. Used during terraforming.
            bool frameRenderingQueued(const Ogre::FrameEvent &evt);

            void getBrush(BrushMode mode);
            void dropBrush();

            // mouse events called by the editor
            bool mousePressed(Input::Code button, Input::Event const &evt);
            bool mouseReleased(Input::Code button, Input::Event const &evt);
            bool mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt);
            bool mouseWheeled(int delta, Input::Event const &evt);

            /** Tries to project a ray starting at the current camera and passing through the
             * given screen position, through the given plane. If the ray does, the intersection
             * is set into the given pos (in coordinates local), and the method returns true; It eturns false otherwise */
            bool mousePlaneProjection(const Ogre::Plane &plane, float x, float y, Ogre::Vector3 &pos);

            void onShow();
            void onHide();

            /// command interface
            bool processCommand(std::vector< Ogre::String > command);

            /// saves the current editing mode
            void pushMode();

            ///restore the last saved editing mode
            void popMode();

            void loadConfig(ConfigFile const &config);
            void saveConfig(ConfigFile &config) const;

            //getters
            inline BrushMode mode()
            {
                return mMode;
            }
            
            /**
             * Sets the brush in linking mode:
            * - upon clicking an agent, sourceAgentValidation is called with the 
            * agent id as argument. If the callback returns false, the process is 
            * cancelled.
            * - upon releasing the mouse on another agent, destinationAgentValidation 
            * is called with the agent id as argument. If the callback returns false, 
            * the process is cancelled.
            * - if both callback returned true, linkingValidatedCallbackFn is called 
            * with both agents as arguments. If shift is hold at the time the mouse 
            * is released, linkingValidatedAlternateCallbackFn is called instead. These 
            * callbacks return codes are not currently used for anything.
            */
            void setLinkingMode(std::function<bool(AgentId srcAid)> sourceAgentValidation,
                                std::function<bool(AgentId dstAid)> destinationAgentValidation,
                                std::function<bool(AgentId srcAid, AgentId dstAid)> linkingValidatedCallbackFn ,
                                std::function<bool(AgentId srcAid, AgentId const dstAid)> linkingValidatedAlternateCallbackFn = nullptr);
            
            //setters
            void setMode(BrushMode mode);

            float intensity();

            float radius();
            
            inline bool isDragging()
            {
                return mIsDraggingSelection;
            }
            
            inline bool isInContiniousMode()
            {
                return mContinuousModeActivated;
            }
            
            inline bool isSelecting()
            {
                return mIsSelecting;
            }

        private:
            /// Warns, in case some values make using the brush impossible.
            void checkTerraScaleFactorValue();
            /// Updates the SelectionManager's selection, and sets mIsSelecting.
            Selection mousePressedSelectionUpdate(Input::Code button, Input::Event const &evt);
            /// Internal helper method to switch to the right linking mode.
            bool processLinkCommand(std::vector<Ogre::String> command);

            //not owned
            Engine *mEngine;
            Editor *mEditor;
            InputManager *mInputMan;

            //owned
            /// holds the current way dragging selection will affect it
            BrushMode mMode;
            /// true between a mousePressed and its following mouseRelease when the brush is in a continuous mode (->terraforming)
            bool mContinuousModeActivated;

            /// holds the position to put the selection back to, if edition is cancelled
            std::vector<Ogre::Vector3> mSelectionPosBeforeTransformation;
            std::vector<Ogre::Quaternion> mSelectionRotBeforeTransformation;
            std::vector<Ogre::Vector3> mSelectionScaleBeforeTransformation;

            /// true when selection is begin dragged (translated/rotated/scaled)
            bool mIsDraggingSelection;
            /// set to true when cancelling a selection drag movement.
            bool mIsDraggingSelectionCancelled;

            /// disk used as visual terraforming brush
            static Ogre::SceneNode *sTerraBrushVisual;
            /// factor by which the terraforming brush is scaled up (down: 1/mTerraScale)
            float mTerraScaleFactor;
            /// scale of the brush
            Ogre::Vector3 mTerraScale;
            /// terrain height at mousePressed (used in terrascale equalize mode)
            float mSelectedTerrainHeight;
            /// current terrabrush shape
            TerrainManager::RaiseShape mRaiseShape;

            std::vector<BrushMode> mModeStack;
            std::set<Ogre::Terrain *> mModifiedTerrains;

            /// True while the mouse is being dragged, but drag did not start on a selected agent.
            bool mIsSelecting;
            /// Helper for selecting many agents at once
            SelectionBox *mSelectionBox;
            
            /// id of the source agent in link mode
            AgentId mFirstLinkedAgent;
            /// Line drawn during linking (of LocationModel, etc)
            DynamicLines mLinkingLine;
            
            std::function<bool(AgentId const srcAid)> mLinkingSourceAgentValidationFn;
            std::function<bool(AgentId const dstAid)> mLinkingDestinationAgentValidationFn;
            std::function<bool(AgentId const srcAid, AgentId const dstAid)> mLinkingValidatedCallbackFn;
            std::function<bool(AgentId const srcAid, AgentId const dstAid)> mLinkingValidatedAlternateCallbackFn;
    };
    
    Ogre::String toString(EditorBrush::BrushMode e);
    
}
#endif // STEEL_EDITORBRUSH_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
