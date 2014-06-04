#include "UI/PropertyGrid/PropertyGridModelAdapter.h"
#include "models/PhysicsModel.h"
#include "models/PhysicsModelManager.h"
#include "models/AgentManager.h"
#include "models/Agent.h"
#include "models/BTModel.h"
#include "models/BTModelManager.h"
#include "models/BlackBoardModel.h"
#include "models/BlackBoardModelManager.h"
#include "Level.h"
#include "Numeric.h"

namespace Steel
{

    PropertyGridModelAdapter::PropertyGridModelAdapter(Level *const level, AgentId aid, ModelId mid): PropertyGridAdapter(),
        mLevel(level), mAid(aid), mMid(mid)
    {}

    PropertyGridModelAdapter::~PropertyGridModelAdapter()
    {
        mLevel = nullptr;
        mMid = INVALID_ID;
    }

    void PropertyGridModelAdapter::buildProperties()
    {
        //PropertyGridAdapter::mProperties;
    }

    StringVector const &PropertyGridPhysicsModelAdapter::boundingShapeValues()
    {
        static const StringVector values(
        {
            toString(Steel::BoundingShape::BOX),
                     toString(BoundingShape::SPHERE),
                     toString(BoundingShape::CONVEXHULL),
                     toString(BoundingShape::TRIMESH)
        });
        return values;
    }

    void PropertyGridPhysicsModelAdapter::buildProperties()
    {
        // shape list
        {
            PropertyGridProperty *prop = new PropertyGridProperty(PhysicsModel::BBOX_SHAPE_ATTRIBUTE);
            PropertyGridProperty::StringVectorSelectionReadCallback readCB([this]()->PropertyGridProperty::StringVectorSelection
            {
                PropertyGridProperty::StringVectorSelection readItem;
                PhysicsModel *const pmodel = mLevel->physicsModelMan()->at(mMid);

                if(nullptr == pmodel)
                    return readItem;

                readItem.selectableValues = boundingShapeValues();
                readItem.selectedIndex = (PropertyGridProperty::StringVectorSelection::selection_type)(std::find(readItem.selectableValues.begin(), readItem.selectableValues.end(), toString(pmodel->shape())) - readItem.selectableValues.begin());

                return readItem;
            });
            PropertyGridProperty::StringVectorSelectionWriteCallback writeCB([this](PropertyGridProperty::StringVectorSelection::selection_type index)
            {
                Agent *const agent = mLevel->agentMan()->getAgent(mAid);

                if(nullptr == agent)
                    return;

                OgreModel *const omodel = agent->ogreModel();
                PhysicsModel *const pmodel = mLevel->physicsModelMan()->at(mMid);

                if(nullptr == omodel || nullptr == pmodel)
                    return;

                StringVector possibleValues = boundingShapeValues();
                BoundingShape shape = BoundingShape::SPHERE;

                if(index < possibleValues.size())
                    shape = toBoundingShape(possibleValues[index]);

                pmodel->setShape(omodel, shape);
            });
            prop->setCallbacks(readCB, writeCB);
            PropertyGridAdapter::mProperties.push_back(prop);
        }

        // mass
        {
            PropertyGridProperty *prop = new PropertyGridProperty(PhysicsModel::MASS_ATTRIBUTE);
            PropertyGridProperty::RangeReadCallback readCB([this]()->PropertyGridProperty::Range
            {
                PropertyGridProperty::Range range;
                PhysicsModel *const pmodel = mLevel->physicsModelMan()->at(mMid);

                if(nullptr != pmodel)
                    range.value = (PropertyGridProperty::Range::value_type)pmodel->mass();

                range.min = 0.;
                range.max = 100.;
                Numeric::clamp(range.value, range.min, range.max);
                return range;
            });
            PropertyGridProperty::RangeWriteCallback writeCB([this](PropertyGridProperty::Range const & range)
            {
                PhysicsModel *const pmodel = mLevel->physicsModelMan()->at(mMid);
                Numeric::clamp(range.value, range.min, range.max);
                pmodel->setMass((decltype(pmodel->mass()))range.value);
            });
            prop->setCallbacks(readCB, writeCB);
            PropertyGridAdapter::mProperties.push_back(prop);
        }

        // damping
        {
            PropertyGridProperty *prop = new PropertyGridProperty(PhysicsModel::DAMPING_ATTRIBUTE);
            PropertyGridProperty::RangeReadCallback readCB([this]()->PropertyGridProperty::Range
            {
                PropertyGridProperty::Range range;
                PhysicsModel *const pmodel = mLevel->physicsModelMan()->at(mMid);

                if(nullptr != pmodel)
                    range.value = (PropertyGridProperty::Range::value_type)pmodel->linearDamping();

                range.min = 0.;
                range.max = 1.;
                Numeric::clamp(range.value, range.min, range.max);
                return range;
            });
            PropertyGridProperty::RangeWriteCallback writeCB([this](PropertyGridProperty::Range const & range)
            {
                PhysicsModel *const pmodel = mLevel->physicsModelMan()->at(mMid);
                Numeric::clamp(range.value, range.min, range.max);
                pmodel->setDamping((decltype(pmodel->mass()))range.value);
            });
            prop->setCallbacks(readCB, writeCB);
            PropertyGridAdapter::mProperties.push_back(prop);
        }
    }

    void PropertyGridBTModelAdapter::buildProperties()
    {
        // stateStream name
        {
            PropertyGridProperty *prop = new PropertyGridProperty("StateStream");
            PropertyGridProperty::StringReadCallback readCB([this]()->Ogre::String
            {
                BTModel *const btModel = mLevel->BTModelMan()->at(mMid);
                return btModel->stateStream().debugName();
            });
            prop->setCallbacks(readCB, nullptr);
            PropertyGridAdapter::mProperties.push_back(prop);
        }
    }

    Signal PropertyGridBlackboardModelAdapter::getSignal(PropertyGridAdapter::PublicSignal signal) const
    {
        /// Replace our "new property" signal by our blackBoardModel's
        if(PropertyGridAdapter::PublicSignal::newProperty == signal)
        {
            BlackBoardModel *const bbModel = mLevel->blackBoardModelMan()->at(mMid);
            if(nullptr!=bbModel)
                return bbModel->getSignal(BlackBoardModel::PublicSignal::newVariable);
        }
        return PropertyGridAdapter::getSignal(signal);
    }
    
    void PropertyGridBlackboardModelAdapter::buildProperties()
    {
        BlackBoardModel *const bbModel = mLevel->blackBoardModelMan()->at(mMid);

        if(nullptr == bbModel)
            return;
        
        // variables
        for(auto it : bbModel->variables())
        {
            Ogre::String key = it.first;
            
            PropertyGridProperty *prop = new PropertyGridProperty(key);
            PropertyGridProperty::StringReadCallback readCB([this, key]()->Ogre::String
            {
                BlackBoardModel *const bbModel = mLevel->blackBoardModelMan()->at(mMid);
                return bbModel->getStringVariable(key);
            });
            prop->setCallbacks(readCB, nullptr);
            PropertyGridAdapter::mProperties.push_back(prop);
        }
    }
}

