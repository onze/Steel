#include "[['%(ClassName)s'%ref]]Model.h"

namespace Steel
{

    [['%(ClassName)s'%ref]]Model::[['%(ClassName)s'%ref]]Model():
        Model()
    {

    }

    [['%(ClassName)s'%ref]]Model::[['%(ClassName)s'%ref]]Model(const [['%(ClassName)s'%ref]]Model& other)
    {

    }

    [['%(ClassName)s'%ref]]Model::~[['%(ClassName)s'%ref]]Model()
    {

    }

    [['%(ClassName)s'%ref]]Model& [['%(ClassName)s'%ref]]Model::operator=(const [['%(ClassName)s'%ref]]Model& other)
    {
        return *this;
    }

    bool [['%(ClassName)s'%ref]]Model::operator==(const [['%(ClassName)s'%ref]]Model& other) const
    {
        return false;
    }

    bool [['%(ClassName)s'%ref]]Model::fromJson(Json::Value &object)
    {
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
