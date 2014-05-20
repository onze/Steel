#ifndef STEEL_SELECTIONMANAGER_H
#define STEEL_SELECTIONMANAGER_H

#include "steeltypes.h"

namespace Steel
{
    class Level;
    class SelectionManager
    {

    public:
        class Listener
        {
        public:
            virtual void onSelectionChanged(Selection const& selection) = 0;
        };
        void addListener(Listener *listener);
        void removeListener(Listener *listener);
        void dispatchSelectionChangedEvent();

        SelectionManager(Level *level);
        SelectionManager(const SelectionManager &other);
        virtual ~SelectionManager();
        virtual SelectionManager &operator=(const SelectionManager &other);

        /// Set the currenlty selected agents. If replacePrevious is true (default), any previous selection is cancelled.
        void setSelectedAgents(Selection selection, bool replacePrevious = true);

        /// Set the currenlty selected agent. If replacePrevious is true (default), any previous selection is cancelled.
        void setSelectedAgent(AgentId aid, bool replacePrevious = true);

        /**
         * Select no agent. If discardEvent is true, Listeners aren't warn about it.
         * This can be useful if you intend on changing the selection multiple times in a row.
         */
        void clearSelection(bool discardEvent = false);
        void deleteSelection();
        void removeFromSelection(const Selection &aids);
        inline bool hasSelection()
        {
            return !mSelection.empty();
        }

        /// Return true if the given AgentId is part of the current selection.
        bool isSelected(AgentId aid);
        /// Returns mean position of selected agents.
        Ogre::Vector3 selectionPosition();
        /// Returns positions of selected agents.
        std::vector<Ogre::Vector3> selectionPositions();

        /// Returns orientations of selected agents' models.
        std::vector<Ogre::Quaternion> selectionRotations();
        /** Returns the shortest arc quaternion to rotate the selection
         * center to the first selected agent's model.*/
        Ogre::Quaternion selectionOrientationFromCenter();
        /** Same as selectionOrientationFromCenter, but for each selected agent's model.*/
        std::vector<Ogre::Quaternion> selectionOrientationsFromCenter();
        /// Returns scale of selected agents' models.
        std::vector<Ogre::Vector3> selectionScales();

        /// Move all selected agents' models by the given amount.
        void moveSelection(const Ogre::Vector3 &dpos);
        /// Move the <i>i</i>th selected agent's model by the <i>i</i>th given amount.
        void moveSelection(const std::vector<Ogre::Vector3> &dpos);
        /// Moves each selected agent away from the selection center.
        void expandSelection(float d);

        /// Move all selected agents' models to the given position.
        void setSelectionPosition(const Ogre::Vector3 &pos);
        /// Move the <i>i</i>th selected agent's model to the <i>i</i>th given position.
        void setSelectionPositions(const std::vector<Ogre::Vector3> &pos);

        /// Rotate the <i>i</i>th selected agent's model to the <i>i</i>th given rotation.
        void setSelectionRotations(const std::vector<Ogre::Quaternion> &rots);

        /** Move all selected agents' models around the selection center position,
         * so that the first agent's model's projection on a plane with the given
         * normal has the given angle to the z axis.
         * */
        void rotateSelectionRotationAroundCenter(const Ogre::Radian &angle, const Ogre::Vector3 &axis);

        /// Rotate all selected agents' models by the given rotation. See OgreModel::rotate for details.
        void rotateSelection(Ogre::Vector3 rotation);

        /// Rescale all selected agents' models by the given factor.
        void rescaleSelection(const Ogre::Vector3 &scale);
        /// Scale the <i>i</i>th selected agent's model to the <i>i</i>th given factor.
        void setSelectionScales(const std::vector<Ogre::Vector3> &scale);

        /// Selects all agents under the given memo.
        void selectMemo(Ogre::String memo);
        /// Saves the current selection as the given memo.
        void saveSelectionToMemo(Ogre::String memo);

        /// Assign tag to all agents in the current selection
        void tagSelection(Tag tag);
        /// Remove tag from all agents in the current selection
        void untagSelection(Tag tag);
        /// Returns a set of all tags found in the current selection.
        std::set<Tag> tagsUnion();
        /// Returns a set of all paths names found in the current selection.
        std::set<Ogre::String> pathsUnion();

        inline Selection selection()
        {
            return mSelection;
        }
    protected:
        // not owned
        Level *mLevel;

        // owned
        /// agents currenlty selected
        Selection mSelection;

        /// maps tags to set of agents
        std::map<Ogre::String, Selection> mMemos;

        /// Get notified when selection changes.
        std::set<Listener *> mListeners;
    };
}
#endif // SELECTIONMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
