#include "terrain/TerrainMaterialGenerator.h"
#include <OgreTerrain.h>

namespace Steel
{
    TerrainMaterialGenerator::TerrainMaterialGenerator(Ogre::String materialName): Ogre::TerrainMaterialGenerator(),
        mMaterialName(materialName)
    {
        mProfiles.push_back(OGRE_NEW Profile(this, "OgreMaterial", "Profile for rendering Ogre standard material"));
        setActiveProfile("OgreMaterial");
    }

    TerrainMaterialGenerator::~TerrainMaterialGenerator()
    {
    }

    TerrainMaterialGenerator::TerrainMaterialGenerator(const TerrainMaterialGenerator &o): Ogre::TerrainMaterialGenerator(o),
        mMaterialName(o.mMaterialName)
    {
    }

    void TerrainMaterialGenerator::setMaterialName(Ogre::String materialName)
    {
        mMaterialName = materialName;
        _markChanged();
    }

    ////////////////////////////////////////////////////////////////////////////////////
    // Profile

    TerrainMaterialGenerator::Profile::Profile(Ogre::TerrainMaterialGenerator *parent, const Ogre::String &name, const Ogre::String &desc)
    : Ogre::TerrainMaterialGenerator::Profile(parent, name, desc)
    {
    }

    TerrainMaterialGenerator::Profile::~Profile()
    {
    }

    Ogre::MaterialPtr TerrainMaterialGenerator::Profile::generate(const Ogre::Terrain *terrain)
    {
        const Ogre::String &matName = terrain->getMaterialName();
        Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(matName);

        if(!mat.isNull())
            Ogre::MaterialManager::getSingleton().remove(matName);

        // Set Ogre material
        Steel::TerrainMaterialGenerator *parent = (Steel::TerrainMaterialGenerator *)getParent();
        mat = Ogre::MaterialManager::getSingleton().getByName(parent->materialName());
//         mat->getTechnique(0)->getPass(0)->setVertexProgram("triPlanarMaterial1_vs");
//         mat->getTechnique(0)->getPass(0)->setFragmentProgram("triPlanarMaterial1_ps");
        
//         // Clone material
//         if(parent->mCloneMaterial)
//         {
//             mat = mat->clone(matName);
//             parent->mMaterialName = matName;
//         }
// 
//         // Add normalmap
//         if(parent->mAddNormalMap)
//         {
//             // Get default pass
//             Ogre::Pass *p = mat->getTechnique(0)->getPass(0);
// 
//             // Add terrain's global normalmap to renderpass so the fragment program can find it.
//             Ogre::TextureUnitState *tu = p->createTextureUnitState(matName + "/nm");
// 
//             Ogre::TexturePtr nmtx = terrain->getTerrainNormalMap();
//             tu->_setTexturePtr(nmtx);
//         }

        return mat;
    }

    Ogre::MaterialPtr TerrainMaterialGenerator::Profile::generateForCompositeMap(const Ogre::Terrain *terrain)
    {
        return terrain->_getCompositeMapMaterial();
    }

    Ogre::uint8 TerrainMaterialGenerator::Profile::getMaxLayers(const Ogre::Terrain */*terrain*/) const
    {
        return 0;
    }

    void TerrainMaterialGenerator::Profile::requestOptions(Ogre::Terrain *terrain)
    {
        terrain->_setMorphRequired(false);
        terrain->_setNormalMapRequired(false); // enable global normal map
        terrain->_setLightMapRequired(false);
        terrain->_setCompositeMapRequired(false);
    }

    void TerrainMaterialGenerator::Profile::updateParams(const Ogre::MaterialPtr &/*mat*/, const Ogre::Terrain */*terrain*/)
    {
    }

    void TerrainMaterialGenerator::Profile::updateParamsForCompositeMap(const Ogre::MaterialPtr &/*mat*/, const Ogre::Terrain */*terrain*/)
    {
    }

}
