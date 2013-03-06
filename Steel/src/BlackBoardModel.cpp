#include "BlackBoardModel.h"

namespace Steel
{

    BlackBoardModel::BlackBoardModel():
        Model()
    {

    }

    BlackBoardModel::BlackBoardModel(const BlackBoardModel& other)
    {

    }

    BlackBoardModel::~BlackBoardModel()
    {

    }

    BlackBoardModel& BlackBoardModel::operator=(const BlackBoardModel& other)
    {
        return *this;
    }

    bool BlackBoardModel::operator==(const BlackBoardModel& other) const
    {
        return false;
    }

    bool BlackBoardModel::fromJson(Json::Value &object)
    {
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
