#ifndef TERRAINMANAGEREVENTLISTENER_H
#define TERRAINMANAGEREVENTLISTENER_H

#include <TerrainManager.h>

namespace Steel
{
    class TerrainManagerEventListener
    {
        public:
            TerrainManagerEventListener();
            ~TerrainManagerEventListener();
            virtual void onTerrainEvent(TerrainManager::LoadingState state);

    };

}
#endif // TERRAINMANAGEREVENTLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
