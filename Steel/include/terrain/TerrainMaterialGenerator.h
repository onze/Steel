#ifndef STEEL_TERRAINMATERIALGENERATOR_H
#define STEEL_TERRAINMATERIALGENERATOR_H

#include <Ogre.h>
#include <OgreTerrainMaterialGenerator.h>

namespace Steel
{
    class TerrainMaterialGenerator : public Ogre::TerrainMaterialGenerator
    {
    public:
        TerrainMaterialGenerator(Ogre::String materialName);
        TerrainMaterialGenerator(TerrainMaterialGenerator const &o);
        virtual ~TerrainMaterialGenerator();

        void setMaterialName(Ogre::String materialName);
        Ogre::String materialName() const { return mMaterialName;}

        class Profile : public Ogre::TerrainMaterialGenerator::Profile
        {
        public:
            Profile(Ogre::TerrainMaterialGenerator *parent, const Ogre::String &name, const Ogre::String &desc);
            ~Profile();

            bool isVertexCompressionSupported() const { return false; }

            Ogre::MaterialPtr generate(const Ogre::Terrain *terrain);

            Ogre::MaterialPtr generateForCompositeMap(const Ogre::Terrain *terrain);

            Ogre::uint8 getMaxLayers(const Ogre::Terrain *terrain) const;

            void requestOptions(Ogre::Terrain *terrain);

            void updateParams(const Ogre::MaterialPtr &mat, const Ogre::Terrain *terrain);

            void updateParamsForCompositeMap(const Ogre::MaterialPtr &mat, const Ogre::Terrain *terrain);

        };

    private:
        Ogre::String mMaterialName;
    };
}
#endif // STEEL_TERRAINMATERIALGENERATOR_H
