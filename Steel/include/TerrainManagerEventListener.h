#ifndef STEEL_TERRAINMANAGEREVENTLISTENER_H
#define STEEL_TERRAINMANAGEREVENTLISTENER_H

#include <TerrainManager.h>

namespace Steel
{
/// Class inheriting void onTerrainEvent(TerrainManager::LoadingState state);
    class TerrainManagerEventListener
    {
        public:
            TerrainManagerEventListener();
            ~TerrainManagerEventListener();
            virtual void onTerrainEvent(TerrainManager::LoadingState state);

    };

}
#endif // STEEL_TERRAINMANAGEREVENTLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
