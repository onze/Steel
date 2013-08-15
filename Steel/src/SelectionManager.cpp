#include "SelectionManager.h"

#include "Agent.h"
#include "Engine.h"
#include <tools/OgreUtils.h>
#include "Level.h"

namespace Steel
{

    SelectionManager::SelectionManager(Engine *engine)
        : mSelection(Selection()), mSelectionsTags(std::map<Ogre::String, Selection>())
    {
        mEngine = engine;
    }

    SelectionManager::SelectionManager(const SelectionManager& o)
        : mEngine(o.mEngine), mSelection(o.mSelection), mSelectionsTags(o.mSelectionsTags)
    {

    }

    SelectionManager::~SelectionManager()
    {

    }

    SelectionManager& SelectionManager::operator=(const SelectionManager& o)
    {
        if (this != &o)
        {
            mSelection = o.mSelection;
            mSelectionsTags = o.mSelectionsTags;
        }
        return *this;
    }

    void SelectionManager::clearSelection()
    {
        Agent *agent;
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->setSelected(false);
        }
        mSelection.clear();
    }

    void SelectionManager::deleteSelection()
    {
        if (!hasSelection())
            return;
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
            mEngine->level()->deleteAgent(*it);
        mSelection.clear();
    }

    void SelectionManager::rotateSelection(Ogre::Vector3 rotation)
    {
        //TODO:make selection child of a node on which the rotation is applied
        Agent *agent;
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->rotate(rotation);
        }
    }

    Ogre::Vector3 SelectionManager::selectionPosition()
    {
        if (!hasSelection())
            return Ogre::Vector3::ZERO;
        return OgreUtils::mean(selectionPositions());
    }

    std::vector<Ogre::Vector3> SelectionManager::selectionPositions()
    {
        std::vector<Ogre::Vector3> pos;
        if (!hasSelection())
            return pos;
        Agent *agent;
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            pos.push_back(agent->position());
        }
        return pos;
    }

    std::vector<Ogre::Quaternion> SelectionManager::selectionRotations()
    {
        std::vector<Ogre::Quaternion> rots;
        if (!hasSelection())
            return rots;
        Agent *agent;
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            rots.push_back(agent->rotation());
        }
        return rots;
    }

    std::vector<Ogre::Vector3> SelectionManager::selectionScales()
    {
        std::vector<Ogre::Vector3> scales;
        if (!hasSelection())
            return scales;
        Agent *agent;
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            scales.push_back(agent->scale());
        }
        return scales;
    }

    Ogre::Quaternion SelectionManager::selectionOrientationFromCenter()
    {
        if (!hasSelection())
            return Ogre::Quaternion::IDENTITY;

        AgentId aid = mSelection.front();
        Agent *agent = mEngine->level()->getAgent(aid);
        if (agent == NULL)
        {
            Debug::error("SelectionManager::selectionOrientationFromCenter(): selection's first item (agent ")(aid)(
                ") is not valid.").endl();
            return Ogre::Quaternion::IDENTITY;
        }
        return selectionPosition().getRotationTo(agent->position(), Ogre::Vector3::UNIT_Z);
    }

    std::vector<Ogre::Quaternion> SelectionManager::selectionOrientationsFromCenter()
    {
        std::vector<Ogre::Quaternion> rots;
        if (!hasSelection())
            return rots;
        Agent *agent;
        auto mean = selectionPosition();
        ;
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            rots.push_back(mean.getRotationTo(agent->position(), Ogre::Vector3::UNIT_Z));
        }
        return rots;
    }

    void SelectionManager::setSelectedAgents(Selection selection, bool replacePrevious)
    {
        if (replacePrevious)
            clearSelection();
        Agent *agent = NULL;
        //process actual selections
        for (Selection::iterator it = selection.begin(); it != selection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (NULL == agent)
                continue;
            mSelection.push_back(agent->id());
            agent->setSelected(true);
        }
        Debug::log("Selected agents: ")(mSelection).endl();
    }

    void SelectionManager::removeFromSelection(const Selection &selection)
    {
        for (auto it_sel = selection.begin(); it_sel != selection.end(); ++it_sel)
        {
            auto aid = *it_sel;
            if (INVALID_ID == aid)
                break;
            while (true)
            {
                auto it = std::find(mSelection.begin(), mSelection.end(), aid);
                if (mSelection.end() == it)
                    break;
                mSelection.erase(it);

                auto agent = mEngine->level()->getAgent(aid);
                if (NULL == agent)
                    continue;
                agent->setSelected(false);
            }
        }
    }

    bool SelectionManager::isSelected(AgentId aid)
    {
        return std::find(mSelection.begin(), mSelection.end(), aid) != mSelection.end();
    }

    void SelectionManager::setSelectionPosition(const Ogre::Vector3 &pos)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        Ogre::Vector3 diff = pos - OgreUtils::mean(selectionPositions());
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->move(diff);
        }
    }

    void SelectionManager::setSelectionPositions(const std::vector<Ogre::Vector3> &pos)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        assert(pos.size()==mSelection.size());
        auto it_pos = pos.begin();
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->setPosition(*(it_pos++));
        }
    }

    void SelectionManager::moveSelection(const Ogre::Vector3 &dpos)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (NULL == agent)
                continue;
            agent->move(dpos);
        }
    }

    void SelectionManager::moveSelection(const std::vector<Ogre::Vector3> &dpos)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        assert(dpos.size()==mSelection.size());
        auto it_pos = dpos.begin();
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->move(*(it_pos++));
        }
    }

    void SelectionManager::expandSelection(const float dpos)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        auto center = selectionPosition();
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->move((agent->position() - center).normalisedCopy() * dpos);
        }
    }

    void SelectionManager::setSelectionRotations(const std::vector<Ogre::Quaternion> &rots)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        assert(rots.size()==mSelection.size());
        auto it_rot = rots.begin();
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->setRotation(*(it_rot++));
        }
    }

    void SelectionManager::rotateSelectionRotationAroundCenter(const Ogre::Radian &angle, const Ogre::Vector3 &axis)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        Ogre::Vector3 center = selectionPosition();
        auto rotation = Ogre::Quaternion(angle, axis);
        auto plane = Ogre::Plane(axis, center);
        if (mSelection.size() == 1)
        {
            agent = mEngine->level()->getAgent(mSelection.front());
            if (agent == NULL)
                return;
            agent->rotate(rotation);
        }
        else
        {
            for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
            {
                agent = mEngine->level()->getAgent(*it);
                if (agent == NULL)
                    continue;;

                auto pos = agent->position();
                auto proj = plane.projectVector(pos);
                // rotated projection (rotated within plane space, but kept in world space)
                auto rotatedProj = rotation * (proj - center) + center;
                // deproject it as far as it was
                rotatedProj += pos - proj;
                agent->setPosition(rotatedProj);
                agent->rotate(rotation);
            }
        }
    }

    void SelectionManager::rescaleSelection(const Ogre::Vector3 &scale)
    {
        if (!hasSelection())
            return;
        Agent *agent;
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->rescale(scale);
        }
    }

    void SelectionManager::setSelectionScales(const std::vector<Ogre::Vector3> &scales)
    {
        if (!hasSelection())
            return;

        Agent *agent;
        assert(scales.size()==mSelection.size());
        auto it_sca = scales.begin();
        for (Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mEngine->level()->getAgent(*it);
            if (agent == NULL)
                continue;
            agent->setScale(*(it_sca++));
        }
    }

    void SelectionManager::setSelectionTag(const Ogre::String &tag)
    {
        Debug::log("SelectionManager::setSelectionTag(): saving ")(mSelection)(" under tag ")(tag).endl();
        mSelectionsTags.erase(tag);
        if (mSelection.size())
            mSelectionsTags.insert(std::pair<Ogre::String, Selection>(tag, mSelection));
    }

    void SelectionManager::setTaggedSelection(const Ogre::String &tag)
    {
        clearSelection();
        auto it = mSelectionsTags.find(tag);
        if (it != mSelectionsTags.end())
        {
            auto selection = (*it).second;
            Debug::log("SelectionManager::setTaggedSelection(): selecting ")(mSelection)(" as tag ")(tag).endl();
            setSelectedAgents(selection);
        }
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
