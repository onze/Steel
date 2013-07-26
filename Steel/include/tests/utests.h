#ifndef STEEL_UTESTS_H
#define STEEL_UTESTS_H

namespace Steel
{
    class Engine;

    bool startTests(Engine* engine, bool abortOnFail);
    bool test_StringUtils();
    bool test_File();
    bool test_ConfigFile();

    void registerGameplayUTests(Engine *e);
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
