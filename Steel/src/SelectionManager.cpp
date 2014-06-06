#include "SelectionManager.h"

#include "models/Agent.h"
#include "Engine.h"
#include "tools/OgreUtils.h"
#include "Level.h"
#include "models/AgentManager.h"
#include "TagManager.h"
#include "models/LocationModel.h"
#include "Debug.h"

namespace Steel
{

    SelectionManager::SelectionManager(Level *level)
        : mLevel(level), mSelection(Selection()), mMemos(std::map<Ogre::String, Selection>())
    {
    }

    SelectionManager::SelectionManager(const SelectionManager &o)
        : mLevel(o.mLevel), mSelection(o.mSelection), mMemos(o.mMemos)
    {
    }

    SelectionManager::~SelectionManager()
    {
        clearSelection();
    }

    SelectionManager &SelectionManager::operator=(const SelectionManager &o)
    {
        if(this != &o)
        {
            mSelection = o.mSelection;
            mMemos = o.mMemos;
        }

        return *this;
    }

    void SelectionManager::clearSelection(bool discardEvent/* = false*/)
    {
        Agent *agent;

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            agent->setSelected(false);
        }

        mSelection.clear();

        if(!discardEvent)
            dispatchSelectionChangedEvent();
    }

    void SelectionManager::deleteSelection()
    {
        if(!hasSelection())
            return;

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
            mLevel->agentMan()->deleteAgent(*it);

        mSelection.clear();
        dispatchSelectionChangedEvent();
    }

    void SelectionManager::rotateSelection(Ogre::Vector3 rotation)
    {
        //TODO:make selection child of a node on which the rotation is applied
        Agent *agent;

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            agent->rotate(rotation);
        }
    }

    Ogre::Vector3 SelectionManager::selectionPosition()
    {
        if(!hasSelection())
            return Ogre::Vector3::ZERO;

        return OgreUtils::mean(selectionPositions());
    }

    std::vector<Ogre::Vector3> SelectionManager::selectionPositions()
    {
        std::vector<Ogre::Vector3> pos;

        if(!hasSelection())
            return pos;

        Agent *agent;

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            pos.push_back(agent->position());
        }

        return pos;
    }

    std::vector<Ogre::Quaternion> SelectionManager::selectionRotations()
    {
        std::vector<Ogre::Quaternion> rots;

        if(!hasSelection())
            return rots;

        Agent *agent;

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            rots.push_back(agent->rotation());
        }

        return rots;
    }

    std::vector<Ogre::Vector3> SelectionManager::selectionScales()
    {
        std::vector<Ogre::Vector3> scales;

        if(!hasSelection())
            return scales;

        Agent *agent;

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            scales.push_back(agent->scale());
        }

        return scales;
    }

    Ogre::Quaternion SelectionManager::selectionOrientationFromCenter()
    {
        if(!hasSelection())
            return Ogre::Quaternion::IDENTITY;

        AgentId aid = mSelection.front();
        Agent *agent = mLevel->agentMan()->getAgent(aid);

        if(agent == nullptr)
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

        if(!hasSelection())
            return rots;

        Agent *agent;
        auto mean = selectionPosition();
        ;

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            rots.push_back(mean.getRotationTo(agent->position(), Ogre::Vector3::UNIT_Z));
        }

        return rots;
    }

    void SelectionManager::setSelectedAgents(Selection selection, bool replacePrevious)
    {
        if(replacePrevious)
            clearSelection(true);

        Agent *agent = nullptr;

        //process actual selections
        for(Selection::iterator it = selection.begin(); it != selection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(nullptr == agent)
                continue;

            mSelection.push_back(agent->id());
            agent->setSelected(true);
        }

        Debug::log("Selected agents: ")(mSelection).endl();
        dispatchSelectionChangedEvent();
    }

    void SelectionManager::setSelectedAgent(AgentId aid, bool replacePrevious /*= true*/)
    {
        if(replacePrevious)
            clearSelection(true);

        Agent *agent = mLevel->agentMan()->getAgent(aid);

        if(nullptr == agent)
        {
            dispatchSelectionChangedEvent();
            return;
        }

        mSelection.push_back(aid);
        agent->setSelected(true);

        Debug::log("Selected agent: ")(aid).endl();
        dispatchSelectionChangedEvent();
    }

    void SelectionManager::removeFromSelection(const Selection &selection)
    {
        for(auto it_sel = selection.begin(); it_sel != selection.end(); ++it_sel)
        {
            auto aid = *it_sel;

            if(INVALID_ID == aid)
                break;

            while(true)
            {
                auto it = std::find(mSelection.begin(), mSelection.end(), aid);

                if(mSelection.end() == it)
                    break;

                mSelection.erase(it);

                auto agent = mLevel->agentMan()->getAgent(aid);

                if(nullptr == agent)
                    continue;

                agent->setSelected(false);
            }
        }

        dispatchSelectionChangedEvent();
    }

    bool SelectionManager::isSelected(AgentId aid)
    {
        return std::find(mSelection.begin(), mSelection.end(), aid) != mSelection.end();
    }

    void SelectionManager::setSelectionPosition(const Ogre::Vector3 &pos)
    {
        if(!hasSelection())
            return;

        Agent *agent;
        Ogre::Vector3 diff = pos - OgreUtils::mean(selectionPositions());

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            agent->move(diff);
        }
    }

    void SelectionManager::setSelectionPositions(const std::vector<Ogre::Vector3> &pos)
    {
        if(!hasSelection())
            return;

        Agent *agent;
        assert(pos.size() == mSelection.size());
        auto it_pos = pos.begin();

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            agent->setPosition(*(it_pos++));
        }
    }

    void SelectionManager::moveSelection(const Ogre::Vector3 &dpos)
    {
        if(!hasSelection())
            return;

        Agent *agent;

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(nullptr == agent)
                continue;

            agent->move(dpos);
        }
    }

    void SelectionManager::moveSelection(const std::vector<Ogre::Vector3> &dpos)
    {
        if(!hasSelection())
            return;

        Agent *agent;
        assert(dpos.size() == mSelection.size());
        auto it_pos = dpos.begin();

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            agent->move(*(it_pos++));
        }
    }

    void SelectionManager::expandSelection(const float dpos)
    {
        if(!hasSelection())
            return;

        Agent *agent;
        auto center = selectionPosition();

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            agent->move((agent->position() - center).normalisedCopy() * dpos);
        }
    }

    void SelectionManager::setSelectionRotations(const std::vector<Ogre::Quaternion> &rots)
    {
        if(!hasSelection())
            return;

        Agent *agent;
        assert(rots.size() == mSelection.size());
        auto it_rot = rots.begin();

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            agent->setRotation(*(it_rot++));
        }
    }

    void SelectionManager::rotateSelectionRotationAroundCenter(const Ogre::Radian &angle, const Ogre::Vector3 &axis)
    {
        if(!hasSelection())
            return;

        Agent *agent;
        Ogre::Vector3 center = selectionPosition();
        auto rotation = Ogre::Quaternion(angle, axis);
        auto plane = Ogre::Plane(axis, center);

        if(mSelection.size() == 1)
        {
            agent = mLevel->agentMan()->getAgent(mSelection.front());

            if(agent == nullptr)
                return;

            agent->rotate(rotation);
        }
        else
        {
            for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
            {
                agent = mLevel->agentMan()->getAgent(*it);

                if(agent == nullptr)
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
        if(!hasSelection())
            return;

        Agent *agent;

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            agent->rescale(scale);
        }
    }

    void SelectionManager::setSelectionScales(const std::vector<Ogre::Vector3> &scales)
    {
        if(!hasSelection())
            return;

        Agent *agent;
        assert(scales.size() == mSelection.size());
        auto it_sca = scales.begin();

        for(Selection::iterator it = mSelection.begin(); it != mSelection.end(); ++it)
        {
            agent = mLevel->agentMan()->getAgent(*it);

            if(agent == nullptr)
                continue;

            agent->setScale(*(it_sca++));
        }
    }

    void SelectionManager::saveSelectionToMemo(Ogre::String memo)
    {
        Debug::log("SelectionManager::saveSelectionToMemo(): saving agents ")(mSelection)(" as ")(memo).endl();
        mMemos.erase(memo);

        if(mSelection.size())
            mMemos.insert(std::pair<Ogre::String, Selection>(memo, mSelection));
    }

    void SelectionManager::selectMemo(Ogre::String memo)
    {
        clearSelection();
        auto it = mMemos.find(memo);

        if(it != mMemos.end())
        {
            auto selection = (*it).second;
            Debug::log(STEEL_METH_INTRO, "selecting ", mSelection, " from memo ", memo).endl();
            setSelectedAgents(selection);
        }
        else
        {
            Debug::log(STEEL_METH_INTRO, "found no selection under memo ", memo).endl();
        }
    }

    void SelectionManager::tagSelection(Tag tag)
    {
        if(INVALID_TAG == tag)
            return;

        for(AgentId const & aid : mSelection)
        {
            Agent *agent = mLevel->agentMan()->getAgent(aid);

            if(nullptr == agent)
                continue;

            agent->tag(tag);
        }
    }

    void SelectionManager::untagSelection(Tag tag)
    {
        for(AgentId const & aid : mSelection)
        {
            Agent *agent = mLevel->agentMan()->getAgent(aid);

            if(nullptr == agent)
                continue;

            agent->untag(tag);
        }
    }

    std::set<Tag> SelectionManager::tagsUnion()
    {
        std::set<Tag> tags;

        if(nullptr != mLevel)
        {
            for(auto const aid : mSelection)
            {
                if(INVALID_ID == aid)
                    continue;

                auto agent = mLevel->agentMan()->getAgent(aid);

                if(nullptr == agent)
                    continue;

                auto agentTags = agent->tags();
                tags.insert(agentTags.begin(), agentTags.end());
            }
        }

        return tags;
    }

    std::set<Ogre::String> SelectionManager::pathsUnion()
    {
        std::set<Ogre::String> paths;

        if(nullptr != mLevel)
        {
            for(auto const aid : mSelection)
            {
                if(INVALID_ID == aid)
                    continue;

                auto agent = mLevel->agentMan()->getAgent(aid);

                if(nullptr == agent)
                    continue;

                LocationModel *model = agent->locationModel();

                if(nullptr == model || !model->hasPath())
                    continue;

                paths.insert(model->path());
            }
        }

        return paths;
    }

    void SelectionManager::addListener(Steel::SelectionManager::Listener *listener)
    {
        mListeners.insert(listener);
    }

    void SelectionManager::removeListener(Steel::SelectionManager::Listener *listener)
    {
        mListeners.erase(listener);
    }

    void SelectionManager::dispatchSelectionChangedEvent()
    {
        decltype(mListeners) copy(mListeners.begin(), mListeners.end());

        for(auto const & listener : copy)
            listener->onSelectionChanged(mSelection);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
