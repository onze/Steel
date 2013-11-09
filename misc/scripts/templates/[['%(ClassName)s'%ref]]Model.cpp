#include "[['%(ClassName)s'%ref]]Model.h"

namespace Steel
{

    [['%(ClassName)s'%ref]]Model::[['%(ClassName)s'%ref]]Model():
        Model()
    {

    }

    [['%(ClassName)s'%ref]]Model::[['%(ClassName)s'%ref]]Model(const [['%(ClassName)s'%ref]]Model& o) : Model(o)
    {

    }

    [['%(ClassName)s'%ref]]Model::~[['%(ClassName)s'%ref]]Model()
    {

    }

    [['%(ClassName)s'%ref]]Model& [['%(ClassName)s'%ref]]Model::operator=(const [['%(ClassName)s'%ref]]Model& o)
    {
        Model::operator=(o);
        return *this;
    }

    bool [['%(ClassName)s'%ref]]Model::operator==(const [['%(ClassName)s'%ref]]Model& o) const
    {
        return Model::operator==(o);
    }
    
    bool [['%(ClassName)s'%ref]]Model::init([['%(ClassName)s'%ref]]ModelManager const *manager)
    {
        return true;
    }

    bool [['%(ClassName)s'%ref]]Model::fromJson(Json::Value &root)
    {
        deserializeTags(root);
        return true;
    }
    
    void [['%(ClassName)s'%ref]]Model::toJson(Json::Value &root) const
    {
        serializeTags(root);
    }
    
    void [['%(ClassName)s'%ref]]Model::cleanup()
    {
        Model::cleanup();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
